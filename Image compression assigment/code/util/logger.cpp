#include "logger.h"



Logger::Logger(std::string path){
    m_out = std::ofstream(path);
}

Logger::~Logger(){
    m_out.close();
}

void Logger::write_block(Block block){
    for (int y = 0; y < Block::height;++y){
        for (int x = 0; x < Block::width;++x){
            m_out << (block.getElement(x,y)) << " | ";
        }
        m_out << "\n";
    }
}

void Logger::log_block_encoding(const std::vector<Block>& blocks, const std::vector<Block>& dct_blocks, const std::vector<Block>& quant_blocks){

    for (int i = 0; i < blocks.size();++i){
        m_out << "Block[" << i << "]:\n";
        write_block(blocks[i]);
        m_out << "\nBlock encoded dct[" << i << "]:\n";
        write_block(dct_blocks[i]);
        m_out << "\nBlock quantized[" << i << "]:\n";
        write_block(quant_blocks[i]);
        m_out << "\n\n-------------------\n\n";
    }
}