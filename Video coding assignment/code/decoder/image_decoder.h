#ifndef IMAGEDECODER_H
#define IMAGEDECODER_H

#include <vector>
#include <cstdint>
#include <block.h>

class ImageDecoder {
public:
    ImageDecoder();
    ~ImageDecoder();

    std::vector<DataType> createImage(std::vector<Block>& blocks, int width, int height);
    std::vector<Block> decodeBlocks(std::vector<Block> blocks, const std::vector<float>& quant_mat);
private:
    Block dequantizize_block(Block block, const std::vector<float>& quant_mat);
};

#endif