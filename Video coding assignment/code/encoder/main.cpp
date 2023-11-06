
#include <BitStream.h>
#include <configreader.h>
#include <iostream>
#include <logger.h>
#include <util.h>
#include "image_encode.h"
#include <motion_estimation.h>
#include <p_frame_create.h>

#include "../decoder/image_decode.h"
#include "../decoder/decoder.h"


void encode_I_frame(util::BitStreamWriter& writer, const std::vector<uint8_t>& this_frame, int video_width, int video_height, bool rle, std::vector<float>& quant_mat, Logger& logger)
{
    // write encoded image
    std::cout << "Encoding I-frame...\n";
    image_encode(this_frame, video_width, video_height, rle, quant_mat, logger, writer);
}
void encode_P_frame(
        util::BitStreamWriter& writer, const std::vector<uint8_t>& last_frame, const std::vector<uint8_t>& this_frame,
        int video_width, int video_height, int merange, bool rle, std::vector<float>& quant_mat, Logger& logger
    )
{
    std::cout << "Encoding P-frame...\n";
    std::cout << "Calulating motion estimation\n";
    std::vector<MotionVector> motion_estimation = calc_motion_estimation(last_frame, this_frame, merange, video_width, video_height);
    std::cout << "Storing motion estimation\n";
    store_motion_estimation(motion_estimation, writer);

    std::vector<uint8_t> p_frame = P_frame_decode(motion_estimation, last_frame, video_width, video_height);

    // calc P-frame difference
    std::vector<float> diffs;
    diffs.resize(video_width*video_height,0);
    for (int i = 0; i <video_width*video_height;++i){
        diffs[i] = ((float)this_frame[i]) - ((float)p_frame[i]);
    }
    image_encode_float(diffs, video_width, video_height, rle, quant_mat, logger, writer);
}

int main(int argc, char** argv){

    std::cout << "Encoder started\n";

    ConfigReader config{};

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
    uint32_t video_width = std::stoi(widthstring);
    uint32_t video_height = std::stoi(heightstring);

    // Get rle
    std::string rlestring{};
    bool rle = false;
    if (config.getKeyValue("rle", rlestring))
    {
        rle = std::stoi(rlestring);
    }

    // Read quantization matrix
    std::vector<float> quant_mat{};
    std::string filename{};
    if (!config.getKeyValue("quantfile", filename))
    {
        std::cerr << config.getErrorDescription() << "Could not read quantfile\n";
        return -1;
    }
    quant_mat = fileToMatrix(filename);


    // Get GOP size
    std::string gopstring = "";
    if (!config.getKeyValue("gop",gopstring)){
        std::cerr << "Failed to get GOP size." << "\n";
        std::cerr << config.getErrorDescription() << "\n";
        return -1;
    }
    int gop = std::stoi(gopstring);


    // Get merange
    std::string merangestring = "";
    if (!config.getKeyValue("merange",merangestring)){
        std::cerr << "Failed to get merange." << "\n";
        std::cerr << config.getErrorDescription() << "\n";
        return -1;
    }
    int merange = std::stoi(merangestring);


    // Read inputfile
    const std::vector<uint8_t> data_uint8 = fileToInt8Vec(file_input);

    // split into frames
    const int frame_size = video_width*video_height*(3./2.);
    const int total_frames = data_uint8.size()/frame_size;
    const int frame_Y_size = video_width* video_height;
    std::vector<std::vector<uint8_t>> frames;
    for (int p = 0; p < data_uint8.size(); p += frame_size){
        frames.push_back(std::vector<uint8_t>(data_uint8.begin() + p, data_uint8.begin() + p + frame_Y_size));
    }


    // create writer
    int writer_buffer_size = 2*frames[0].size();
    util::BitStreamWriter writer{writer_buffer_size};
    std::ofstream output_file{file_output};

    // Write used settings to outputfile
    writer.put(32, video_width);
    writer.put(32, video_height);
    writer.put(8, rle);
    writer.put(32, gop);
    for (int i = 0; i < quant_mat.size(); ++i)
    {
        writer.put(32, *(uint32_t *)&quant_mat[i]);
    }
    util::write(output_file, writer);
    writer.reset();

    // Write frames
    int GOP_counter = 0;

    // reader for I-frames
    util::BitStreamReader reader{writer.get_buffer(), writer.get_size()};


    std::vector<uint8_t> last_frame;
    
    for (int i = 0; i < frames.size();++i){
        writer.put(32, 0);  // reserve space for frame byte length
        writer.put(8, 0);   // reserve space for extra bits in frame

        if (GOP_counter == 0){
            // encode the i-frame
            encode_I_frame(writer, frames[i], video_width, video_height, rle, quant_mat, logger);

            // update last frame
            // read last frame
            std::cout << "Reading I-frame...\n";
            reader.reset();
            reader.set_position(32 + 8);
            std::cout << "Writer pos: " << writer.get_position() << "\n";
            last_frame = image_decode(reader, writer.get_position() - (32  + 8),video_width, video_height, rle, quant_mat);
        } else {
            // encode the frame
            encode_P_frame(writer, last_frame, frames[i], video_width, video_height, merange, rle, quant_mat, logger);

            // update last frame
            // read last frame
            std::cout << "Reading P-frame...\n";
            reader.reset();
            reader.set_position(32 + 8);    // skips byte length and extra bits
            std::vector<uint8_t> read_p_frame = P_frame_read(reader, last_frame, video_width, video_height);
            std::vector<float> read_diffs = image_decode_float(reader, writer.get_position() - reader.get_position(),video_width, video_height, rle, quant_mat);

            last_frame.resize(video_width*video_height,0);
            for (int j = 0; j < video_width*video_height;++j){
                float r = (float)read_p_frame[j] + read_diffs[j];
                if (r > 255.){
                    r = 255.;
                } else if (r < 0.){
                    r = 0.;
                }
                last_frame[j] = round(r);
            }
        }
        
        GOP_counter = (GOP_counter + 1) % gop;

        // Write extra bits to end frame on a full byte
        int last_pos_bit = writer.get_position();
        writer.flush();
        int extra_bits = writer.get_position() - last_pos_bit;
        int last_pos_byte = writer.get_position()/8;

        writer.set_position(0);
        writer.put(32, last_pos_byte);
        writer.set_position(32);
        writer.put(8, extra_bits);

        writer.set_position(8*last_pos_byte);
        
        std::cout << "Frame " << i << " encoding success with length:" << last_pos_byte <<  "\n";
        util::write(output_file, writer);
        writer.reset();
    }

    return 0;
}
