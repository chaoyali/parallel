/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "helloneon-intrinsics.h"
#include <arm_neon.h>
#include <stdio.h>

/* this source file should only be compiled by Android.mk when targeting
 * the armeabi-v7a ABI, and should be built in NEON mode
 */
void
fir_filter_neon_intrinsics(short *output, const short* input, const short* kernel, int width, int kernelSize)
{
#if 1
   int nn, offset = -kernelSize/2;

   for (nn = 0; nn < width; nn++)
   {
        int mm, sum = 0;
        int32x4_t sum_vec = vdupq_n_s32(0);
        for(mm = 0; mm < kernelSize/4; mm++)
        {
            int16x4_t  kernel_vec = vld1_s16(kernel + mm*4);
            int16x4_t  input_vec = vld1_s16(input + (nn+offset+mm*4));
            sum_vec = vmlal_s16(sum_vec, kernel_vec, input_vec);
        }

        sum += vgetq_lane_s32(sum_vec, 0);
        sum += vgetq_lane_s32(sum_vec, 1);
        sum += vgetq_lane_s32(sum_vec, 2);
        sum += vgetq_lane_s32(sum_vec, 3);

        if(kernelSize & 3)
        {
            for(mm = kernelSize - (kernelSize & 3); mm < kernelSize; mm++)
                sum += kernel[mm] * input[nn+offset+mm];
        }

        output[nn] = (short)((sum + 0x8000) >> 16);
    }
#else /* for comparison purposes only */
    int nn, offset = -kernelSize/2;
    for (nn = 0; nn < width; nn++) {
        int sum = 0;
        int mm;
        for (mm = 0; mm < kernelSize; mm++) {
            sum += kernel[mm]*input[nn+offset+mm];
        }
        output[n] = (short)((sum + 0x8000) >> 16);
    }
#endif
}

void sqrt_neon_intrinsics(int N, float initialGuess, float values[], float output[]) {
    int mm;
    float32x4_t  one_vec = vdupq_n_f32(1.f);
    float32x4_t  half_vec = vdupq_n_f32(0.5f);
    float32x4_t  three_vec = vdupq_n_f32(3.f);
    float32x4_t  threshold_vec = vdupq_n_f32(0.00001f);

    for (mm = 0; mm < N/4; mm ++)
    {
        float32x4_t  x_vec = vld1q_f32(values + mm * 4);
        float32x4_t  guess_vec = vdupq_n_f32(initialGuess);
        float32x4_t  guess_squ_vec = vdupq_n_f32(initialGuess * initialGuess);
        float32x4_t  pred_vec = vabdq_f32(vmulq_f32(guess_squ_vec, x_vec), one_vec);
        uint32x4_t  op = vcleq_f32(pred_vec, threshold_vec);
        uint32_t finished = vgetq_lane_u32(op, 0) & vgetq_lane_u32(op, 1) & vgetq_lane_u32(op, 2) & vgetq_lane_u32(op, 3);
        while (!finished) {
            guess_vec = vmulq_f32( vsubq_f32( vmulq_f32(three_vec, guess_vec), vmulq_f32( guess_squ_vec, vmulq_f32(x_vec, guess_vec) ) ), half_vec );
            guess_squ_vec = vmulq_f32(guess_vec, guess_vec);
            pred_vec = vabdq_f32(vmulq_f32(guess_squ_vec, x_vec), one_vec);
            op = vcleq_f32(pred_vec, threshold_vec);
            finished = vgetq_lane_u32(op, 0) & vgetq_lane_u32(op, 1) & vgetq_lane_u32(op, 2) & vgetq_lane_u32(op, 3);
        }
        vst1q_f32(output + mm * 4, vmulq_f32(guess_vec, x_vec));
    }
}
