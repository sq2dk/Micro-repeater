// Pins definitions

//serial comms RX enable pins

#define Player_pin 10
#define Receiver_pin 2
#define Transmitter_pin 12

#define LED_pin 13

//oter pins
// #define Analog_in A0  //old design
#define Analog_in A6  // new design
#define Player_busy 9
#define TX_PTT  A1    // transmit on low impedance
#define TX_PD   A2    //power down
#define TX_Low_power A3
#define RX_Af_out  3 // receiver active - signal present - LOW

#define DTMF_Enabled 1  //0 - disable, 1 enabled
#define DTMF_0 4
#define DTMF_1 5
#define DTMF_2 6
#define DTMF_3 7
#define DTMF_STQ 8
// #define DTMF_STQ 11  //old design

#define OneWire_pin 11
//#define OneWire_pin 8  // old design

// Receier module configuration

#define RX_frequency "145.1125"   //for cross-band
//#define RX_freuency "430.5125"   //for 70cm only repeater
#define RX_bandwith 0     // 0=12.5kHz, 1=25kHz
#define RX_volume 6       //0=min, 8=max
#define RX_CTCSS "0010"   //"0010"=94.8 
#define RX_squelch 5       //0-8

#define RX_delay 100     // 100ms - delay for avoid opening on super-short signals - minum length of signal opening repeater

// Transmitter module configuration
#define TX_freuency "438.1125"  //for cross-band
// #define TX_freuency "438.1125"  //for 70cm rpt
#define TX_bandwith 0     // 0=12.5kHz, 1=25kHz
#define TX_volume 3       //0=min, 8=max
#define TX_CTCSS "0010"   //"0010"=94.8 
#define TX_squelch 8

//Beeps 
#define Beep_on 0  //0-off, 1-on
#define minimum_beep_time 2000   // minimum time in ms between beeps

//Repeater callsign set-up
#define Repeater_callsign_active 1   // 1-active (repeater introduces itself every Repeater_callsign_time), 0-repeater does not introduces itself
#define Repeater_callsign_time 600000   //in ms : 600000 - every 10 minutes

//Power saving
#define power_save_enable 1       // 1- power save enabled, 0 - power save disabled
#define power_save_delay 10000    //10s (10000ms)  delay after receiving a signal when repeater goes to low power mode.

//oscilation detection - rapid (osc_det_timeout) changes of signal / no signal states could indicate of self-oscilations - happens sometimes when TX module is beeing received on RX module.
#define osc_det_active 1  //oscilation protection on=1, off=0
#define osc_det_timeout 500 // [ms]    - 

