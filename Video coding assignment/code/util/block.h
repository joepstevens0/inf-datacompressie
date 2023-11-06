#ifndef BLOCK_H
#define BLOCK_H

#include <vector>
#include <cstdint>

using DataType = float;


struct Block{
    std::vector<DataType> data;
    static const int width = 4;
    static const int height = 4;
    DataType getElement(int x, int y) const {
        return data[y*width+x];
    }

    void setElement(int x, int y, DataType value) {
        data[y*width+x] = value;
    }
};

#endif