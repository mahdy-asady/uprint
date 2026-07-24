#include <gcc-plugin.h>
#include <plugin-version.h>
#include <iostream>

#include "uprint.hpp"

// GCC GPL Compatibile verification signature
int plugin_is_GPL_compatible;


// Register additional information about the plugin. Used by --help and --version.
void register_plugin_info() {
    static struct plugin_info uprint_plugin_info = {
        .version = PLUGIN_VERSION,
        .help    = "Deferred, tokenized logging. Visit " PLUGIN_WEBSITE " for more information."
    };

    register_callback(PLUGIN_NAME, PLUGIN_INFO, NULL, &uprint_plugin_info);
}


int plugin_init(struct plugin_name_args *plugin_info, struct plugin_gcc_version *version) {
    // Check the GCC version used to compile the plugin against the current GCC version. If they don't equal, the plugin aborts initialization
    if (!plugin_default_version_check (version, &gcc_version))
        return 1;
    std::cout << PLUGIN_NAME << " plugin v" << PLUGIN_VERSION << " loaded!" << std::endl;
    
    register_plugin_info();

    return 0;
    (void) plugin_info;
}
