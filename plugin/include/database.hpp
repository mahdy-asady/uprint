#pragma once
#include <stdint.h>
#include <fstream>
#include <vector>
#include <string>

/**
 * @brief Database class for storing format strings and their associated argument sizes.
 *
 * This class manages a simple database that stores format strings used in uprint() calls.
 * Each format string is assigned a unique record ID (currently line number in the database file), which can be used for later retrieval.
 */

class database {
    public:
        database();

        uint32_t append(const std::string &fmt_str, const std::vector<uint8_t> &arg_sizes = {});

    private:
        uint32_t get_new_id(FILE *db_file);

        void write_escaped_fmt_str(FILE *db_file, const std::string &str);
};
