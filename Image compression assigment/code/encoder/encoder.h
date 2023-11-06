#ifndef ENCODER_H
#define ENCODER_H

#include <vector>
#include <cstdint>
#include <block.h>

class Encoder{
public:
    Encoder();
    ~Encoder();

    std::vector<Block> createBlocks(const std::vector<DataType>& input_img, int width, int height);

    std::vector<Block> dctBlocks(std::vector<Block> blocks);
    std::vector<Block> quantBlocks(std::vector<Block> dct_blocks, const std::vector<float>& quant_mat);
private:
    Block quantizize_block(Block block, const std::vector<float>& quant_mat);
};

#endif