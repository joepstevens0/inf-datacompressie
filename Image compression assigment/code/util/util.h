#ifndef UTIL_H
#define UTIL_H

#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <block.h>

std::vector<uint8_t> fileToInt8Vec(std::string file_path){
    std::ifstream instream(file_path, std::ios::in | std::ios::binary);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
    return data;
}

void int8VecToFile(std::string file_path, std::vector<uint8_t> data){
    std::ofstream outstream(file_path, std::ios::out | std::ios::binary);
    outstream.write((const char*)&data[0], data.size()*sizeof(uint8_t));
    outstream.close();
}

std::vector<float> fileToMatrix(std::string file_path){
    std::ifstream instream(file_path, std::ios::in);

    std::vector<float> result;

    for (int x = 0;x < Block::width;++x){
        for (int y = 0; y< Block::height;++y){
            float value;
            instream >> value;
            result.push_back(value);
        }
    }

    return result;
}


#endif