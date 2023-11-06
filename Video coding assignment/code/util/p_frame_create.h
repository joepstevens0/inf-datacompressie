#ifndef P_FRAME_CREATE_H
#define P_FRAME_CREATE_H

#include <vector>
#include <motion_estimation.h>
#include <stdint.h>

std::vector<uint8_t> P_frame_decode(const std::vector<MotionVector>& motion_estimation, const std::vector<uint8_t>& last_frame, const int width, const int height){

    // calc motion blocks
    std::vector<MotionBlock> blocks;
    for (int i = 0; i < motion_estimation.size();++i){
        const MotionVector& motion_vec = motion_estimation[i];

        const int block_column= i%(width/MotionBlock::width);
        const int block_row = i/(width/MotionBlock::width);
        const int mid_x = MotionBlock::width*block_column + MotionBlock::width/2;
        const int mid_y = MotionBlock::height*block_row + MotionBlock::height/2;

        MotionBlock block;
        block.mid_x = mid_x;
        block.mid_y = mid_y;
        for (int x = 0; x < MotionBlock::width;++x){
            for (int y = 0; y < MotionBlock::height;++y){
                int pixel_x = x + motion_vec.x + mid_x - MotionBlock::width/2;
                int pixel_y = y + motion_vec.y + mid_y - MotionBlock::height/2;
                if (pixel_x >= 0 && pixel_x < width && pixel_y >= 0 && pixel_y < height){
                    block.setElement(x,y, last_frame[pixel_y*width + pixel_x]);
                }
                else
                    block.setElement(x,y,0);
            }
        }

        blocks.push_back(block);
    }

    // motionblocks to image
    std::vector<uint8_t> result;
    int num_blocks_in_row = width / MotionBlock::width;

    // Reuse code of previous task //
    for (int i = 0; i < blocks.size(); i += num_blocks_in_row) {    // i = index of first block in row i
        // Read 4 rows aka #num_blocks_in_row full blocks
        for (int k = 0; k < MotionBlock::height; k++) {       // i*4 + k = row in the image
            std::vector<uint8_t> row{};
            for (int l = 0; l < num_blocks_in_row; l++) {   // l = block in row
                std::vector<uint8_t> part {&blocks[i+l].data[k*MotionBlock::width], &blocks[i+l].data[(k*MotionBlock::width)+MotionBlock::width]};
                row.insert(row.end(), part.begin(), part.end());
            }
            result.insert(result.end(), row.begin(), row.end());
        }
    }

    return result;
}

#endif