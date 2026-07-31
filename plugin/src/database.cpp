#include <iostream>
#include <cstdio>
#include <cstdlib>
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

uint32_t database::append() {
    if (!db_stream.is_open()) {
        return ++last_record_id;
    }

    db_stream << "New Record\n";
    db_stream.flush();
    return ++last_record_id;
}
