#include <iostream>
#include <util.h>
#include "decoder.h"
#include <zigzag.h>
#include <BitStream.h>
#include <algorithm>
#include <configreader.h>
#include <rle.h>
#include "image_decode.h"
#include <math.h>

int main(int argc, char** argv){
    std::cout << "Decoder started\n";

    ConfigReader config{};

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
    // Get motion compensation
    std::string motion_compensation_string = "0";
    if (!config.getKeyValue("motioncompensation", motion_compensation_string)){
        std::cerr << "Failed to get motion compensation boolean." << "\n";
        std::cerr << config.getErrorDescription() << "\n";
        return -1;
    }
    bool motion_compensation = motion_compensation_string == "1";

    // open file
    std::ifstream instream(file_input, std::ios::in | std::ios::binary);

    // create setting reader
    const int SETTINGS_SIZE = 4 + 4 + 1 + 4 + Block::width*Block::height*4;
    uint8_t* buffer = new uint8_t[SETTINGS_SIZE];
    instream.read((char*)buffer, SETTINGS_SIZE);
    util::BitStreamReader settings_reader{buffer, SETTINGS_SIZE};

    // Read settings from encoded file
    const uint32_t width = settings_reader.get(32);
    const uint32_t height = settings_reader.get(32);
    std::cout << "Image width: " << width << " Image height: " << height << "\n";
    const bool rle = settings_reader.get(8);
    const uint32_t gop = settings_reader.get(32);
    std::vector<float> quant_mat{};
    for (int i = 0; i < Block::width*Block::height;++i){
        uint32_t t = settings_reader.get(32);
        quant_mat.push_back(*(float*)&t);
    }

    // create frame reader
    const int frame_max_byte_size = 2*width*height;
    uint8_t* frame_buffer = new uint8_t[frame_max_byte_size];
    util::BitStreamReader reader{frame_buffer, frame_max_byte_size};

    // read frames
    std::vector<std::vector<uint8_t>> frames;
    int GOP_counter = 0;
    while (!instream.eof() && instream.peek() != EOF){

        const int FRAME_LENGTH_AND_EXTRA_BITS_LENGTH = 5;

        // read frame settings
        instream.read((char*)frame_buffer, FRAME_LENGTH_AND_EXTRA_BITS_LENGTH);
        uint32_t current_frame_size = reader.get(32);
        uint32_t extra_bits = reader.get(8);
        std::cout << "Current frame size: "  << current_frame_size << "\n";
        std::cout << "Extra bits: "  << extra_bits << "\n";
        reader.reset();

        // read frame
        instream.read((char*)frame_buffer, current_frame_size-FRAME_LENGTH_AND_EXTRA_BITS_LENGTH);
        
        std::vector<uint8_t> frame;
        if (GOP_counter == 0){
            std::cout << "------------ START I-FRAME ----------------\n";
            frame = image_decode(reader, 8*current_frame_size-8*FRAME_LENGTH_AND_EXTRA_BITS_LENGTH-extra_bits,width, height, rle, quant_mat);
            std::cout << "------------ END I-FRAME ----------------\n";
        } else {
            std::cout << "------------ START P-FRAME ----------------\n";
            std::vector<uint8_t>& last_frame = frames[frames.size() - 1];

            // frame = P_frame_read(reader, last_frame, width, height);

            std::vector<uint8_t> p_frame = P_frame_read(reader, last_frame, width, height);

            std::vector<float> diffs = image_decode_float(reader, 8*current_frame_size-8*FRAME_LENGTH_AND_EXTRA_BITS_LENGTH-extra_bits - reader.get_position(),width, height, rle, quant_mat);

            frame.resize(width*height, 0);
            for (int i = 0; i < width*height;++i){
                if (motion_compensation){
                    float r = (float)p_frame[i] + diffs[i];
                    if (r > 255.){
                        // std::cout << "OVERFLOW " << (float)p_frame[i] << " + " << diffs[i] << "\n";
                        r = 255.;
                    } else if (r < 0.){
                        // std::cout << "UNDERFLOW " << (float)p_frame[i] << " + " << diffs[i] << "\n";
                        r = 0.;
                    }
                    frame[i] = round(r);
                } else {
                    frame[i] = p_frame[i];
                }
            }
            std::cout << "------------ END P-FRAME ----------------\n";
        }

        GOP_counter = (GOP_counter + 1) % gop;

        frames.push_back(frame);
        std::cout << "Decoded frame " << frames.size() << "\n";

        std::cout << "Bytes read: " << reader.get_position()/8 << "\n";

        reader.reset();
    }
    std::cout << "Decoded " << frames.size() << " frames\n"; 

    std::vector<uint8_t> result_uint8;
    for (int i = 0; i < frames.size();++i){
        frames[i].resize(width*height + (width*height)/2, 128);
        result_uint8.insert(result_uint8.end(), frames[i].begin(), frames[i].begin() + width*height + (width*height)/2);
    }

    // Write result to outputfile
    int8VecToFile(file_output, result_uint8);

    return 0;
}