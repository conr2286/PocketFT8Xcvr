
#include "arm_math.h"

#define FFT_SIZE 2048
#define num_que_blocks 8
#define block_size 128
#define input_gulp_size 1024

#define ft8_buffer 400  //arbitrary for 3 kc
#define ft8_min_bin 48
#define FFT_Resolution 6.25
#define ft8_min_freq FFT_Resolution* ft8_min_bin

#define ft8_msg_samples 92

void init_DSP(void);
float ft_blackman_i(int i, int N);

void process_FT8_FFT(void);
void update_offset_waterfall(int offset);
