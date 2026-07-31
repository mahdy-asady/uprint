#include "pass_uprint_lower.hpp"

#include <iostream>
#include <tree.h>
#include <print-tree.h>
#include <gimple.h>
#include <gimple-iterator.h>
#include <context.h>

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


unsigned int pass_uprint_lower::execute (function* exec_fun) {
    // Get the name of function whose body we are reading
    const char *fn_name = function_name(exec_fun);

    // print the function name
    std::cout << std::endl << "Inspecting function '" << fn_name << "'" << std::endl;

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
                        const char *format_string = extract_format_string(stmt);
                        if (format_string != nullptr) {
                            uint32_t id = db->append(format_string);
                            std::cout << "Found call to uprint(). ID: " << id << std::endl;
                        }
                    }
                }
            }
        }
    }

    // done!
    return 0;
}

// Extract first argument of called function in statement. That must be format string
const char *pass_uprint_lower::extract_format_string(gimple* stmt) {
    tree arg = gimple_call_arg(stmt, 0);
    if (arg) {
        tree format_node = TREE_OPERAND (arg, 0);
        return TREE_STRING_POINTER(format_node);
    }
    return nullptr;
}
