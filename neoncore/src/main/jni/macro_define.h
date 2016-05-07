//
// Created by 朱晓光 on 4/15/16.
//

#ifndef HELLO_NEON_MACRO_DEFINE_H
#define HELLO_NEON_MACRO_DEFINE_H

#define N 133232
#define SHRINK_FACTOR 1.3
#define INT_MAX 0x7fffffff
#define APPNAME "NALIB"

#define CACHE_LINE 2048

#define LANES_INT_NUM 4
#define LANES_SHORT_NUM 8
#define LANES_FLOAT_NUM 4
#define LANES_LONG_NUM 2

#define BLOCK_M 4
#define BLOCK_N 4
#define BLOCK_K 4

#define BLOCK_TRANSPOSE 8

#define BIT_LEN 4096

#endif //HELLO_NEON_MACRO_DEFINE_H
