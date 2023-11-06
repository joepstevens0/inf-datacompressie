#ifndef ZIGZAG_H
#define ZIGZAG_H

#include "block.h"
#include <algorithm>

struct Point{
    int x;
    int y;
};

Point write_stair_ascending(Point start_pos, const Block& block, std::vector<DataType>& result){
    int x = start_pos.x;
    int y = start_pos.y;

    while (y >= 0 && x < Block::width){
        // std::cout << "Write asc pos: " << x << "|" << y << "\n";
        // std::cout << unsigned(block.getElement(x,y)) << "\n";
        result.push_back(block.getElement(x,y));
        // std::cout << "success\n";
        x += 1;
        y -= 1;

    }

    x -= 1;
    y += 1;
    
    return Point{x,y};
}

Point write_stair_descending(Point start_pos, const Block& block, std::vector<DataType>& result){
    int x = start_pos.x;
    int y = start_pos.y;

    while (x >= 0 && y < Block::height){
        // std::cout << "Write desc pos: " << x << "|" << y << "\n";
        // std::cout << unsigned(block.getElement(x,y)) << "\n";
        result.push_back(block.getElement(x,y));
        // std::cout << "success\n";
        x -= 1;
        y += 1;
    }

    x += 1;
    y -= 1;

    return Point{x,y};
}

void write_block_zigzag(const Block& block, std::vector<DataType>& result){
    Point pos{0,0};

    bool descending = false;
    while (pos.x < Block::width && pos.y < Block::height){
        if (!descending){
            pos = write_stair_ascending(pos, block, result);
            if (pos.x < Block::width-1) {
                pos.x += 1;
            } else {
                pos.y += 1;
            }
        } else {
            pos = write_stair_descending(pos, block, result);
            if (pos.y < Block::height-1) {
                pos.y += 1;
            } else {
                pos.x += 1;
            }
        }

        descending = !descending;
    }
}

Point read_stair_ascending(Point start_pos, std::vector<DataType>& data, Block& result){
    int x = start_pos.x;
    int y = start_pos.y;

    while (y >= 0 && x < Block::width){
        // std::cout << "Write asc pos: " << x << "|" << y << "\n";
        // std::cout << unsigned(block.getElement(x,y)) << "\n";
        result.setElement(x,y, data[data.size() - 1]);
        data.pop_back();
        // std::cout << "success\n";
        x += 1;
        y -= 1;

    }

    x -= 1;
    y += 1;
    
    return Point{x,y};
}

Point read_stair_descending(Point start_pos, std::vector<DataType>& data, Block& result){
    int x = start_pos.x;
    int y = start_pos.y;

    while (x >= 0 && y < Block::height){
        // std::cout << "Write desc pos: " << x << "|" << y << "\n";
        // std::cout << unsigned(block.getElement(x,y)) << "\n";
        result.setElement(x,y, data[data.size() - 1]);
        data.pop_back();
        // std::cout << "success\n";
        x -= 1;
        y += 1;
    }

    x += 1;
    y -= 1;

    return Point{x,y};
}

void read_block_zigzag(std::vector<DataType>& data, Block& result){
    result.data.resize(Block::width*Block::height);

    std::reverse(data.begin(), data.end());
    
    Point pos{0,0};

    bool descending = false;
    while (pos.x < Block::width && pos.y < Block::height){
        if (!descending){
            pos = read_stair_ascending(pos, data, result);
            if (pos.x < Block::width-1) {
                pos.x += 1;
            } else {
                pos.y += 1;
            }
        } else {
            pos = read_stair_descending(pos, data, result);
            if (pos.y < Block::height-1) {
                pos.y += 1;
            } else {
                pos.x += 1;
            }
        }

        descending = !descending;
    }
}

#endif