#pragma once
#include <Arduino.h>


   void transmit_sequence(void);

   void receive_sequence(void);

   void  set_Xmit_Freq(void);

   void set_FT8_Tone( uint8_t ft8_tone);

   void setup_to_transmit_on_next_DSP_Flag(void);

   void tune_On_sequence(void);
   
   void tune_Off_sequence(void);

   //void service_CQ (void);

   
