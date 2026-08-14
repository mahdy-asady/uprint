#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>

#include "database.hpp"
#include "config.hpp"

#define DB_FILE_NAME PLUGIN_NAME ".db"

database::database() {
    std::ifstream check_file(DB_FILE_NAME, std::ios::ate);
    bool file_is_empty_or_new = !check_file.is_open() || (check_file.tellg() <= 0);
    check_file.close();


    std::ofstream db_stream(DB_FILE_NAME, std::ios::out | std::ios::app);
    if (!db_stream.is_open()) {
        std::cerr << "Error opening database file " << DB_FILE_NAME << "." << std::endl;
        std::exit(1);
    }

    if (file_is_empty_or_new) {
        // TODO: Store database & communication configurations in first line
        db_stream << "\n";
        db_stream.flush();
    }
    db_stream.close();
}

uint32_t database::append(const std::string &fmt_str, const std::vector<uint8_t> &arg_sizes) {
    int fd = open(DB_FILE_NAME, O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        return 0;
    }

    flock(fd, LOCK_EX);

    FILE *file = fdopen(fd, "r+");
    if (!file) {
        return 0;
    }

    uint32_t assigned_id = get_new_id(file);

    fseek(file, 0, SEEK_END);

    write_escaped_fmt_str(file, fmt_str);

    for (size_t i = 0; i < arg_sizes.size(); ++i) {
        fprintf(file, ",%u", arg_sizes[i]);
    }
    fputs("\n", file);
    fflush(file);

    flock(fd, LOCK_UN);
    fclose(file);

    return assigned_id;
}

uint32_t database::get_new_id(FILE *db_file) {
    uint32_t line_count = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), db_file)) {
        line_count++;
    }
    return line_count;
}

void database::write_escaped_fmt_str(FILE *db_file, const std::string &str) {
    for (char c : str) {
        switch (c) {
            case '\\': fputs("\\\\", db_file); break;
            case ',':  fputs("\\,", db_file);  break;
            case '\n': fputs("\\n", db_file);  break;
            case '\r': fputs("\\r", db_file);  break;
            case '\t': fputs("\\t", db_file);  break;
            default:   fputc(c, db_file);      break;
        }
    }
}
