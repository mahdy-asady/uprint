#pragma once
#include <stdint.h>
#include <fstream>

class database {
    public:
        database();
        ~database();

        uint32_t append(const std::string &str);

    private:
        uint32_t last_record_id = 0;
        std::ofstream db_stream;
};
