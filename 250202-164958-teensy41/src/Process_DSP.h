
#include "arm_math.h"

#define FFT_SIZE 2048
#define num_que_blocks 8
#define block_size 128
#define input_gulp_size 1024

#define ft8_buffer 400  // arbitrary for 3 kc
#define ft8_min_bin 48
#define FFT_Resolution 6.25
#define ft8_min_freq FFT_Resolution* ft8_min_bin

// Define the number of message samples.  To the best of my knowledge:
//   + The audio pipeline queues blocks of time domain samples at the rate of 6400 samples/second
//   + Each queue block contains 128 16-bit samples (256 bytes)
//   + The forward FFT operates on one "message sample," a set of 8 blocks (1024 16-bit samples)
//   + The ADC acquired those 1024 samples over a period of 1024/6400 = 0.160 seconds (one FT8 symbol)
//   + Thus, one FT8 symbol requires one "message sample" of 8 blocks
//   + An FT8 message requires a minimum of 79 "message samples"
//   + find_sync() allows "time offsets that exceed signal boundaries" (i.e. >79 message samples)
#define ft8_msg_samples 92  // Number of 1024 point message samples feeding the FFT?

void init_DSP(void);
float ft_blackman_i(int i, int N);

void process_FT8_FFT(void);
void update_offset_waterfall(int offset);
