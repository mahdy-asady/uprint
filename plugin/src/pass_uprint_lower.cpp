#include "pass_uprint_lower.hpp"

#include <iostream>

#include <tree.h>
#include <stringpool.h>
#include <stor-layout.h>

#include <gimple.h>
#include <gimple-iterator.h>

#include <context.h>

#include "config.hpp"

// Register callback to PASS MANAGER
void register_uprint_lower(database *db) {
    static pass_uprint_lower pass_instant(g, db);

    // PLUGIN_PASS_MANAGER_SETUP event
    struct register_pass_info pass = {
        .pass = &pass_instant,
        .reference_pass_name = "ssa",           // Get called after GCC has produced SSA representation
        .ref_pass_instance_number = 1,          // After the first opt pass to be sure opt will not throw away our stuff
        .pos_op = PASS_POS_INSERT_AFTER
    };

    // add our pass hooking into pass manager
    register_callback(PLUGIN_NAME, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass);
}

/*
 * Build a packed payload RECORD_TYPE for the given argument list.
 * The returned struct has a leading `uint32_t record_id` field followed by
 * one FIELD_DECL per provided argument. `arg_sizes` is populated with the
 * byte-size of each argument field (for registration in the database).
 * The function finalizes layout by calling `layout_type` and returns the
 * constructed RECORD_TYPE node.
 */
tree pass_uprint_lower::create_payload_struct(const std::vector<tree>& args,
                                             std::vector<uint8_t> &arg_sizes) {
    // Create anonymous RECORD_TYPE (struct)
    tree struct_type = make_node(RECORD_TYPE);
    TYPE_NAME(struct_type) = create_tmp_var_name("uprint_payload_t");
    TYPE_PACKED(struct_type) = 1; // __attribute__((packed))
    SET_TYPE_ALIGN(struct_type, 8); // Force overall struct alignment to 8 bits (1 byte)

    // Field 0: uint32_t record_id
    tree first_field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                get_identifier("record_id"),
                                uint32_type_node);
    DECL_CONTEXT(first_field) = struct_type;
    DECL_PACKED(first_field) = 1;
    tree last_field = first_field;

    arg_sizes.clear();

    // Append remaining argument fields (f1, f2, ...)
    for (size_t i = 0; i < args.size(); ++i) {
        tree arg = args[i];
        tree arg_type = TREE_TYPE(arg);
        tree size_unit = TYPE_SIZE_UNIT(arg_type);
        uint8_t size_bytes = (size_unit && tree_fits_uhwi_p(size_unit))
                            ? static_cast<uint8_t>(tree_to_uhwi(size_unit)) : 0;

        arg_sizes.push_back(size_bytes);

        DEBUG("Arg %zu: type=%s, precision=%u bits, unsigned=%d, size=%u bytes", 
                i + 1, get_tree_code_name(TREE_CODE(arg_type)), TYPE_PRECISION(arg_type), TYPE_UNSIGNED(arg_type), size_bytes);

        // Create field declaration: type field_i;
        tree field = build_decl(UNKNOWN_LOCATION, FIELD_DECL,
                                get_identifier(("f" + std::to_string(i)).c_str()),
                                arg_type);
        DECL_CONTEXT(field) = struct_type;
        DECL_PACKED(field) = 1;
        DECL_CHAIN(last_field) = field;
        last_field = field;
    }

    TYPE_FIELDS(struct_type) = first_field;
    layout_type(struct_type); // Finalize field offsets and struct size

    return struct_type;
}

/*
 * Emit GIMPLE assignments to populate the given `payload_var` instance:
 * - write `record_id` into the first field (record_id)
 * - assign each subsequent `args[i]` into the corresponding struct field
 * All assignments are inserted before the provided `gsi` iterator.
 */
void pass_uprint_lower::fill_payload_struct(tree struct_type,
                                            tree payload_var,
                                            const std::vector<tree>& args,
                                            uint32_t record_id,
                                            gimple_stmt_iterator *gsi) {
    // 1. Assign record_id to field 0: packet.record_id = 0x7C9A4B12
    tree first_field = TYPE_FIELDS(struct_type);
    tree record_ref = build3(COMPONENT_REF, uint32_type_node, payload_var, first_field, NULL_TREE);
    gassign *assign_record = gimple_build_assign(record_ref, build_int_cstu(uint32_type_node, record_id));
    gsi_insert_before(gsi, assign_record, GSI_SAME_STMT);

    tree current_field = DECL_CHAIN(first_field);
    for (size_t i = 0; i < args.size(); ++i) {
        tree arg = args[i];
        tree field_type = TREE_TYPE(current_field);

        // Build: payload_var.field_i
        tree field_ref = build3(COMPONENT_REF, field_type,
                                payload_var, current_field, NULL_TREE);

        // Build GIMPLE assignment: payload_var.field_i = arg_i;
        gassign *assign_stmt = gimple_build_assign(field_ref, arg);
        gsi_insert_before(gsi, assign_stmt, GSI_SAME_STMT);

        current_field = DECL_CHAIN(current_field);
    }
}


unsigned int pass_uprint_lower::execute (function* exec_fun) {
    // print the function name
    DEBUG("Inspecting function '%s'", function_name(exec_fun));

    basic_block bb;
    // Iterate through all statements (GIMPLE instructions) inside the function
    FOR_EACH_BB_FN(bb, exec_fun) {
        for (gimple_stmt_iterator it = gsi_start_bb(bb); !gsi_end_p(it); gsi_next(&it)) {
            gimple* stmt = gsi_stmt(it);
            // If current statement is a function call then we shall check callee name
            if (is_gimple_call(stmt)) {
                tree callee = gimple_call_fndecl(stmt);
                if (callee) {
                    const char *callee_name = IDENTIFIER_POINTER(DECL_NAME(callee));
                    if (callee_name != nullptr && std::strcmp(callee_name, INTERFACE_FN_NAME) == 0) {
                        replace_uprint_call(stmt, &it);
                    }
                }
            }
        }
    }

    // done!
    return 0;
}

// Extract first argument of called function in statement. That must be format string
std::string pass_uprint_lower::extract_format_string(gimple* stmt) {
    tree arg = gimple_call_arg(stmt, 0);
    if (arg) {
        tree format_node = TREE_OPERAND (arg, 0);
        return std::string(TREE_STRING_POINTER(format_node));
    }
    return "";
}

/*
 * Lower a `uprint(...)` GIMPLE call into a packed payload and an emit call.
 * Steps performed:
 * - build a packed struct type containing `record_id` and each argument field
 * - create a local payload instance and populate it with `record_id` and args
 * - replace the original call with a call to `__uprint_emit(payload_ptr, size)`
 * All new GIMPLE assignments are inserted immediately before the original call.
 */
void pass_uprint_lower::replace_uprint_call(gimple *uprint_stmt, gimple_stmt_iterator *gsi) {
    unsigned num_args = gimple_call_num_args(uprint_stmt);

    if (num_args < 1) {
        std::cerr << "Error: uprint() call has no arguments!" << std::endl;
        return;
    }

    std::string format_string = extract_format_string(uprint_stmt);
    if (format_string.empty()) {
        std::cerr << "Error: uprint() call has no format string!" << std::endl;
    }

    std::vector<tree> args = get_uprint_real_args(uprint_stmt);
    std::vector<uint8_t> arg_sizes;

    // Build struct type
    tree struct_type = create_payload_struct(args, arg_sizes);

    // Declare a local temporary variable of this struct type
    tree payload_var = create_tmp_var(struct_type, "uprint_data");

    uint32_t record_id = db->append(format_string, arg_sizes);

    // Populate payload fields and record id
    fill_payload_struct(struct_type, payload_var, args, record_id, gsi);

    // Address of payload: (const void*)&payload_var
    tree payload_ptr_expr = build1(ADDR_EXPR, const_ptr_type_node, payload_var);
    
    // Size of payload: sizeof(struct_type)
    tree payload_size_expr = fold_convert(uint16_type_node, TYPE_SIZE_UNIT(struct_type));


    // ------------------------------------------------------------------------
    // Replace uprint(...) with __uprint_emit(record_id, ptr, size)
    // ------------------------------------------------------------------------
    tree emit_fndecl = get_or_create_uprint_emit_fndecl();

    gcall *emit_call = gimple_build_call(emit_fndecl, 2, 
                                        payload_ptr_expr, 
                                        payload_size_expr);

    // Preserve original source line location for debugging
    gimple_set_location(emit_call, gimple_location(uprint_stmt));

    // Replace original call statement in GIMPLE stream
    gsi_replace(gsi, emit_call, true);
}

tree pass_uprint_lower::get_or_create_uprint_emit_fndecl() {
    static tree emit_fndecl = NULL_TREE;

    // Return cached declaration if already created in this translation unit
    if (emit_fndecl != NULL_TREE) {
        return emit_fndecl;
    }

    const char *fn_name = "__uprint_emit";
    tree fn_id = get_identifier(fn_name);

    // 1. Define parameter types: (const void *packet, uint16_t len)
    tree param_types = void_list_node; // Terminates argument list (non-variadic)
    param_types = tree_cons(NULL_TREE, uint16_type_node, param_types);
    param_types = tree_cons(NULL_TREE, const_ptr_type_node, param_types);

    // 2. Build function type: void fn(const void *, uint16_t)
    tree fn_type = build_function_type(void_type_node, param_types);

    // 3. Create the FUNCTION_DECL node
    emit_fndecl = build_decl(UNKNOWN_LOCATION, FUNCTION_DECL, fn_id, fn_type);

    // 4. Set symbol linkage and visibility flags
    DECL_EXTERNAL(emit_fndecl) = 1;
    TREE_PUBLIC(emit_fndecl) = 1;
    TREE_STATIC(emit_fndecl) = 0;
    DECL_ARTIFICIAL(emit_fndecl) = 1;
    DECL_PRESERVE_P(emit_fndecl) = 1;

    return emit_fndecl;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-use-of-uninitialized-value"

std::vector<tree> pass_uprint_lower::get_uprint_real_args(gimple *uprint_stmt) {
    unsigned num_args = gimple_call_num_args(uprint_stmt);
    std::vector<tree> real_args;
    real_args.reserve(num_args - 1);

    for (unsigned i = 1; i < num_args; ++i) {
        tree arg = gimple_call_arg(uprint_stmt, i);
        // Trace back SSA_NAME to unwrap variadic promotion casts (NOP_EXPR / CONVERT_EXPR)
        if (TREE_CODE(arg) == SSA_NAME) {
            gimple *def_stmt = SSA_NAME_DEF_STMT(arg);
            if (def_stmt && is_gimple_assign(def_stmt)) {
                enum tree_code rhs_code = gimple_assign_rhs_code(def_stmt);
                if (rhs_code == NOP_EXPR || rhs_code == CONVERT_EXPR) {
                    tree orig_op = gimple_assign_rhs1(def_stmt);
                    arg = orig_op;
                }
            }
        }
        real_args.push_back(arg);
    }

    return real_args;
}
#pragma GCC diagnostic pop
