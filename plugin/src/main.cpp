#include <gcc-plugin.h>
#include <tree.h>
#include <plugin-version.h>
#include <iostream>
#include <memory>

#include "config.hpp"
#include "pass_uprint_lower.hpp"
#include "database.hpp"

// Global instance of database
static std::unique_ptr<database> db;

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

// Callback to register attributes with GCC
static void register_uprint_attribute(void *event_data, void *data) {
    static struct attribute_spec uprint_attr = { "uprint_loaded", 0, 0, false, false, false, false, nullptr, nullptr };
    register_attribute(&uprint_attr);
    (void) event_data;
    (void) data;
}

bool plugin_version_check (struct plugin_gcc_version *host_version) {
    struct { unsigned major, minor, build; } host, plugin;

    sscanf(host_version->basever, "%u.%u.%u", &host.major, &host.minor, &host.build);
    sscanf(gcc_version.basever, "%u.%u.%u", &plugin.major, &plugin.minor, &plugin.build);

    if(host.major == plugin.major && host.minor == plugin.minor)
        return true;
    return false;
}

// Cleanup plugin resources when GCC finishes processing the compilation unit.
void plugin_deinit(void *gcc_data, void *user_data) {
    db.reset();
    (void) gcc_data;
    (void) user_data;
}

// Initialize the plugin, validate GCC compatibility, create the database,
// register plugin metadata, and install the lowering pass.
int plugin_init(struct plugin_name_args *plugin_info, struct plugin_gcc_version *version) {
    // Check the GCC version used to compile the plugin against the current GCC version. If they don't equal, the plugin aborts initialization
    if (!plugin_version_check(version)) {
        std::cout << "Error: Incompatible gcc/plugin versions" << std::endl;
        return 1;
    }
    DEBUG("%s plugin v%s loaded!", PLUGIN_NAME, PLUGIN_VERSION);

    db = std::make_unique<database>();

    // Register plugin deinit event
    register_callback(PLUGIN_NAME, PLUGIN_FINISH, plugin_deinit,NULL);

    register_plugin_info();

    // Register custom attribute
    register_callback(plugin_info->base_name, PLUGIN_ATTRIBUTES, register_uprint_attribute, nullptr);

    register_uprint_lower(db.get());

    return 0;
    (void) plugin_info;
}
