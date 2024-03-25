# Micro-repeater
Two way radio FM micro repeater based on SA-818 modules

This project is by no means complete and finalized. Care must be taken while reproducing, as design might have faults. For radio-amateur use only. You have to have license to experiment with radio transmissions and have to be aware of consequences of your actions!

Repeater can be used to extend usable range of two way portable VHF/UHF radio by placing it in a convenient location between both radios. It consists of receiver (RX) and transmitter (TX) which work at the same time while TX is transmitting everything what is received by RX.
In this project two ready-made TX/RX (SA-818) modules were used. Depending on chosen band of these modules, various band configuration can be achieved. The lowest cost repeater can be done by using 2m / 70cm (cross-band) configuration, since for this you do not need expensive duplexer.

Repository consist of Kicad design files, Arduino code and whatever information might be required.

## Concept:
### -Radio
Small footprint and light weight is achieved by miniature RX/TX modules. Those are based around SDR based chip RDA1847, which is not perfect, but is basically self-contained two-way radio in itself. SA-818 modules consist also microcontroller and power amplifier to achieve about 1W output power. No RF output filtering is made, so at least low-pass filter should be added at the output of TX. RX achieving reasonable sensitivity (better than -120dBm). Communication and set-up of modules is done by serial communication (AT commands). 
### -Microcontroller
Clone of Arduino pro-mini was used. Basically minimal computational power is required, so ATMega8 is more than enough. Small footprint and cost is another benefit. Downside is the programming interface, but almost any USB<->serial TTL interface can be used. Basically it was used, because it was on hand :-).
### -MP3 Player
In some (most?) of the countries, repeaters are required to transmit their call signs aery now and then. This is done by employing small MP3 player module (MP3-TF-16P), which can play MP3 files stored on SD card. On this card various sound files are stored, which also allow transmitting numerical values (such as voltage, temperature etc) by voice. Communication with the module is done by serial protocol.
### -DTMF
DTMF decode module based on MT8870D is added to be able to decode commands sent by user's radio keys. This is fully optional, at the moment requests for voltage, temperature and signal strength are implemented. Others can be coded.
### -Serial communication
As mentioned above, both radio modules and MP3 players are communicating by the serial protocol. All are connected to the same bus, thus additional switches of bus lines (made on transistors) are added, so only one device is able to "talk" with microcontroller at the time. Not perfect, but it works. This has added a lot of complexity to the design, but the principle is easy.
### -Power supply
Two voltages are required - 5V for DTMF module and about 4.3 for MP3 and radio modules. Arduino is also powered by 4.3V, which is fine (you have to use 5V variant). Device can be powered by 5V (from USB) or 7-15V. If you supply 5V, it has to be stabilized. Do not connect both sources at the same time. If you use 5V input jumper pin has to be installed. 5-15V is feed to DC-DC buck converter based on ready module and converted to 4.3V. In case of 7-15V input, 5V is produced by 78L05 linear stabilizer, in case of 5V supply it is feed directly. Reverse-polarity diode is installed, so in case of connection 7-15V short will be made.
### -Others
DS18D20 1-wire temperature sensor can be connected to get temperature readings
2 pins are provided to connect voltage reading. Voltages up to 20V should be fine (voltage divider can be changed in case of other requirements). Calibration of readings is required in software in case of using.

## PCB
PCB is designed to be able to do as single layer at home. Most of the traces are thick, as I was using home-made PCBs for prototyping. In case of homemade PCB recommend to use 2-layer PCB and leave one side with full-copper (ground) layer. Some "bodge" wires are unfortunately required. Complete device has dimensions of approx 10 x 10 x 2 cm. It can definitely be shrunk-down, by re-designing PCB.

## Challenges:
-Power consumption - at the moment, it is not optimized for power consumption. Additional power-save modes are required in software (and in hardware)
-RF interference - modules and their surroundings require additional shielding. On entrance of RX, strong remains of TX transmission is present. Additional shielding of both modules should be made.

Jedrzej Marsz SQ2DK
