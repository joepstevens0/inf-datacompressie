#include <iostream>
#include <util.h>
#include "decoder.h"
#include <zigzag.h>
#include <BitStream.h>
#include <algorithm>
#include <configreader.h>
#include <rle.h>

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

void read_data_rle(util::BitStreamReader& reader, std::vector<DataType>& data){
    const uint32_t bits_per_zero = reader.get(32);
    const uint32_t bits_per_value = reader.get(32);

    std::vector<std::vector<RLE_encoded>> rle_blocks;
    std::vector<RLE_encoded> last_block;

    for (int i = 0;i < reader.get_size()*8/(bits_per_value + bits_per_zero);++i){
        uint32_t zeroes = reader.get(bits_per_zero);
        uint8_t sign = reader.get_bit();
        int32_t value = reader.get(bits_per_value-1);
        RLE_encoded r;
        r.zeroes = zeroes;
        r.value = value;
        if (sign){
            r.value = -r.value;
        }
        last_block.push_back(r);

        if (r.zeroes == 0 && r.value == 0){
            rle_blocks.push_back(last_block);
            last_block.clear();
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

void read_data(util::BitStreamReader& reader, std::vector<DataType>& data) {
    const uint32_t bits_per_value = reader.get(32);

    std::cout << "Bits per value:" << bits_per_value << "\n";

    for (int i = 0;i < reader.get_size()*8/bits_per_value;++i){
        uint8_t sign = reader.get_bit();
        uint32_t value = reader.get(bits_per_value-1);
        DataType r = static_cast<DataType>(value);
        if (sign){
            r = -r;
        }
        data.push_back(r);
    }
}

int main(int argc, char** argv){
    std::cout << "Decoder started\n";

    ConfigReader config{};
    Decoder decoder{};

    // Read configfile
    if (argc > 1) { 
        bool loaded = config.read(argv[1]);
        if (!loaded) {
            std::cerr << "Could not read configfile: " << argv[1] << "\n";
            std::cerr << "Error: " << config.getErrorDescription() << "\n";
            return -1;
        }
        config.printAll();
    }

    // Get inputfile
    std::string file_input = "";
    if (!config.getKeyValue("encfile",file_input)){
        std::cerr << "Failed to get encoded file." << "\n";
        std::cerr << config.getErrorDescription() << "\n";
        return -1;
    }
    // Get outputfile
    std::string file_output = "";
    if (!config.getKeyValue("decfile",file_output)){
        std::cerr << "Failed to get raw file." << "\n";
        std::cerr << config.getErrorDescription() << "\n";
        return -1;
    }

    // Read full encoded file
    const std::vector<uint8_t> data_uint8 = fileToInt8Vec(file_input);
    uint8_t data_uint8_p[data_uint8.size()];
    std::copy(data_uint8.begin(), data_uint8.end(), data_uint8_p);
    util::BitStreamReader reader{data_uint8_p, (int)data_uint8.size()};

    // Read settings from encoded file
    const uint32_t width = reader.get(32);
    const uint32_t height = reader.get(32);
    const bool rle = reader.get(1);
    std::vector<float> quant_mat{};
    for (int i = 0; i < Block::width*Block::height;++i){
        uint32_t t = reader.get(32);
        quant_mat.push_back(*(float*)&t);
    }

    // Read data from encoded file
    std::vector<DataType> data;
    if (rle) {
        read_data_rle(reader, data);
    } else {
        read_data(reader, data);
    }

    // transform data to blocks
    std::vector<Block> blocks = zigzagdata_to_blocks(data, width, height);
    std::cout << "Transformed file into " << blocks.size() << " blocks\n";

    // decode blocks
    blocks = decoder.decodeBlocks(blocks, quant_mat);
    
    // Create image
    std::vector<DataType> result = decoder.createImage(blocks, width, height);

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
        result_uint8.push_back(static_cast<uint8_t>(result[i]));
    }

    // Write result to outputfile
    int8VecToFile(file_output, result_uint8);
    std::cout << "Decoded to a file with size: " << result.size() << "\n";

    return 0;
}