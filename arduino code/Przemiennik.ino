#include "przemiennik.h"    //internal config file
#include "mp3tf16p.h"       //mp3 player lib
#include "OneWire.h"        // for temperature sesnor
#include "DallasTemperature.h" // for temperature sesnor https://github.com/milesburton/Arduino-Temperature-Control-Library/



#define FPSerial Serial    // we are using hardware serial for MP3player (not software serial)

// #define DEBUG 1

//HardwareSerial *dra_serial;
//dra_serial *Serial;  // same for radio modules...


DFRobotDFPlayerMini myDFPlayer;
OneWire oneWire(OneWire_pin);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;

static boolean transmit_active = false;
static uint8_t beep_one = 0;
unsigned long tail_timer;
unsigned long time_DTMF;
unsigned long last_beep_time;
unsigned long last_signal_change;
unsigned long first_oscilation_time;
unsigned long start_rx_delay;
unsigned long last_transmit_time;

boolean oscilation_detected = 0;
uint8_t oscilation_count;
uint8_t status_change_detected = 0;
uint8_t power_save_mode = 0;

uint8_t readSerialTimeout()
{
	uint8_t readed = false;
	unsigned long timeout = millis();
	while (millis() - timeout < 500) {
		delay(100);
		//wdt_reset();
		while (Serial.available() > 0) {
#ifdef DEBUG			
			Serial.write(Serial.read());
#else
			Serial.read();
#endif			
			readed = true;
		}
		if (readed == true)break;
	}
#ifdef DEBUG			
	if(!readed){
		Serial.println("Read timeout");
	}
#endif			
	return readed;
}

uint8_t SA818_begin(uint8_t device_pin)
{
	//not a commmand, but for sure
  digitalWrite(device_pin, HIGH);
//	Serial.print("AT\r\n");
  //Serial.print("AT\n\r");
	delay(100);

	for(uint8_t r = 0; r<5; r++){
		Serial.print("AT+DMOCONNECT\r\n");
    //Serial.print("AT+DMOCONNECT\n\r");
    
		if(readSerialTimeout()){
			//got +DMOCONNECT:0<CR><LF>
      delay(100);
      digitalWrite(device_pin, LOW);
			return true;
		}
	}
  delay(100);
  digitalWrite(device_pin, LOW);
	return false;
  
}

uint8_t SA818_setConfig(uint8_t device_pin, uint8_t bw, char* tx_f, char* rx_f, char* tx_ctcss, char* rx_ctcss, uint8_t squelch)
{

	digitalWrite(device_pin, HIGH);
	delay(100); 

	for(uint8_t r=0; r<5; r++){
		Serial.print("AT+DMOSETGROUP=");
		Serial.print(bw); // 0/1
		Serial.print(",");
		Serial.print(tx_f);//134-174/400-480, format to 415.1250
		Serial.print(",");
		Serial.print(rx_f);//format to 415.1250
		Serial.print(",");
		Serial.print(tx_ctcss);//format to 0000
		Serial.print(",");
		Serial.print(squelch); // <= 8
		Serial.print(",");
		Serial.print(rx_ctcss);
		Serial.print("\r\n");

		if(readSerialTimeout()){
      delay(100);
      digitalWrite(device_pin, LOW);
			return true;
		}
	}
  delay(100);
  digitalWrite(device_pin, LOW);
	return false;
}

uint8_t SA818_setVolume(uint8_t device_pin, uint8_t volume)
{
	// Set PTT off, so we can communicate with the uC.
	// Delay for a bit, to finish TX
	digitalWrite(device_pin, HIGH); 
	delay(100);

	if(volume>8) volume = 8;

	//sprintf(this->buffer,"AT+DMOSETVOLUME=%1d\r\n",this->volume);
	Serial.print("AT+DMOSETVOLUME=");
	Serial.print(volume);
	Serial.print("\r\n");
  delay(100);
  digitalWrite(device_pin, LOW); 
  return readSerialTimeout();

}

uint8_t SA818_setFilters(uint8_t device_pin, boolean preemph, boolean highpass, boolean lowpass)
{

	digitalWrite(device_pin, HIGH); 
	delay(100);

	//sprintf(this->buffer,"AT+SETFILTER=%1d,%1d,%1d\r\n",this->preemph,this->highpass,this->lowpass);
	//Serial.print(this->buffer);

	Serial.print("AT+SETFILTER=");
	Serial.print(preemph);
	Serial.print(",");
	Serial.print(highpass);
	Serial.print(",");
	Serial.print(lowpass);
	Serial.print("\r\n");

  delay(100);
  digitalWrite(device_pin, LOW);
	//return true; //must be +DMOSETFILTER: X (X=0->ok, X=1-fail)
	return readSerialTimeout();

}


void setup() {
  // put your setup code here, to run once:

  // pin mode config
  
  pinMode(Player_pin, OUTPUT);
  pinMode(Receiver_pin, OUTPUT);
  pinMode(Transmitter_pin, OUTPUT);

  pinMode(TX_PD, OUTPUT);
  pinMode(TX_PTT, OUTPUT);
  pinMode(TX_Low_power, OUTPUT);
  pinMode(Player_busy, INPUT);
  
  digitalWrite(TX_PTT, LOW);
  digitalWrite(TX_Low_power, LOW);            //LOW=high power, HIGH=low power

  pinMode(RX_Af_out, INPUT);
  attachInterrupt(digitalPinToInterrupt(RX_Af_out), Signal_received, CHANGE);

  pinMode(DTMF_0, INPUT);
  pinMode(DTMF_1, INPUT);
  pinMode(DTMF_2, INPUT);
  pinMode(DTMF_3, INPUT);
  pinMode(DTMF_STQ, INPUT);

  pinMode(Player_busy, INPUT);


  digitalWrite(Receiver_pin, LOW);
  digitalWrite(Transmitter_pin, LOW);
  digitalWrite(Player_pin, LOW);
  
  
  Serial.begin(9600);               //hardware serial setup

  sensors.getAddress(insideThermometer, 0);
  
  myDFPlayer.begin(FPSerial);

  // Configure RX module
  
  SA818_begin(Receiver_pin);
  SA818_setConfig(Receiver_pin, RX_bandwith, RX_frequency, RX_frequency, RX_CTCSS, RX_CTCSS, RX_squelch);  //RX and TX frequency same for RX module
  SA818_setVolume(Receiver_pin, RX_volume); // 0 to 8
  SA818_setFilters(Receiver_pin, 1, 0, 0);   //pin, preemph ON/OFF, higpass ON/OFF, lowpass ON/OFF
  digitalWrite(Receiver_pin, LOW);

 // Configure RX module
  digitalWrite(TX_PD, HIGH);
  delay(100);
  SA818_begin(Transmitter_pin);
  SA818_setConfig(Transmitter_pin, TX_bandwith, TX_freuency, TX_freuency, TX_CTCSS, TX_CTCSS, TX_squelch);  //RX and TX frequency same for RX module
  SA818_setVolume(Transmitter_pin, TX_volume); // 0 to 8
  SA818_setFilters(Transmitter_pin, 1, 0, 0);   //pin, preemph ON/OFF, higpass ON/OFF, lowpass ON/OFF
 
  digitalWrite(Transmitter_pin, LOW);   //finish comms with transmitter

  digitalWrite(Player_pin, HIGH);  //Transmitting only to MP3 player
  
  if (!myDFPlayer.begin(FPSerial, false)) {  //Use serial to communicate with mp3.
    digitalWrite(Player_pin, LOW);  
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  myDFPlayer.volume(20);  //Set volume value. From 0 to 30
  
  play_file_no(1);  //play repeater callsign file (file No1)
  
  
  digitalWrite(Player_pin, LOW); //finish transmitting to MP3 player
  
  //digitalWrite(TX_PTT, HIGH);
  //digitalWrite(TX_PD, LOW);
  
}

uint8_t read_RSSI()
{
  uint8_t temp_player_pin = digitalRead(Player_pin);
  uint8_t tmp_result = 0;
  digitalWrite(Player_pin, LOW); //finish transmitting to MP3 player

  digitalWrite(Receiver_pin, HIGH); 
	delay(100);
	Serial.print("RSSI?\r\n");
	delay(100);
  unsigned long timeout = millis();
	while (millis() - timeout < 500) {
		delay(100);
		//wdt_reset();
		while (Serial.available() > 0) {
    uint8_t tmp_r = Serial.read();
    //Serial.print(char(tmp_r));
   // Serial.print((tmp_r));
		if (tmp_r==61)   //we have "="  (replay will be "RSSI=XXX")
     { 
      uint8_t fd = Serial.read();  //first digit
      uint8_t sd = Serial.read();  //second digit
      uint8_t td = Serial.read();  //third digit
      tmp_result = fd-48;
      if (sd>47) {tmp_result = (tmp_result*10+(sd-48));}
      if (td>47) {tmp_result  =(tmp_result*10+(td-48));}
     }		
	  }  //while
  }

  digitalWrite(Receiver_pin, LOW); 
  digitalWrite(Player_pin,temp_player_pin);
  return tmp_result;

}

uint8_t read_DTMF()
{
  uint8_t DTMF_code=0;
  
  DTMF_code=digitalRead(DTMF_3);
  DTMF_code=DTMF_code<<1;
  DTMF_code+=digitalRead(DTMF_2);
  DTMF_code=DTMF_code<<1;
  DTMF_code+=digitalRead(DTMF_1);
  DTMF_code=DTMF_code<<1;
  DTMF_code+=digitalRead(DTMF_0);
  return DTMF_code;
}

void play_file_no(uint8_t filenumber)
{
  digitalWrite(TX_PD,HIGH);
  digitalWrite(TX_PTT,HIGH);  //switch on TX
  if (power_save_mode) {   //if we are in power save mode, go back to normal
    Serial.print(0x0B);
    delay(500);}
  power_save_mode = 0;

  if (!transmit_active) { delay(700); }                  //if not on, wait since it takes a long time to activate transmiter...
  myDFPlayer.play(filenumber);         //transmit reuested file 
  delay(200);                 //smoe more time to start playing...
  while (!digitalRead(Player_busy)){   //wait untill finis playing...
       delay(10);
  }
  if (!transmit_active){        //if we are not in the middle of transmission
    digitalWrite(TX_PTT,LOW);  //switch off TX
    last_transmit_time = millis();
   
  } 
}

void play_number(float number, uint8_t vol_tem = 0)
{
  uint8_t filetoplay;
  uint8_t hundreds;
  uint8_t tens;
  uint16_t number_to_play = number;
  uint8_t float_part;

  float_part=10*(number-number_to_play);
  boolean transmit_was_active=transmit_active;

  digitalWrite(TX_PD,HIGH);
  digitalWrite(TX_PTT,HIGH);  //switch on TX
  if (power_save_mode) {   //if we are in power save mode, go back to normal
    Serial.print(0x0B);
    delay(500);}
  power_save_mode = 0;

  if (!transmit_active) delay(700);                  //if not on, wait since it takes a long time to activate transmiter...
  transmit_active=1;
  
  if ((vol_tem==1) || (vol_tem==2)) {play_file_no(2+(vol_tem*2));}
  if (vol_tem==3) {play_file_no(8);}
  if (number_to_play>100) {
     hundreds=number_to_play/100;
     number_to_play=number_to_play-(hundreds*100);
     if (number_to_play!=0) play_file_no(hundreds+40);  
  }
  if (number_to_play>20) {
    tens=(number_to_play/10);
    play_file_no(tens+30);
    number_to_play=number_to_play-(tens*10);
  }
  if (!((number>19) && (number_to_play==0))) {  //play number from 0 to 19 if it is not zero after full number of tens (not to say "twenty zero")
  play_file_no(number_to_play+10); }

  if (float_part>0){
    play_file_no(9);  //przecinek
    play_file_no(float_part+10);
  }
  if ((vol_tem==1) || (vol_tem==2)) {play_file_no(3+(vol_tem*2));}

  if (!transmit_was_active) {
    digitalWrite(TX_PTT,LOW);
    last_transmit_time = millis();
    
  }
  transmit_active=transmit_was_active;
}

float read_voltage()
{
  return (analogRead(Analog_in)/40.1);        //10-bit - max value 1024 - 5V, voltage divider set to 5 (25Vin=1024)
}

void loop() {
  // put your main code here, to run repeatedly:
static unsigned long timer = millis();
digitalWrite(Player_pin, HIGH);  //Transmitting only to MP3 player
  
  if (status_change_detected) RX_stats_change();

  if (millis() - timer > Repeater_callsign_time) {    //send repeater callsign enery...
    timer = millis();
    
    play_file_no(1);  //play repeater callsign file (file No1)
  }

 if ((transmit_active) && ((millis()-tail_timer) > 2000) && (digitalRead(RX_Af_out))) {  //if we are not receiving anything transmitting and more than 2000ms from last rx signal
  digitalWrite(TX_PTT,LOW);  //switch off TX
  last_transmit_time = millis();
  transmit_active=0;         // set TX flag to 0

 }

 if ((digitalRead(DTMF_STQ)) && (millis()>(time_DTMF+500)) && (DTMF_Enabled)) {   //if we have DTMF detected and more than 0.5s from last DTMF.
  uint8_t temp_num=read_DTMF();
  play_number(temp_num);
  if (temp_num==11) {
    play_number(read_voltage(),2);
  }
  if (temp_num==12) {
    sensors.requestTemperatures();
    play_number(sensors.getTempC(insideThermometer),1);
  }
  if (temp_num==10) {
    uint8_t RSSI_read = read_RSSI();
    //play_number(152);
    play_number(RSSI_read, 3);
  }
  time_DTMF=millis();
 }

 if ((power_save_mode == 0) && (millis() > (last_transmit_time + power_save_delay))) {  //switch to power save mode (power down transmitter) after power_save_delay since last transmision
  power_save_mode = 1;
  digitalWrite(TX_PD, LOW);   //power down transmitter
  //mp3 module should go to sleep as well...
  Serial.print(0x0A);

 }


}

void RX_stats_change()                   
{
    status_change_detected = 0;

    if ((!transmit_active) && (!digitalRead(RX_Af_out)) && (!oscilation_detected)) {    //begin of receiving (not transmiting)
    
    uint8_t continous_signal = 1;
    
    start_rx_delay = millis();
    


    while (millis() < (RX_delay + start_rx_delay)) 
       {
       if (digitalRead(RX_Af_out)) { continous_signal=0;}    //check if there is no braks in the signal for RX_delay preriod
       }
      if (continous_signal) {   //if signal was continous for this time
        
        transmit_active=1;
        digitalWrite(TX_PD,HIGH);
        digitalWrite(TX_PTT,HIGH);
        power_save_mode = 0;
     }
  }

  if ((transmit_active) && (digitalRead(RX_Af_out))) {    //we are transmitting, but there is no more signal...
      //noInterrupts();        //temporarly disable interupts - not to call same function again...
      
      // beep only if, last signal change was >0.5s ago, last time beeped more tan minimum time and beep is enabled
      if ((Beep_on) && ((millis() - last_beep_time) > minimum_beep_time) && ((millis() - last_signal_change) > 500)) {
         myDFPlayer.play(beep_one+2);    //play beep of various tone
         last_beep_time = millis();

      }

      beep_one++;         //alternative beep tones...
      if (beep_one>1) beep_one=0;

      if ((digitalRead(RX_Af_out))) {  //if there is still no more signal...
        tail_timer = millis();
      }
      //interrupts();
      

  }
}

void Signal_received(){              // function called on change of signal detect pin

//With receiver module we have problem with self-oscilation in case of small signal. Signal appears and disapears in short successions, causing beep to activate
//millis() is not changing values in this routine (ISR), so any routines using those have to be made outside of this space...
  
  //noInterrupts();

  if ((millis() - last_signal_change) < 500) {   //if last change of signal was lestt than ... we need to check for oscilations
      if (oscilation_count==0) { first_oscilation_time = millis();}  //if this is first occurance, note the time
      oscilation_count++;  
  }
  if (oscilation_count>8) { 
    oscilation_detected = 1;
    transmit_active = 0;
    digitalWrite(TX_PTT,LOW);
    last_transmit_time = millis();
  }
  if ((millis() - last_signal_change) > 500) { 
    oscilation_count = 0;
    oscilation_detected = 0;
  }

 

  digitalWrite(LED_pin, !digitalRead(RX_Af_out));   // show status of signal on LED
  
    

        


  status_change_detected = 1;

  last_signal_change = millis();

  
  
  //digitalWrite(TX_PTT, !digitalRead(RX_Af_out));

}