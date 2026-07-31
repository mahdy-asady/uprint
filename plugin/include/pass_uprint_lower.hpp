#pragma once
#include <gcc-plugin.h>
#include <tree-pass.h>
#include "config.hpp"
#include "database.hpp"


void register_uprint_lower(database *db);


/**
 * Metadata for a pass, non-varying across all instances of a pass
 * @see Declared in tree-pass.h
 * @note Refer to tree-pass for docs about
 */
const pass_data pass_data_uprint_lower =
{
    .type = GIMPLE_PASS,                                    // type of pass
    .name = PLUGIN_NAME,                                    // name of plugin
    .optinfo_flags = OPTGROUP_NONE,                         // no opt dump
    .tv_id = TV_NONE,                                       // no timevar (see timevar.h)
    .properties_required = PROP_gimple_any,                 // entire gimple grammar as input
    .properties_provided = 0,                               // no prop in output
    .properties_destroyed = 0,                              // no prop removed
    .todo_flags_start = 0,                                  // need nothing before
    .todo_flags_finish = TODO_update_ssa | TODO_cleanup_cfg // need to update SSA repr after and repair cfg
};

/**
 * Lowers uprint() calls into tokenized ID emissions and stashes string literals into external database
 * @note Extends gimple_opt_pass class
 * @see Declared in tree-pass.h
 */
class pass_uprint_lower : public gimple_opt_pass {
    public:
        /**
         * Constructor
         */
        pass_uprint_lower(gcc::context *ctxt, database *db_ref)
            : gimple_opt_pass(pass_data_uprint_lower, ctxt), db(db_ref) {}

        /**
         * This and all sub-passes are executed only if the function returns true
         * @note Defined in opt_pass father class
         * @see Defined in tree-pass.h
         */
        bool gate (function* gate_fun)
        {
            return true;
            (void) gate_fun;
        }

        /**
         * This is the code to run when pass is executed
         * @note Defined in opt_pass father class
         * @see Defined in tree-pass.h
         */
        unsigned int execute (function* exec_fun);

    private:
        database *db;
};
