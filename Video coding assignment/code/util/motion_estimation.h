#ifndef MOTION_ESTIMATION_H
#define MOTION_ESTIMATION_H

#include <BitStream.h>
#include <vector>
#include <iostream>

struct MotionVector{
    int32_t x = 0;
    int32_t y = 0;
};

struct MotionBlock{
    std::vector<uint8_t> data = std::vector<uint8_t>(width*height, 0);
    static const int width = 16;
    static const int height = 16;
    int mid_x = 0;
    int mid_y = 0;
    uint8_t getElement(int x, int y) const {
        return data[y*width+x];
    }

    void setElement(int x, int y, uint8_t value) {
        data[y*width+x] = value;
    }
};

std::vector<MotionBlock> createMotionBlocks(const std::vector<uint8_t>& input_img, int width, int height) {
    std::vector<MotionBlock> blocks{};

    for (int i = 0; i < height; i += MotionBlock::height) {
        for (int j = 0; j < width; j += MotionBlock::width) {
            MotionBlock block{};
            block.data = std::vector<uint8_t>();
            for (int k = 0; k < MotionBlock::height; k++) {
                std::vector<uint8_t> part {&input_img[(i+k)*width+j], &input_img[(i+k)*width+j+MotionBlock::width]};
                block.data.insert(block.data.end(), part.begin(), part.end());
            }
            block.mid_x = j + MotionBlock::width/2;
            block.mid_y = i + MotionBlock::height/2;
            blocks.push_back(block);
        }
    }

    return blocks;
}

int32_t calc_block_MAD(const std::vector<uint8_t>& last_frame, const MotionBlock& block, int i, int j, int frame_width, int frame_height){
    int32_t mad = 0;

    int x = block.mid_x - MotionBlock::width/2;
    int y = block.mid_y - MotionBlock::height/2;
    for (int k = 0; k < MotionBlock::width; ++k){
        for (int l = 0; l < MotionBlock::height; ++l){
            int32_t frame_value = 0;

            if ((y+l+j) >= 0 && (y+l+j) < frame_height && (x+k+i) >= 0 && (x+k+i) < frame_width){
                frame_value = (int32_t)last_frame[(y+l+j)*frame_width + (x+k+i)];
            }
            
            mad += abs( ((int32_t)block.getElement(k, l)) - frame_value);
        }
    }
    return mad;
}

std::vector<MotionVector> calc_motion_estimation(const std::vector<uint8_t>& last_frame,const std::vector<uint8_t>& this_frame, int merange, int frame_width, int frame_height){
    // transform into blocks
    std::vector<MotionBlock> this_frame_blocks = createMotionBlocks(this_frame, frame_width, frame_height);

    std::cout << "Created " << this_frame_blocks.size() << " motion blocks\n";

    // calculate best motion vectors per block
    std::vector<MotionVector> result;
    for (int block_index = 0; block_index < this_frame_blocks.size(); ++block_index){

        // calc best vector using sequential search
        int32_t best_mad = INT32_MAX;
        int32_t vec_x = 0;
        int32_t vec_y = 0;
        for (int i = -merange; i < merange + 1; ++i){
            for (int j = -merange; j < merange + 1; ++j){

                int32_t mad = calc_block_MAD(last_frame, this_frame_blocks[block_index], i,j, frame_width, frame_height);
                if (mad < best_mad){
                    best_mad = mad;
                    vec_x = i;
                    vec_y = j;
                }
            }
        }

        result.push_back(MotionVector{vec_x, vec_y});
    }

    return result;
}

void store_motion_estimation(std::vector<MotionVector>& motion_estimation, util::BitStreamWriter& writer){
    for (int i = 0; i < motion_estimation.size();++i){
        writer.put(32,motion_estimation[i].x);
        writer.put(32,motion_estimation[i].y);
    }
}

#endif