#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sstream>

#include "database.hpp"
#include "config.hpp"

#define DB_FILE_NAME PLUGIN_NAME ".db"

database::database() {
    db_stream.open(DB_FILE_NAME);

    if (!db_stream.is_open()) {
        std::cerr << "Error opening database file " << DB_FILE_NAME << "." << std::endl;
        std::exit(1);
    }
}

database::~database() {
    if (db_stream.is_open()) {
        db_stream.close();
    }
}

uint32_t database::append(const std::string &fmt_str, const std::vector<uint8_t> &arg_sizes) {
    if (!db_stream.is_open()) {
        return ++last_record_id;
    }

    std::ostringstream record_stream;
    record_stream << fmt_str;

    for (size_t i = 0; i < arg_sizes.size(); ++i) {
        record_stream << ",";
        record_stream << +arg_sizes[i];
    }

    db_stream << record_stream.str() << "\n";
    db_stream.flush();
    return ++last_record_id;
}
