#ifndef IMAGE_DECODE_H
#define IMAGE_DECODE_H

#include <iostream>
#include <util.h>
#include "image_decoder.h"
#include <zigzag.h>
#include <BitStream.h>
#include <algorithm>
#include <configreader.h>
#include <rle.h>
#include <math.h>

std::vector<Block> zigzagdata_to_blocks(const std::vector<DataType> data, const int image_width, const int image_height){
    // transform data to blocks
    std::vector<Block> blocks;
    int block_size = Block::width*Block::height;
    int total_blocks = (image_width*image_height)/block_size;
    for (int i = 0; i < total_blocks;++i){
        const DataType* block_start_addr = &data[i*block_size];

        std::vector<DataType> data;

        for (int j = 0; j < block_size;++j){
            data.push_back(block_start_addr[j]);
        }
        Block block{};
        read_block_zigzag(data, block);
        blocks.push_back(block);
    }

    return blocks;
}

void read_data_rle(util::BitStreamReader& reader, std::vector<DataType>& data, uint32_t image_width, uint32_t image_height, uint32_t total_frame_bits){
    uint32_t bits_per_bits_per_zeroes_dc = reader.get(32);
    uint32_t bits_per_bits_per_value_dc = reader.get(32);
    uint32_t bits_per_bits_per_zeroes_ac = reader.get(32);
    uint32_t bits_per_bits_per_value_ac = reader.get(32);

    std::vector<std::vector<RLE_encoded>> rle_blocks;
    std::vector<RLE_encoded> last_block;

    uint32_t total_rle_bits = total_frame_bits - 32 - 32 - 32 - 32; // total bits - bits per zero - bits per value

    // int total_value_pairs = total_rle_bits/(bits_per_value + bits_per_zero);

    bool dcRead = true;
    uint32_t bits_per_zeroes_dc;
    uint32_t bits_per_value_dc;
    uint32_t bits_per_zeroes_ac;
    uint32_t bits_per_value_ac;
    while (rle_blocks.size() < (image_height*image_width/(Block::width*Block::height))){

        RLE_encoded r;
        
        if (dcRead) {
            dcRead = false;

            // read bits per value
            bits_per_zeroes_dc = reader.get(bits_per_bits_per_zeroes_dc);
            bits_per_value_dc = reader.get(bits_per_bits_per_value_dc);
            bits_per_zeroes_ac = reader.get(bits_per_bits_per_zeroes_ac);
            bits_per_value_ac = reader.get(bits_per_bits_per_value_ac);

            // Read DC
            r.zeroes = reader.get(bits_per_zeroes_dc);
            uint8_t sign = reader.get_bit();
            r.value = reader.get(bits_per_value_dc-1);
            if (sign){
                r.value = -r.value;
            }
        } else {
            // READ AC
            r.zeroes = reader.get(bits_per_zeroes_ac);
            uint8_t sign = reader.get_bit();
            r.value = reader.get(bits_per_value_ac-1);
            if (sign){
                r.value = -r.value;
            }
        }
        last_block.push_back(r);

        if (r.zeroes == 0 && r.value == 0){
            rle_blocks.push_back(last_block);
            last_block.clear();
            dcRead = true;
        }
    }

    // RLE decoding
    for (int i = 0; i < rle_blocks.size();++i){
        std::vector<int32_t> r = rle_decode(rle_blocks[i]);
        for (int j = 0; j < r.size();++j){
            data.push_back(static_cast<DataType>(r[j]));
        }
    }
}

void read_data(util::BitStreamReader& reader, std::vector<DataType>& data, uint32_t image_width, uint32_t image_height) {
    uint32_t bits_per_bits_per_value_dc = reader.get(32);
    uint32_t bits_per_bits_per_value_ac = reader.get(32);
    uint32_t total_values = image_width*image_height;

    printf("Bits per value of bits per value DC(%u) AC(%u), total values: %u\n", bits_per_bits_per_value_dc, bits_per_bits_per_value_ac, total_values);

    uint32_t bits_per_value_dc;
    uint32_t bits_per_value_ac;
    for (int i = 0;i < total_values;++i){
        uint8_t sign;
        uint32_t value;
        if ((i % (Block::width*Block::height)) == 0){
            bits_per_value_dc = reader.get(bits_per_bits_per_value_dc);
            bits_per_value_ac = reader.get(bits_per_bits_per_value_ac);
            sign = reader.get_bit();
            value = reader.get(bits_per_value_dc-1);
        }
        else{
            sign = reader.get_bit();
            value = reader.get(bits_per_value_ac-1);
        }
        
        DataType r = static_cast<DataType>(value);
        if (sign){
            r = -r;
        }
        data.push_back(r);
    }
}

std::vector<float> image_decode_float(util::BitStreamReader& reader, uint32_t total_frame_bits, int width,int height, bool rle, const std::vector<float>& quant_mat) {
    // Read data from encoded file
    std::vector<DataType> data;
    if (rle) {
        read_data_rle(reader, data, width, height, total_frame_bits);
    } else {
        read_data(reader, data, width, height);
    }

    std::cout << "Read data size:" << data.size() << "\n";

    // transform data to blocks
    std::vector<Block> blocks = zigzagdata_to_blocks(data, width, height);
    std::cout << "Transformed file into " << blocks.size() << " blocks\n";

    ImageDecoder decoder{};

    // decode blocks
    blocks = decoder.decodeBlocks(blocks, quant_mat);
    
    // Create image
    std::vector<DataType> result = decoder.createImage(blocks, width, height);

    return result;
}

std::vector<uint8_t> image_decode(util::BitStreamReader& reader, uint32_t total_frame_bits, int width,int height, bool rle, const std::vector<float>& quant_mat) {
    // Create image
    std::vector<DataType> result = image_decode_float(reader, total_frame_bits, width, height, rle, quant_mat);

    // Convert imagedata to uint8_t
    std::vector<uint8_t> result_uint8;
    for (int i = 0;i < result.size();++i){
        if (result[i] > 255){
            // std::cerr << "Value "<< result[i] << "is larger than 255\n";
            // STOP 8-BIT OVERFLOW
            result[i] = 255.;
        }
        if (result[i] < 0){
            // std::cerr << "Value "<< result[i] << "is lower than 0\n";
            // STOP 8-BIT UNDERFLOW
            result[i] = 0.;
        }
        result_uint8.push_back(static_cast<uint8_t>(round(result[i])));
    }
    return result_uint8;
}

#endif