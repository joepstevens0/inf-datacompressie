#include "decoder.h"
#include <iostream>
#include <dct.h>

Decoder::Decoder() {
}
Decoder::~Decoder() {
}

std::vector<DataType> Decoder::createImage(std::vector<Block>& blocks, int width, int height) {
    std::vector<DataType> image{};
    int num_blocks_in_row = width / Block::width;

    for (int i = 0; i < blocks.size(); i += num_blocks_in_row) {    // i = index of first block in row i
        // Read 4 rows aka #num_blocks_in_row full blocks
        for (int k = 0; k < Block::height; k++) {       // i*4 + k = row in the image
            std::vector<DataType> row{};
            for (int l = 0; l < num_blocks_in_row; l++) {   // l = block in row
                std::vector<DataType> part {&blocks[i+l].data[k*Block::width], &blocks[i+l].data[(k*Block::width)+Block::width]};
                row.insert(row.end(), part.begin(), part.end());
            }
            image.insert(image.end(), row.begin(), row.end());
        }
    }

    return image;
}

std::vector<Block> Decoder::decodeBlocks(std::vector<Block> blocks, const std::vector<float>& quant_mat){
    for (int i = 0; i < blocks.size();++i){
        blocks[i] = dequantizize_block(blocks[i], quant_mat);
        blocks[i] = inverse_dct(blocks[i]);
    }
    return blocks;
}

Block Decoder::dequantizize_block(Block block, const std::vector<float>& quant_mat){
    for (int y = 0; y < Block::height; ++y) {
        for (int x = 0; x < Block::width; ++x) {
            DataType new_val = block.getElement(x,y)*mat_get_Element(quant_mat, x,y);
            block.setElement(x,y,new_val);
        }
    }
    return block;
}
