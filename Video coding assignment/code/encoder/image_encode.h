#ifndef IMAGE_ENCODE_H
#define IMAGE_ENCODE_H

#include <cstdint>
#include <vector>

#include <iostream>
#include "image_encoder.h"
#include <util.h>
#include <BitStream.h>
#include <zigzag.h>
#include <configreader.h>
#include <logger.h>
#include <cmath>
#include <rle.h>
#include <algorithm>

uint32_t calc_required_bits_for_value(uint32_t max_value){
    // calc required bits
    uint32_t required_bits = 0;
    while (pow(2, required_bits) <= max_value) {
        required_bits += 1;
    }
    return required_bits;
}

std::vector<Block> encode_to_blocks(const std::vector<float>& data_float, const int image_width, const int image_height, const std::vector<float> &quant_mat, Logger &logger) {
    ImageEncoder encoder{};

    // divide into blocks
    std::vector<Block> blocks = encoder.createBlocks(data_float, image_width, image_height);
    std::cout << "Data divided into blocks\n";

    // encode blocks
    std::vector<Block> dct_blocks = encoder.dctBlocks(blocks);
    std::vector<Block> quant_blocks = encoder.quantBlocks(dct_blocks, quant_mat);
    std::cout << "Blocks encoded\n";

    // log block encoding
    logger.log_block_encoding(blocks, dct_blocks, quant_blocks);

    return quant_blocks;
}

std::vector<std::vector<int32_t>> encode_data(const std::vector<float>& data_float, const int image_width, const int image_height, const std::vector<float> &quant_mat, Logger &logger) {
    // encode into blocks
    std::vector<Block> encoded_blocks = encode_to_blocks(data_float, image_width, image_height, quant_mat, logger);

    // write into RLE
    std::vector<std::vector<int32_t>> r;
    for (int i = 0; i < encoded_blocks.size(); ++i) {
        std::vector<DataType> result;
        write_block_zigzag(encoded_blocks[i], result);

        // round result to int
        std::vector<int32_t> result_int;
        for (int j = 0; j < result.size(); ++j) {
            result_int.push_back((int32_t)round(result[j]));
        }

        r.push_back(result_int);
    }
    return r;
}

std::vector<std::vector<RLE_encoded>> encode_data_rle(const std::vector<float>& data_float, const int image_width, const int image_height, const std::vector<float> &quant_mat, Logger &logger) {
    std::vector<std::vector<int32_t>> zigzag_data = encode_data(data_float, image_width, image_height, quant_mat, logger);

    // RLE encode
    std::vector<std::vector<RLE_encoded>> r;
    for (int i = 0; i < zigzag_data.size(); ++i) {
        std::vector<RLE_encoded> result_rle = rle_encode(zigzag_data[i]);
        r.push_back(result_rle);
    }
    return r;
}

void write_rle_blocks(util::BitStreamWriter &writer, std::vector<std::vector<RLE_encoded>> rle_blocks) {
    std::vector<uint32_t> required_dc_value_bits_per_block;
    std::vector<uint32_t> required_dc_zeroes_bits_per_block;
    std::vector<uint32_t> required_ac_value_bits_per_block;
    std::vector<uint32_t> required_ac_zeroes_bits_per_block;
    for (int i = 0; i < rle_blocks.size(); ++i) {

        uint32_t required_dc_zeroes_bits = calc_required_bits_for_value(rle_blocks[i][0].zeroes);
        uint32_t required_dc_value_bits = calc_required_bits_for_value(abs(rle_blocks[i][0].value));

        uint32_t max_value_ac_zeroes = 0;
        uint32_t max_value_ac_values = 0;
        for (int j = 1; j < rle_blocks[i].size(); ++j) {
            if (abs(rle_blocks[i][j].value) > max_value_ac_values)
                max_value_ac_values = abs(rle_blocks[i][j].value);
            if (rle_blocks[i][j].zeroes > max_value_ac_zeroes)
                max_value_ac_zeroes = rle_blocks[i][j].zeroes;
        }
        uint32_t required_ac_value_bits = calc_required_bits_for_value(max_value_ac_values);
        uint32_t required_ac_zeroes_bits = calc_required_bits_for_value(max_value_ac_zeroes);

        required_dc_value_bits += 1; // sign bit
        required_ac_value_bits += 1; // sign bit

        required_dc_value_bits_per_block.push_back(required_dc_value_bits);
        required_dc_zeroes_bits_per_block.push_back(required_dc_zeroes_bits);
        required_ac_value_bits_per_block.push_back(required_ac_value_bits);
        required_ac_zeroes_bits_per_block.push_back(required_ac_zeroes_bits);
    }

    uint32_t max_required_dc_zeroes_bits_value = *std::max_element(std::begin(required_dc_zeroes_bits_per_block), std::end(required_dc_zeroes_bits_per_block));
    uint32_t max_required_dc_values_bits_value = *std::max_element(std::begin(required_dc_value_bits_per_block), std::end(required_dc_value_bits_per_block));
    uint32_t max_required_ac_zeroes_bits_value = *std::max_element(std::begin(required_ac_zeroes_bits_per_block), std::end(required_ac_zeroes_bits_per_block));
    uint32_t max_required_ac_values_bits_value = *std::max_element(std::begin(required_ac_value_bits_per_block), std::end(required_ac_value_bits_per_block));

    uint32_t required_dc_bits_zeroes_required_bits = calc_required_bits_for_value(max_required_dc_zeroes_bits_value);
    uint32_t required_dc_bits_values_required_bits = calc_required_bits_for_value(max_required_dc_values_bits_value);
    uint32_t required_ac_bits_zeroes_required_bits = calc_required_bits_for_value(max_required_ac_zeroes_bits_value);
    uint32_t required_ac_bits_values_required_bits = calc_required_bits_for_value(max_required_ac_values_bits_value);

    // write bit settings
    writer.put(32, required_dc_bits_zeroes_required_bits);
    writer.put(32, required_dc_bits_values_required_bits);
    writer.put(32, required_ac_bits_zeroes_required_bits);
    writer.put(32, required_ac_bits_values_required_bits);

    printf("Bits needed to encode required zeroes is DC(%u) AC(%u)\n", required_dc_bits_zeroes_required_bits, required_ac_bits_zeroes_required_bits);
    printf("Bits needed to encode required values is DC(%u) AC(%u)\n", required_dc_bits_values_required_bits, required_ac_bits_values_required_bits);

    // Write result to outputfile
    for (int i = 0; i < rle_blocks.size(); ++i) {

        // write bits used for AC and DC
        uint32_t required_dc_zeroes_bits = required_dc_zeroes_bits_per_block[i];
        uint32_t required_dc_values_bits = required_dc_value_bits_per_block[i];
        uint32_t required_ac_zeroes_bits = required_ac_zeroes_bits_per_block[i];
        uint32_t required_ac_values_bits = required_ac_value_bits_per_block[i];
        writer.put(required_dc_bits_zeroes_required_bits, required_dc_zeroes_bits);
        writer.put(required_dc_bits_values_required_bits, required_dc_values_bits);
        writer.put(required_ac_bits_zeroes_required_bits, required_ac_zeroes_bits);
        writer.put(required_ac_bits_values_required_bits, required_ac_values_bits);

        // encode with RLE
        const std::vector<RLE_encoded>& block_rle = rle_blocks[i];

        // write DC
        {
            // write amount of zeroes
            writer.put(required_dc_zeroes_bits, block_rle[0].zeroes);

            // write sign bit of value
            if (block_rle[0].value <= 0) {
                writer.put(1, 1);
            }
            else {
                writer.put(1, 0);
            }
            writer.put(required_dc_values_bits - 1, static_cast<uint32_t>(abs(block_rle[0].value)));
        }

        // write AC
        for (int j = 1; j < block_rle.size(); ++j) {
            // write amount of zeroes
            writer.put(required_ac_zeroes_bits, block_rle[j].zeroes);

            // write sign bit of value
            if (block_rle[j].value <= 0) {
                writer.put(1, 1);
            }
            else {
                writer.put(1, 0);
            }
            writer.put(required_ac_values_bits - 1, static_cast<uint32_t>(abs(block_rle[j].value)));
        }
    }
}

void write_zigzag_data(util::BitStreamWriter &writer, std::vector<std::vector<int32_t>> zigzag_data) {
    std::vector<uint32_t> required_dc_bits_per_block;
    std::vector<uint32_t> required_ac_bits_per_block;
    for (int i = 0; i < zigzag_data.size(); ++i) {
        uint32_t required_dc_bits = calc_required_bits_for_value(abs(zigzag_data[i][0]));

        uint32_t max_value_ac = 0;
        for (int j = 1; j < zigzag_data[i].size(); ++j) {
            if (abs(zigzag_data[i][j]) > max_value_ac)
                max_value_ac = abs(zigzag_data[i][j]);
        }
        uint32_t required_ac_bits = calc_required_bits_for_value(max_value_ac);

        required_dc_bits += 1; // sign bit
        required_ac_bits += 1; // sign bit

        required_dc_bits_per_block.push_back(required_dc_bits);
        required_ac_bits_per_block.push_back(required_ac_bits);
    }

    uint32_t max_required_dc_bits_value = *std::max_element(std::begin(required_dc_bits_per_block), std::end(required_dc_bits_per_block));
    uint32_t max_required_ac_bits_value = *std::max_element(std::begin(required_ac_bits_per_block), std::end(required_ac_bits_per_block));
    uint32_t required_dc_bits_required_bits = calc_required_bits_for_value(max_required_dc_bits_value);
    uint32_t required_ac_bits_required_bits = calc_required_bits_for_value(max_required_ac_bits_value);

    writer.put(32, required_dc_bits_required_bits);
    writer.put(32, required_ac_bits_required_bits);

    printf("Bits needed to encode required values is DC(%u) AC(%u)\n", required_dc_bits_required_bits, required_ac_bits_required_bits);

    // Write result to outputfile
    for (int i = 0; i < zigzag_data.size(); ++i) {
        uint32_t required_dc_bits = required_dc_bits_per_block[i];
        uint32_t required_ac_bits = required_ac_bits_per_block[i];
        
        writer.put(required_dc_bits_required_bits, required_dc_bits);
        writer.put(required_ac_bits_required_bits, required_ac_bits);

        if (zigzag_data[i][0] < 0) {
            writer.put(1, 1);
        } else {
            writer.put(1, 0);
        }
        writer.put(required_dc_bits - 1, static_cast<uint32_t>(abs(zigzag_data[i][0])));

        for (int j = 1; j < zigzag_data[i].size(); ++j) {
            // write sign bit of value
            if (zigzag_data[i][j] < 0) {
                writer.put(1, 1);
            } else {
                writer.put(1, 0);
            }
            writer.put(required_ac_bits - 1, static_cast<uint32_t>(abs(zigzag_data[i][j])));
        }
    }
}

void image_encode_float(std::vector<float>& data_float, uint32_t image_width, uint32_t image_height, bool rle, std::vector<float>& quant_mat, Logger& logger, util::BitStreamWriter& writer){
    if (rle) {
        // encode data
        const std::vector<std::vector<RLE_encoded>> rle_blocks = encode_data_rle(
            data_float, image_width, image_height, quant_mat, logger);

        // write encoded blocks
        write_rle_blocks(writer, rle_blocks);

    } else {
        // encode data
        const std::vector<std::vector<int32_t>> data = encode_data(
            data_float, image_width, image_height, quant_mat, logger);

        // write encoded blocks
        write_zigzag_data(writer, data);
    }
}

void image_encode(const std::vector<uint8_t>& data_uint8, uint32_t image_width, uint32_t image_height, bool rle, std::vector<float>& quant_mat, Logger& logger, util::BitStreamWriter& writer) {

    // transform into datatype
    std::vector<DataType> data_float;
    for (int i = 0; i < data_uint8.size(); ++i) {
        data_float.push_back(static_cast<DataType>(data_uint8[i]));
    }
    image_encode_float(data_float ,image_width, image_height, rle, quant_mat, logger, writer);
}

#endif