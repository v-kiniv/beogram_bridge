# beogram bridge
Connect Bang&Olufsen Beogram 5000 turntable with any receiver and remote. 

## BeoLink
Beogram 5000 use BeoLink protocol to connect with other B&O products(receiver, deck, etc.) using two datalink pins in 7din pinconnector.

Pinout:

![Image of Datalink](https://www.beoworld.org/beotech/mclconn/6_9b.jpg)

## Wiring
* Connect datalink pins to Arduino hardware or software UART. If you are using Software serial, modify serial initialization in setup.
* Connect IR sensor data pin to IR_RECEIVER_PIN(see sketch). 
* Connect RECEIVER_INPUT_PIN to receiver "Remote" port on your receiver(usually 3.5mm jack) used for IR remote retranslation.

## Setup
You'll need to figure out keycodes for your remote controller. 
Uncomment DEBUG_MODE define in sketch, flash firmware, open terminal - here you can see received keycodes.
Replace keycodes defined in nec_code_t enum by your own.
