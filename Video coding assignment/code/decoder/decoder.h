#ifndef DECODER_H
#define DECODER_H

#include <BitStream.h>
#include <vector>
#include <motion_estimation.h>
#include <p_frame_create.h>

std::vector<uint8_t> P_frame_read(util::BitStreamReader& reader, const std::vector<uint8_t>& last_frame, const int width, const int height){
    int total_motion_blocks = width*height/(MotionBlock::width*MotionBlock::height);

    // read motion estimation
    std::vector<MotionVector> motion_estimation;
    for (int i = 0; i < total_motion_blocks;++i){
        int32_t vec_x = reader.get(32);
        int32_t vec_y = reader.get(32);

        motion_estimation.push_back(MotionVector{vec_x, vec_y});
    }

    return P_frame_decode(motion_estimation, last_frame, width, height);
}



#endif