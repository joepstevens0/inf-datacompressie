#ifndef RLE_H
#define RLE_H

#include <block.h>
#include <vector>

struct RLE_encoded{
    uint32_t zeroes;
    int32_t value;
};

std::vector<RLE_encoded> rle_encode(std::vector<int32_t> values_in_zigzag){
    std::vector<RLE_encoded> result;
    uint32_t zeroes = 0;
    for (int i = 0; i < values_in_zigzag.size();++i){
        if (values_in_zigzag[i] != 0){
            result.push_back(RLE_encoded{zeroes, values_in_zigzag[i]});
            zeroes = 0;
        } else{
            zeroes += 1;
        }
    }
    result.push_back(RLE_encoded{0,0});
    return result;
}

std::vector<int32_t> rle_decode(std::vector<RLE_encoded> encoded_values){
    std::vector<int32_t> result;

    for (int i = 0; i < encoded_values.size() - 1;++i){ // does not look at (0,0)
        for (int j = 0; j < encoded_values[i].zeroes;++j){
            result.push_back(0);
        }
        result.push_back(encoded_values[i].value);
    }


    while (result.size() < Block::width*Block::height){
        result.push_back(0);
    }
    return result;
}

#endif