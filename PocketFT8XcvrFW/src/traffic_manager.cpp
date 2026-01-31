#include "traffic_manager.h"

#include <Arduino.h>
#include <SI4735.h>
#include <TimeLib.h>

#include "NODEBUG.h"
#include "button.h"
#include "decode_ft8.h"
// #include "display.h"
#include "PocketFT8Xcvr.h"
#include "UserInterface.h"
#include "gen_ft8.h"
#include "pins.h"
#include "si5351.h"

#define FT8_TONE_SPACING 625

extern Si5351 si5351;
extern SI4735 si4735;
extern int xmit_flag, ft8_xmit_counter, Transmit_Armned;

extern int num_decoded_msg;

// extern int offset_freq;

uint64_t F_Long, F_FT8, F_Offset;

extern int tune_flag;

extern UserInterface& ui;

/**
 * Turn-on the transmitter at the carrier frequency, F_Long
 *
 **/
void transmit_sequence(void) {
    DTRACE();

    // ui.applicationMsgs->setText(get_message(), A_RED);

    // Program the transmitter clock at F_Long
    set_Xmit_Freq();
    // si5351.set_freq(F_Long, SI5351_CLK0);

    // Disconnect receiver from antenna and enable the SN74ACT244 PA
    pinMode(PIN_RCV, OUTPUT);
    digitalWrite(PIN_RCV, LOW);

    // Set receiver's volume down low
    si4735.setVolume(0);

    // Enable the transmitter clock
    si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);  // Set for max power if desired
    if (thisStation.getEnableTransmit()) si5351.output_enable(SI5351_CLK0, 1);

    // Connect transmitter to antenna and short the receiver RF input to ground
    pinMode(PIN_PTT, OUTPUT);
    digitalWrite(PIN_PTT, HIGH);

    ui.setXmitRecvIndicator(INDICATOR_ICON_TRANSMIT);
}

/**
 *  Switch hardware from transmitting to receiving, and clears the outbound FT8 message
 *
 **/
void receive_sequence(void) {
    DTRACE();

    ui.applicationMsgs->setText(" ");

    // Turn off the transmitter's clock -- this should stop the xmit RF chain
    si5351.output_enable(SI5351_CLK0, 0);
    delay(1);  // Allow the clock to respond

    // Disconnect the SN74ACT244 PA from antenna and float the receiver's RF input
    pinMode(PIN_PTT, OUTPUT);
    digitalWrite(PIN_PTT, LOW);

    // Connect receiver to antenna and disable the SN74ACT244 PA
    pinMode(PIN_RCV, OUTPUT);
    digitalWrite(PIN_RCV, HIGH);

    // Receive
    si4735.setVolume(50);
    clearOutboundMessageDisplay();
    ui.setXmitRecvIndicator(INDICATOR_ICON_RECEIVE);

}  // receive_sequence()

/**
 * Turn-on the transmitter at F_Long for tuning
 *
 **/
void tune_On_sequence(void) {
    DTRACE();

    // ui.applicationMsgs->setText("TUNE");

    // Program the transmitter clock to F_Long
    uint64_t tuneFreq = thisStation.getFrequency() * 1000ULL * 100ULL;  // KQ7B tuning at FT8 base subband freq (e.g. 7074)
    // DPRINTF("tuneFreq=%llu Hz\n", tuneFreq / 100);
    si5351.set_freq(tuneFreq, SI5351_CLK0);  // Freq is in hundreths of a HZ

    // Drop the receiver's volume
    si4735.setVolume(0);

    // Turn-on the transmitter clock
    if (thisStation.getEnableTransmit()) si5351.output_enable(SI5351_CLK0, 1);

    // Disconnect receiver from antenna and enable the SN74ACT244 PA
    pinMode(PIN_RCV, OUTPUT);
    digitalWrite(PIN_RCV, LOW);

    // Short receiver's RF input to ground
    pinMode(PIN_PTT, OUTPUT);
    digitalWrite(PIN_PTT, HIGH);

    // Let loop() know we are tuning
    tune_flag = 1;
    ui.setXmitRecvIndicator(INDICATOR_ICON_TUNING);

    // DPRINTF("TUNE\n");

}  // tune_On_sequence()

/**
 * Turn-off transmitter w/o affecting outbound message[]
 **/
void tune_Off_sequence(void) {
    DTRACE();

    ui.applicationMsgs->setText(" ");

    // Turn-off the transmitter's clock
    si5351.output_enable(SI5351_CLK0, 0);

    // Float the receiver's RF input
    pinMode(PIN_PTT, OUTPUT);
    digitalWrite(PIN_PTT, LOW);

    // Connect receiver to antenna and disable SN74ACT244 PA
    pinMode(PIN_RCV, OUTPUT);
    digitalWrite(PIN_RCV, HIGH);

    // Crank-up the receiver volume
    si4735.setVolume(50);

    // Finished tuning
    tune_flag = 0;
    ui.setXmitRecvIndicator(INDICATOR_ICON_RECEIVE);

    // DPRINTF("TUNE OFF\n");

}  // tune_Off_sequence()

/**
 * @brief Recalculate carrier frequency, F_Long, and program Si5351 with unmodulated carrier frequency
 *
 * @note The Si5351 is programmed in terms of 0.01 Hz, not Hz nor kHz nor mHz.  Also recall that
 * the station frequency is stored as kHz.
 */
void set_Xmit_Freq() {
    F_Long = (uint64_t)((thisStation.getFrequency() * 1000 + thisStation.getCursorFreq() /* + offset_freq*/) * 100);
    si5351.output_enable(SI5351_CLK0, 0);
    si5351.set_freq(F_Long, SI5351_CLK0);
    delay(1);
    si5351.output_enable(SI5351_CLK0, 0);  // I think there is a sneak path in Si5351 that turns on clock when setting freq

}  // set_Xmit_Freq()

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

// Immediately turns on the transmitter's carrier at the current F_Long frequency.
// Sets xmit_flag notifying loop() to modulate the carrier, apparently the only place
// where this happens (i.e. if you want to have the carrier modulated, you must
// call setup_to_transmit_on_next_DSP_Flag).  I think the outbound string
// resides in the global message[].
void setup_to_transmit_on_next_DSP_Flag(void) {
    DTRACE();
    ft8_xmit_counter = 0;  // Reset symbol slot counter
    transmit_sequence();   // Turns-on the transmitter carrier at current F_Long ??
    // set_Xmit_Freq();                         //Recalculates F_long and reprograms SI5351 ??
    xmit_flag = 1;  // This flag appears to trigger loop() to modulate the carrier
    // ui.applicationMsgs->setText(get_message(), A_RED);  // Display transmitted message
}
