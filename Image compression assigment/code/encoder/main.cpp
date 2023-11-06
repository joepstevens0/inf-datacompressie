
#include <iostream>

#include "encoder.h"
#include <util.h>
#include <BitStream.h>
#include <zigzag.h>
#include <configreader.h>
#include <logger.h>
#include <cmath>
#include <rle.h>


std::vector<Block> encode_to_blocks(const std::vector<uint8_t> data_uint8, const int image_width, const int image_height, const std::vector<float>& quant_mat, Logger& logger){
    Encoder encoder{};
    
    // transform into datatype
    std::vector<DataType> data;
    for (int i = 0;i < data_uint8.size();++i){
        data.push_back(static_cast<DataType>(data_uint8[i]));
    }

    // divide into blocks
    std::vector<Block> blocks = encoder.createBlocks(data, image_width, image_height);
    std::cout << "Data divided into blocks\n";

    // encode blocks
    std::vector<Block> dct_blocks = encoder.dctBlocks(blocks);
    std::vector<Block> quant_blocks = encoder.quantBlocks(dct_blocks, quant_mat);
    std::cout << "Blocks encoded\n";

    // log block encoding
    logger.log_block_encoding(blocks, dct_blocks, quant_blocks);

    return quant_blocks;
}

std::vector<std::vector<int32_t>> encode_data(const std::vector<uint8_t> data_uint8, const int image_width, const int image_height, const std::vector<float>& quant_mat, Logger& logger){
    // encode into blocks
    std::vector<Block> encoded_blocks = encode_to_blocks(data_uint8, image_width, image_height, quant_mat, logger);

    // write into RLE
    std::vector<std::vector<int32_t>> r;
    for (int i = 0; i < encoded_blocks.size();++i){
        std::vector<DataType> result;
        write_block_zigzag(encoded_blocks[i], result);

        
        // round result to int
        std::vector<int32_t> result_int;
        for (int j = 0;j < result.size();++j){
            result_int.push_back((int32_t)result[j]);
        }
        
        r.push_back(result_int);
    }
    return r;
}

std::vector<std::vector<RLE_encoded>> encode_data_rle(const std::vector<uint8_t> data_uint8, const int image_width, const int image_height, const std::vector<float>& quant_mat, Logger& logger){
    std::vector<std::vector<int32_t>> zigzag_data = encode_data(data_uint8, image_width, image_height, quant_mat, logger);
    
    // RLE encode
    std::vector<std::vector<RLE_encoded>> r;
    for (int i = 0; i < zigzag_data.size();++i){
        std::vector<RLE_encoded> result_rle = rle_encode(zigzag_data[i]);
        r.push_back(result_rle);
    }
    return r;
}

void write_rle_blocks(util::BitStreamWriter& writer, std::vector<std::vector<RLE_encoded>> rle_blocks){
    // get max value and zero value in all blocks
    int32_t max_value = 0.;
    uint32_t max_zeroes = 0.;
    for (int i = 0; i < rle_blocks.size();++i){
        for (int j = 0;j < rle_blocks[i].size();++j){
            if (abs(rle_blocks[i][j].value) > max_value){
                max_value = abs(rle_blocks[i][j].value);
            }
            if (rle_blocks[i][j].zeroes > max_zeroes){
                max_zeroes = rle_blocks[i][j].zeroes;
            }
        }
    }

    // calc required bits for value
    uint32_t required_bits_value = 0;
    while (pow(2, required_bits_value) < max_value){
        required_bits_value += 1;
    }

    // calc required bits for zeroes
    uint32_t required_bits_zeroes = 0;
    while (pow(2, required_bits_zeroes) < max_zeroes){
        required_bits_zeroes += 1;
    }

    // write bit settings
    writer.put(32, required_bits_zeroes);
    writer.put(32, required_bits_value + 1);

    std::cout << "Bits needed to encode zeroes is: " << required_bits_zeroes << "\n";
    std::cout << "Bits needed to encode values is: " << required_bits_value << "\n";
    
    // Write result to outputfile
    for (int i = 0; i < rle_blocks.size();++i){

        // encode with RLE
        std::vector<RLE_encoded> block_rle = rle_blocks[i];
        for (int j = 0;j < block_rle.size();++j){
            // write amount of zeroes
            writer.put(required_bits_zeroes, block_rle[j].zeroes);

            // write sign bit of value
            if (block_rle[j].value < 0){
                writer.put(1,1);
            } else {
                writer.put(1,0);
            }
            writer.put(required_bits_value, static_cast<uint32_t>(abs(block_rle[j].value)));
        }
    }
}

void write_zigzag_data(util::BitStreamWriter& writer, std::vector<std::vector<int32_t>> zigzag_data){
    // get max value in all blocks
    float max_value = 0.;
    for (int i = 0; i < zigzag_data.size();++i){
        for (int j = 0;j < zigzag_data[i].size();++j){
            if (abs(zigzag_data[i][j]) > max_value)
                max_value = abs(zigzag_data[i][j]);
        }
    }

    // calc required bits
    uint32_t required_bits = 0;
    while (pow(2, required_bits) < max_value){
        required_bits += 1;
    }

    writer.put(32, required_bits + 1);

    std::cout << "Bits needed to encode values is: " << required_bits << "\n";

    // Write result to outputfile
    for (int i = 0; i < zigzag_data.size();++i){
        for (int j = 0; j < zigzag_data[i].size();++j){
            // write sign bit of value
            if (zigzag_data[i][j] < 0){
                writer.put(1,1);
            } else {
                writer.put(1,0);
            }
            writer.put(required_bits, static_cast<uint32_t>(abs(zigzag_data[i][j])));
        }
    }
}

int main(int argc, char** argv){

    std::cout << "Encoder started\n";

    ConfigReader config{};
    Encoder encoder{};

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
    if (!config.getKeyValue("rawfile",file_input)){
        std::cerr << "Failed to get raw file." << "\n";
        std::cerr << config.getErrorDescription() << "\n";
        return -1;
    }


    // Get outputfile
    std::string file_output = "";
    if (!config.getKeyValue("encfile",file_output)){
        std::cerr << "Failed to get encoded file." << "\n";
        std::cerr << config.getErrorDescription() << "\n";
        return -1;
    }

    // Get logfile
    std::string logfile = "";
    if (!config.getKeyValue("logfile",logfile)){
        std::cerr << "Failed to get logfile." << "\n";
        std::cerr << config.getErrorDescription() << "\n";
        return -1;
    }
    Logger logger(logfile);


    // Get width and height
    std::string widthstring{};
    std::string heightstring{};
    if (!config.getKeyValue("width",widthstring)){
        std::cerr << "Failed to get width." << "\n";
        std::cerr << config.getErrorDescription() << "\n";
        return -1;
    }
    if (!config.getKeyValue("height",heightstring)){
        std::cerr << "Failed to get height." << "\n";
        std::cerr << config.getErrorDescription() << "\n";
        return -1;
    }
    uint32_t width = std::stoi(widthstring);
    uint32_t height = std::stoi(heightstring);

    // Get rle
    std::string rlestring{};
    bool rle = false;
    if (config.getKeyValue("rle", rlestring)) {
        rle = std::stoi(rlestring);
    } 

    // Read quantization matrix
    std::vector<float> quant_mat{};
    std::string filename{};
    if (!config.getKeyValue("quantfile",filename)){
        std::cerr << config.getErrorDescription() << "Could not read quantfile\n";
        return -1;
    }
    quant_mat = fileToMatrix(filename);


    // Read inputfile
    const std::vector<uint8_t> data_uint8 = fileToInt8Vec(file_input);

    if (rle) {
        // encode data
        const std::vector<std::vector<RLE_encoded>> rle_blocks = encode_data_rle(
            data_uint8,width, height,  quant_mat, logger
        );

        const int BYTES_PER_RLE = 1 + 4; // 1 byte for zeroes, 4 bytes for value

        // start writer
        int writer_buffer_size = 4 + 4 + 1 + quant_mat.size()*4; // width + height + rle bit + quant mat
        for (int i = 0; i < rle_blocks.size();++i){
            writer_buffer_size += rle_blocks[i].size()*BYTES_PER_RLE;
        }
        util::BitStreamWriter writer{writer_buffer_size};
        
        // Write used settings to outputfile
        writer.put(32, width);
        writer.put(32, height);
        writer.put(1, rle);
        for (int i = 0; i < quant_mat.size();++i){
            writer.put(32, *(uint32_t*)&quant_mat[i]);
        }

        // write encoded blocks
        write_rle_blocks(writer, rle_blocks);

        std::ofstream output_file{file_output};
        util::write(output_file, writer);
    } else {
        // encode data
        const std::vector<std::vector<int32_t>> data = encode_data(
            data_uint8,width, height,  quant_mat, logger
        );

        // start writer
        int writer_buffer_size = 4 + 4 + 1 + quant_mat.size()*4; // width + height + rle bit + quant mat
        for (int i = 0; i < data.size();++i){
            writer_buffer_size += data[i].size()*4;
        }
        util::BitStreamWriter writer{writer_buffer_size};
        
        // Write used settings to outputfile
        writer.put(32, width);
        writer.put(32, height);
        writer.put(1, rle);
        for (int i = 0; i < quant_mat.size();++i){
            writer.put(32, *(uint32_t*)&quant_mat[i]);
        }

        // write encoded blocks
        write_zigzag_data(writer, data);
        std::ofstream output_file{file_output};
        util::write(output_file, writer);
    }

    return 0;
}