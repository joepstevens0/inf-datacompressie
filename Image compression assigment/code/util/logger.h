#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include "block.h"

class Logger{
public:
    Logger(std::string path);
    ~Logger();


    void log_block_encoding(const std::vector<Block>& blocks, const std::vector<Block>& dct_blocks, const std::vector<Block>& quant_blocks);

    void write_block(Block block);
private:
    std::ofstream m_out;
};

#endif