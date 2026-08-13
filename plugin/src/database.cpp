#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sstream>

#include "database.hpp"
#include "config.hpp"

#define DB_FILE_NAME PLUGIN_NAME ".db"

static void write_escaped_fmt_str(std::ostream &os, const std::string &str);

database::database() {
    db_stream.open(DB_FILE_NAME);

    if (!db_stream.is_open()) {
        std::cerr << "Error opening database file " << DB_FILE_NAME << "." << std::endl;
        std::exit(1);
    }

    // TODO: Store database & communication configurations in first line
    db_stream << "\n";
    db_stream.flush();
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
    write_escaped_fmt_str(db_stream, fmt_str);

    for (size_t i = 0; i < arg_sizes.size(); ++i) {
        record_stream << ",";
        record_stream << +arg_sizes[i];
    }

    db_stream << record_stream.str() << "\n";
    db_stream.flush();
    return ++last_record_id;
}


static void write_escaped_fmt_str(std::ostream &os, const std::string &str) {
    for (char c : str) {
        switch (c) {
            case '\\': os << "\\\\"; break;
            case ',':  os << "\\,";  break;
            case '\n': os << "\\n";  break;
            case '\r': os << "\\r";  break;
            case '\t': os << "\\t";  break;
            default:   os << c;      break;
        }
    }
}
