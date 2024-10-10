#include "DEBUG.h"
//#include "NODEBUG.h"
#include "HX8357_t3n.h"
#include "Process_DSP.h"
#include "WF_Table.h"
#include "arm_math.h"
#include "decode_ft8.h"
#include "display.h"
#include <TimeLib.h>
#include "traffic_manager.h"


extern HX8357_t3n tft;
extern q15_t dsp_buffer[] __attribute__((aligned(4)));
extern q15_t dsp_output[] __attribute__((aligned(4)));
q15_t window_dsp_buffer[FFT_SIZE] __attribute__((aligned(4)));


uint8_t WF_index[900];
float window[FFT_SIZE];

extern uint16_t cursor_line;

extern int ft8_flag, FT_8_counter, ft8_marker, decode_flag, WF_counter;
extern int num_decoded_msg;

int master_offset, offset_step;
extern int CQ_Flag;

q15_t FFT_Scale[FFT_SIZE * 2];
q15_t FFT_Magnitude[FFT_SIZE];
int32_t FFT_Mag_10[FFT_SIZE / 2];
uint8_t FFT_Buffer[FFT_SIZE / 2];
float mag_db[FFT_SIZE / 2 + 1];

arm_rfft_instance_q15 fft_inst;
arm_cfft_radix4_instance_q15 aux_inst;

uint8_t export_fft_power[ft8_msg_samples * ft8_buffer * 4];

// void init_DSP(void) {
//   arm_rfft_init_q15(&fft_inst, &aux_inst, FFT_SIZE, 0);  //T4.1
//   for (int i = 0; i < FFT_SIZE; ++i) window[i] = ft_blackman_i(i, FFT_SIZE);
//   offset_step = (int)ft8_buffer * 4;
//       DTRACE();
//     DPRINTF("fft_inst.fftLenReal=%u\n",fft_inst.fftLenReal);
//     DPRINTF("FFT_SIZE=%u\n",FFT_SIZE);
// }
void init_DSP(void) {
  //arm_rfft_init_q15(&fft_inst, &aux_inst, FFT_SIZE, 0, 1);
  arm_rfft_init_q15(&fft_inst, FFT_SIZE, 0, 1);
  for (int i = 0; i < FFT_SIZE; ++i) window[i] = ft_blackman_i(i, FFT_SIZE);
  offset_step = (int)ft8_buffer * 4;
}

int max_bin, max_bin_number;


float ft_blackman_i(int i, int N) {
  const float alpha = 0.16f;  // or 2860/18608
  const float a0 = (1 - alpha) / 2;
  const float a1 = 1.0f / 2;
  const float a2 = alpha / 2;

  float x1 = cosf(2 * (float)M_PI * i / (N - 1));
  //float x2 = cosf(4 * (float)M_PI * i / (N - 1));
  float x2 = 2 * x1 * x1 - 1;  // Use double angle formula
  return a0 - a1 * x1 + a2 * x2;
}



// Compute FFT magnitudes (log power) for each timeslot in the signal
void extract_power(int offset) {
  //DTRACE();

  // Loop over two possible time offsets (0 and block_size/2)
  for (int time_sub = 0; time_sub <= input_gulp_size / 2; time_sub += input_gulp_size / 2) {
    //DTRACE();
    for (int i = 0; i < FFT_SIZE; i++) window_dsp_buffer[i] = (q15_t)((float)dsp_buffer[i + time_sub] * window[i]);
    //DTRACE();
    //DPRINTF("fft_inst.fftLenReal=%u\n", fft_inst.fftLenReal);
    arm_rfft_q15(&fft_inst, window_dsp_buffer, dsp_output);
    //DTRACE();
    arm_shift_q15(&dsp_output[0], 5, &FFT_Scale[0], FFT_SIZE * 2);
    //DTRACE();
    arm_cmplx_mag_squared_q15(&FFT_Scale[0], &FFT_Magnitude[0], FFT_SIZE);
    //DTRACE();
    for (int j = 0; j < FFT_SIZE / 2; j++) {
      FFT_Mag_10[j] = 10 * (int32_t)FFT_Magnitude[j];
      mag_db[j] = 5.0 * log((float)FFT_Mag_10[j] + 0.1);
    }
    //DTRACE();
    // Loop over two possible frequency bin offsets (for averaging)
    for (int freq_sub = 0; freq_sub < 2; ++freq_sub) {
      for (int j = 0; j < ft8_buffer; ++j) {
        float db1 = mag_db[j * 2 + freq_sub];
        float db2 = mag_db[j * 2 + freq_sub + 1];
        float db = (db1 + db2) / 2;

        int scaled = (int)(db);
        export_fft_power[offset] = (scaled < 0) ? 0 : ((scaled > 255) ? 255 : scaled);
        ++offset;
      }
    }
  }
}


void process_FT8_FFT(void) {

  if (ft8_flag == 1) {


    master_offset = offset_step * FT_8_counter;
    extract_power(master_offset);

    update_offset_waterfall(master_offset);

    FT_8_counter++;
    if (FT_8_counter == ft8_msg_samples) {
      ft8_flag = 0;
      decode_flag = 1;
    }
  }
}




void update_offset_waterfall(int offset) {

  for (int j = ft8_min_bin; j < ft8_buffer; j++) FFT_Buffer[j] = export_fft_power[j + offset];

  int bar;
  for (int x = ft8_min_bin; x < ft8_buffer; x++) {
    bar = FFT_Buffer[x];
    if (bar > 63) bar = 63;
    WF_index[x] = bar;
  }

  //DPRINTF("offset=%d, ft8_min_bin=%d, ft8_buffer=%d\n",offset,ft8_min_bin,ft8_buffer);

  //tft.graphicsMode();
  for (int k = ft8_min_bin; k < ft8_buffer; k++) {
    tft.drawPixel(k - ft8_min_bin, WF_counter, WFPalette[WF_index[k]]);
    if (k - ft8_min_bin == cursor_line) tft.drawPixel(k - ft8_min_bin, WF_counter, HX8357_RED);
  }

  //DPRINTF("num_decoded_msg=%u, WF_counter=%u\n",num_decoded_msg,WF_counter);

  if (num_decoded_msg > 0 && WF_counter == 0) {
    display_messages(num_decoded_msg);
    if (CQ_Flag == 1) service_CQ();
    else
      Check_Calling_Stations(num_decoded_msg);

    num_decoded_msg = 0;
  }


  WF_counter++;
}
