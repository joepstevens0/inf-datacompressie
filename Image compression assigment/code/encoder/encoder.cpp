#include "encoder.h"
#include <valarray>
#include <dct.h>

Encoder::Encoder() {
}
Encoder::~Encoder() {
}

std::vector<Block> Encoder::createBlocks(const std::vector<DataType>& input_img, int width, int height) {
    std::vector<Block> blocks{};

    for (int i = 0; i < height; i += Block::height) {
        for (int j = 0; j < width; j += Block::width) {
            Block block{};
            for (int k = 0; k < Block::height; k++) {
                std::vector<DataType> part {&input_img[(i+k)*width+j], &input_img[(i+k)*width+j+Block::width]};
                block.data.insert(block.data.end(), part.begin(), part.end());
            }
            blocks.push_back(block);
        }
    }

    return blocks;
}

std::vector<Block> Encoder::dctBlocks(std::vector<Block> blocks){
    for (int i = 0; i < blocks.size();++i){
        blocks[i] = forward_dct(blocks[i]);
    }
    return blocks;
}

std::vector<Block> Encoder::quantBlocks(std::vector<Block> dct_blocks, const std::vector<float>& quant_mat){
    for (int i = 0; i < dct_blocks.size();++i){
        dct_blocks[i] = quantizize_block(dct_blocks[i], quant_mat);
    }
    return dct_blocks;
}

Block Encoder::quantizize_block(Block block, const std::vector<float>& quant_mat){
    for (int y = 0; y < Block::height; ++y) {
        for (int x = 0; x < Block::width; ++x) {
            DataType new_val = block.getElement(x,y)/mat_get_Element(quant_mat, x,y);
            block.setElement(x,y,new_val);
        }
    }
    return block;
}