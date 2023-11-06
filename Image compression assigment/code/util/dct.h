#ifndef DCT_H
#define DCT_H
#include "block.h"

#include <vector>
#include <iostream>

const std::vector<float> A
    {
        0.5,    0.5,    0.5,    0.5,
        0.653,  0.271,  -0.271, -0.653,
        0.5,    -0.5,   -0.5,   0.5,
        0.271,  -0.653, 0.653, -0.271
    }
;

const std::vector<float> A_TRANSPOSED
    {
        0.5,    0.653,  0.5,    0.271,
        0.5,    0.271,  -0.5,   -0.653,
        0.5,    -0.271, -0.5,   0.653,
        0.5,    -0.653, 0.5,    -0.271
    }
;

float mat_get_Element(const std::vector<float>& mat, int x, int y){
    return mat[y*Block::width + x];
}

void mat_set_Element(std::vector<float>& mat, int x, int y, float value){
    mat[y*Block::width + x] = value;
}

std::vector<float> matrix_mult(std::vector<float> b1, std::vector<float> b2) {
    std::vector<float> b{};
    b.resize(Block::width*Block::height, 0);

    for (int x = 0; x < Block::width;++x){
        for (int y = 0; y < Block::height;++y){
            float value = 0;
            for (int k = 0; k < Block::height;++k){
                float v1 = static_cast<float>(mat_get_Element(b1,k,y));
                float v2 = static_cast<float>(mat_get_Element(b2,x,k));
                value += v1*v2;
            }
            mat_set_Element(b,x,y,value);
        }
    }
    return b;
}


Block forward_dct(Block block){
    // Y = A X At
    std::vector<float> X;
    for (int i = 0; i < block.data.size();++i){
        X.push_back(static_cast<float>(block.data[i]));
    }

    std::vector<float> Y = matrix_mult(X, A_TRANSPOSED);
    Y = matrix_mult(A, Y);

    Block result;
    for (int i = 0; i < Y.size();++i){
        result.data.push_back(static_cast<DataType>(Y[i]));
    }
    return result;
}

Block inverse_dct(Block block){
    // X = At Y A
    std::vector<float> Y;
    for (int i = 0; i < block.data.size();++i){
        Y.push_back(static_cast<float>(block.data[i]));
    }

    std::vector<float> X = matrix_mult(Y, A);
    X = matrix_mult(A_TRANSPOSED, X);

    Block result;
    for (int i = 0; i < X.size();++i){
        result.data.push_back(static_cast<DataType>(X[i]));
    }
    return result;
}

#endif