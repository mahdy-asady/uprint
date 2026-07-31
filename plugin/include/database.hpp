#pragma once
#include <stdint.h>
#include <fstream>

class database {
    public:
        database();
        ~database();

        uint32_t append();

    private:
        uint32_t last_record_id = 0;
        std::ofstream db_stream;
};
