#include "si5351.h"
#include "traffic_manager.h"
#include "display.h"
#include "decode_ft8.h"
#include "gen_ft8.h"
#include "button.h"
#define PTT_Pin 14            //For Teensy 4.1
#include <SI4735.h>

#define FT8_TONE_SPACING        625

extern Si5351 si5351;
extern SI4735 si4735;
extern uint16_t currentFrequency;
extern int xmit_flag, ft8_xmit_counter, Transmit_Armned;

extern uint16_t cursor_freq;
extern uint16_t cursor_line;

extern int CQ_Flag;
extern int Beacon_State;
extern int num_decoded_msg;

extern int offset_freq;

uint64_t F_Long, F_FT8, F_Offset;

   void transmit_sequence(void){
      set_Xmit_Freq();
      si5351.set_freq(F_Long, SI5351_CLK0);
      si4735.setVolume(35);
      si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA); // Set for max power if desired
      si5351.output_enable(SI5351_CLK0, 1);
      pinMode(PTT_Pin, OUTPUT); 
      digitalWrite(PTT_Pin, HIGH);   
   }

      void receive_sequence(void){
       si5351.output_enable(SI5351_CLK0, 0);
       pinMode(PTT_Pin, OUTPUT);
       digitalWrite(PTT_Pin, LOW);
       si4735.setVolume(50);
       clear_FT8_message();   

   }

   
   void tune_On_sequence(void){
      set_Xmit_Freq();
      si5351.set_freq(F_Long, SI5351_CLK0);
      si4735.setVolume(35);
      si5351.output_enable(SI5351_CLK0, 1);
      pinMode(PTT_Pin, OUTPUT); 
      digitalWrite(PTT_Pin, HIGH); 
   }

      void tune_Off_sequence(void){
       si5351.output_enable(SI5351_CLK0, 0); 
       pinMode(PTT_Pin, OUTPUT);
       digitalWrite(PTT_Pin, LOW); 
       si4735.setVolume(50);
   }


   void  set_Xmit_Freq(){

     // display_value(400, 320, ( int ) cursor_freq);
     // display_value(400, 360,  offset_freq);
      F_Long = (uint64_t) ((currentFrequency * 1000 + cursor_freq + offset_freq) * 100);
      //F_Long = (uint64_t) ((currentFrequency * 1000 + cursor_freq ) * 100);
      si5351.set_freq(F_Long, SI5351_CLK0);
}



    void set_FT8_Tone( uint8_t ft8_tone) {
          F_FT8 =  F_Long + uint64_t(ft8_tone) * FT8_TONE_SPACING;
          si5351.set_freq(F_FT8, SI5351_CLK0);
    }


    void setup_to_transmit_on_next_DSP_Flag(void){
        ft8_xmit_counter = 0;
        transmit_sequence();
        set_Xmit_Freq();
        xmit_flag = 1;

    }


    void service_CQ (void) {

      int receive_index;

      switch (Beacon_State) {

      case 0: Beacon_State = 1;//Listen 
      break;

      case 1: receive_index = Check_Calling_Stations(num_decoded_msg);
      
              if(receive_index >= 0) {
              display_selected_call(receive_index);
              set_message(2); // send RSL
              }
              else
              set_message(0); // send CQ
              Transmit_Armned = 1;
              Beacon_State = 2;
       
      break;

      case 2: receive_index = Check_Calling_Stations(num_decoded_msg);
      
              if(receive_index >= 0) {
              display_selected_call(receive_index);
              set_message(3); // send 73
              Transmit_Armned = 1;
              }

              Beacon_State = 0;
      break;

      /*
      case 3: receive_index = Check_Calling_Stations(num_decoded_msg);
      
              if(receive_index >= 0) {
              display_selected_call(receive_index);
              set_message(3); // send 73
              Transmit_Armned = 1;
              }
              Beacon_State = 0;
       
      break;
      */

        
      }
      
    }


