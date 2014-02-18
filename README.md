footpedal
=========

3d printed foot pedal. Powered by Teensy 3. Has adjustable microswitch and potentiometer.


* Can send a configurable string of characters or both press and release
  - e.g. Press triggers CTRL-C Release triggers CTRL-V
* Supports all modifer keys (Crtl, ALt, Shift, Win etc)
  - As well as setting multiple keys at once
* supports key send on press and key release on release (for press CTRL etc)
  - Allows holding the key down
* has a pot for analogue joystick input (not fully implemented)
* daisy chainable pedals for up to 250 devices
  - Each device can be configured through one "master" device.
  - Each device can be the master device
  - You can have multiple masters... if you really want ?!?
* configurable keys sequences and options from QT application

Teensy config
=============

Pin setup:-

GND           - Pushbutton (any pin)
Digital 12    - Pushbutton (remaining pin)
A0 (Analogue) - Potentiometer centre pin
Pot end pin   - GND
Pot other end - +3.3V

--- to be documented!
RX1           - Serial connector
TX1
RX2
TX2


TODO:
Test multiple serial connected units.
Add automatic ID system
Add broadcast feature

Model and more description at
http://www.thingiverse.com/thing:249020

Barry Carter
Barry.carter@gmail.com
