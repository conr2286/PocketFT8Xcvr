#include "DEBUG.h"
#include "si5351.h"
#include "traffic_manager.h"
#include "display.h"
#include "decode_ft8.h"
#include "gen_ft8.h"
#include "button.h"
#include "pins.h"
#include <SI4735.h>


#define FT8_TONE_SPACING 625



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



/**
 * Turn-on the transmitter at the carrier frequency, F_Long
 *
**/
void transmit_sequence(void) {

  DTRACE();

  //Program the transmitter clock at F_Long
  set_Xmit_Freq();
  si5351.set_freq(F_Long, SI5351_CLK0);

  //Disconnect receiver from antenna and enable the SN74ACT244 PA
  pinMode(PIN_RCV, OUTPUT);
  digitalWrite(PIN_RCV, LOW);

  //Set receiver's volume down low
  si4735.setVolume(35);

  //Enable the transmitter clock
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);  // Set for max power if desired
  si5351.output_enable(SI5351_CLK0, 1);

  //Connect transmitter to antenna and short the receiver RF input to ground
  pinMode(PIN_PTT, OUTPUT);
  digitalWrite(PIN_PTT, HIGH);
}




/**
 *  Switch hardware from transmitting to receiving, and clears the outbound FT8 message
 *
**/
void receive_sequence(void) {

  DTRACE();

  //Turn off the transmitter's clock -- this should stop the xmit RF chain
  si5351.output_enable(SI5351_CLK0, 0);

  //Disconnect the SN74ACT244 PA from antenna and float the receiver's RF input
  pinMode(PIN_PTT, OUTPUT);
  digitalWrite(PIN_PTT, LOW);

  //Connect receiver to antenna and disable the SN74ACT244 PA
  pinMode(PIN_RCV, OUTPUT);
  digitalWrite(PIN_RCV, HIGH);

  //Receive
  si4735.setVolume(50);
  clear_FT8_message();

}  //receive_sequence()




/**
 * Turn-on the transmitter at F_Long for tuning
 *
**/
void tune_On_sequence(void) {

  //Program the transmitter clock to F_Long
  set_Xmit_Freq();
  si5351.set_freq(F_Long, SI5351_CLK0);

  //Drop the receiver's volume
  si4735.setVolume(35);

  //Turn-on the transmitter clock
  si5351.output_enable(SI5351_CLK0, 1);

  //Disconnect receiver from antenna and enable the SN74ACT244 PA
  pinMode(PIN_RCV, OUTPUT);
  digitalWrite(PIN_RCV, LOW);

  //Short receiver's RF input to ground
  pinMode(PIN_PTT, OUTPUT);
  digitalWrite(PIN_PTT, HIGH);
}  //tune_On_sequence()





/**
 * Turn-off transmitter w/o affecting outbound message[]
**/
void tune_Off_sequence(void) {

  //Turn-off the transmitter's clock
  si5351.output_enable(SI5351_CLK0, 0);

  //Float the receiver's RF input
  pinMode(PIN_PTT, OUTPUT);
  digitalWrite(PIN_PTT, LOW);

  //Connect receiver to antenna and disable SN74ACT244 PA
  pinMode(PIN_RCV, OUTPUT);
  digitalWrite(PIN_RCV, HIGH);

  //Crank-up the receiver volume
  si4735.setVolume(50);
}  //tune_Off_sequence()





//KQ7B:  Recalculates carrier frequency F_Long and programs SI5351 with new unmodulated carrier frequency
void set_Xmit_Freq() {

  // display_value(400, 320, ( int ) cursor_freq);
  // display_value(400, 360,  offset_freq);
  F_Long = (uint64_t)((currentFrequency * 1000 + cursor_freq + offset_freq) * 100);
  //DPRINTF("%s currentFrequency=%u, cursor_freq=%u, offset_freq=%u, F_Long=%llu\n", __FUNCTION__, currentFrequency, cursor_freq, offset_freq, F_Long);
  //F_Long = (uint64_t) ((currentFrequency * 1000 + cursor_freq ) * 100);
  si5351.set_freq(F_Long, SI5351_CLK0);

}  //set_Xmit_Freq()


/**
 * Transmitter's FSK modulator
 *
 * Programs the SI5351 for F_Long + the specified FT8 tone.  This  function is repeatedly
 * invoked to shift the carrier during transmission.  Note that we are using direct FSK,
 * not AFSK.
 *
 * @param ft8_tone FT8 tone frequency
 *
**/
void set_FT8_Tone(uint8_t ft8_tone) {
  F_FT8 = F_Long + uint64_t(ft8_tone) * FT8_TONE_SPACING;
  si5351.set_freq(F_FT8, SI5351_CLK0);
}


//Immediately turns on the transmitter's carrier at the current F_Long frequency.
//Sets xmit_flag notifying loop() to modulate the carrier, apparently the only place
//where this happens (i.e. if you want to have the carrier modulated, you must
//call setup_to_transmit_on_next_DSP_Flag).  I think the outbound string
//resides in the global message[].
void setup_to_transmit_on_next_DSP_Flag(void) {
  DPRINTF("%s\n", __FUNCTION__);
  ft8_xmit_counter = 0;
  transmit_sequence();  //Turns-on the transmitter carrier at current F_Long ??
  set_Xmit_Freq();      //Recalculates F_long and reprograms SI5351 ??
  xmit_flag = 1;        //This flag appears to trigger loop() to modulate the carrier
}


//Seems to be implementing a state machine for portions of an FT8 QSO???
//  1. The GUI button toggles the CQ_Flag examined by loop().
//  2. The main loop() invokes process_FT8_FFT() which invokes...
//  3. update_offset_waterfall() invokes service_CQ() at the end of receive timeslot
void service_CQ(void) {

  DPRINTF("%s, Beacon_state=%u\n", __FUNCTION__, Beacon_State);

  int receive_index;

  switch (Beacon_State) {

    case 0:
      Beacon_State = 1;  //Listen
      break;

    case 1:
      receive_index = Check_Calling_Stations(num_decoded_msg);

      if (receive_index >= 0) {
        display_selected_call(receive_index);
        set_message(2);  // Prepare the RSL message for transmission
      } else
        set_message(0);  // Prepare the CQ message for transmission
      Transmit_Armned = 1;
      Beacon_State = 2;

      break;

    case 2:
      receive_index = Check_Calling_Stations(num_decoded_msg);

      if (receive_index >= 0) {
        display_selected_call(receive_index);
        set_message(3);  // Prepare the 73 message for transmission
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
}  //service_CQ()
