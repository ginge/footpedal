/*
  FuzzPedal foot pedal controller.
  Copyright (C) 2014  Barry Carter <barry.carter@gmail.com>
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <Bounce.h>
#include <EEPROM.h>

Bounce button11 = Bounce(11, 10);
Bounce button12 = Bounce(12, 10);

#define PACKET_SIZE 7

enum PACKET_FORMAT {
  ID_LOC = 0,
  SRC_LOC,
  CMD_LOC,
  DATA_LOC_0,
  DATA_LOC_1,
  DATA_LOC_A,
  DATA_LOC_A_1,
};

enum COMMANDS {
  CMD_JOYSTICK_X = 0,                  // We can send a joystick command
  CMD_JOYSTICK_Y,                      // For X or Y
  CMD_BUTTON1,                         // When the switch is pressed or released, we send some keystrokes
  CMD_BUTTON2,                         // second button not implemented yet
  CMD_SERIAL_USB,                      // button press send a serial command
  CMD_BUTTON_ON_PRESS,                 // button press send a keystroke, and holds it down
  CMD_BUTTON_ON_RELEASE,
  // Configuration section. Devices can send these commands to configure the pedal and keypresses
  CMD_CONFIGURE_ID = 30,
  CMD_CONFIGURE_BUT_CMD,
  CMD_CONFIGURE_BUT_KEY_PRESS,
  CMD_CONFIGURE_BUT_MODIFIER_PRESS,
  CMD_CONFIGURE_BUT_KEY_RELEASE,
  CMD_CONFIGURE_BUT_MODIFIER_RELEASE,
  CMD_CONFIGURE_POT_CMD,
  CMD_CONFIGURE_GET_KEYS_PRESS=50,
  CMD_CONFIGURE_GET_MODIFIERS_PRESS,
  CMD_CONFIGURE_GET_KEYS_RELEASE,
  CMD_CONFIGURE_GET_MODIFIERS_RELEASE,
  CMD_CONFIGURE_SAVE,
  CMD_CONFIGURE_DEBUG_MSG = 70         // Sends a magic packet before sending serial data so the conf app doesn't break
};

enum SERIAL_PORTS {
  PORT_USB,
  PORT_SER1,
  PORT_SER2
};

// Our data packet structure to hold all packet info
typedef struct DataPackets {
  byte port;
  byte buffer[PACKET_SIZE];
  byte bytesReceived;
  byte nodeID;
  byte sourceID;
  byte command;
  int payload;
  int payloadExtra;
} DataPacket;
  
DataPacket packetSerial1;
DataPacket packetSerial2;
DataPacket packetSerialUSB;

// Configuration stuff
#define MAX_KEY_SEQ 10

struct {
  byte ID               = 1; // 255 is reserved
  // change these lines below to have hard coded default key presses
  byte keyboardKeysPress[MAX_KEY_SEQ]        = {KEY_HOME, KEY_C, KEY_DELETE, KEY_V, 0};
  byte keyboardModifiersPress[MAX_KEY_SEQ]   = {KEY_LEFT_SHIFT, KEY_LEFT_CTRL, 0, KEY_LEFT_CTRL, 0};
  byte keyboardKeysRelease[MAX_KEY_SEQ]      = {KEY_HOME, KEY_A, KEY_DELETE, KEY_V, 0};
  byte keyboardModifiersRelease[MAX_KEY_SEQ] = {KEY_LEFT_SHIFT, KEY_LEFT_CTRL, 0, KEY_LEFT_CTRL};
  byte buttonCommand    = CMD_BUTTON1;       // function that is executed on button press
  byte potAxisCommand   = CMD_JOYSTICK_X;
  int  potSampleFreq    = 10;  //ms
  int  potReportFreq    = 100;  //ms  
} Conf; 

// prototypes
void parseSerial(DataPacket *packet, byte data);
void sendDataType(DataPacket *packet);
void serialRelay(DataPacket *packet);
void packetSend(DataPacket *packet, HardwareSerial *port);
void USBSend(DataPacket *packet);
void processOwnCommand(DataPacket *packet);
void saveEEPROM();

// go go go
void setup() {

  // load eeprom settings  
  for (int i = 0; i < MAX_KEY_SEQ; i++) {
    // read the keypress contents in
    Conf.keyboardKeysPress[i] = EEPROM.read(i);
  }
  for (int i = MAX_KEY_SEQ; i < 2*MAX_KEY_SEQ; i++) {
    // read the keypress contents in
    Conf.keyboardModifiersPress[i-MAX_KEY_SEQ-1] = EEPROM.read(i);
  }
  for (int i = 2*MAX_KEY_SEQ; i < 3*MAX_KEY_SEQ; i++) {
    // read the keypress contents in
    Conf.keyboardKeysRelease[i-2*MAX_KEY_SEQ-1] = EEPROM.read(i);
  }
  for (int i = 3*MAX_KEY_SEQ; i < 4*MAX_KEY_SEQ; i++) {
    // read the keypress contents in
    Conf.keyboardModifiersRelease[i-3*MAX_KEY_SEQ-1] = EEPROM.read(i);
  }
  
  // Setup the USB Serial
  Serial.begin(9600);
  
  // configure the joystick to manual send mode.  This gives precise
  // control over when the computer receives updates, but it does
  // require you to manually call Joystick.send_now().
  Joystick.useManualSend(true);
  
  // Enable pullups on switch inputs
  pinMode(11, INPUT_PULLUP);  // Not used yet
  pinMode(12, INPUT_PULLUP);  // Switch connected to here

  // Start hardware serial ports
  Serial1.begin(9600);
  Serial2.begin(9600);

  // set the defaults
  packetSerial1.port = PORT_SER1;
  packetSerial2.port = PORT_SER2;
  packetSerialUSB.port = PORT_USB;
  packetSerial1.bytesReceived = 0;
  packetSerial2.bytesReceived = 0;
  packetSerialUSB.bytesReceived = 0;
  
  // initialise control over the keyboard:
  Keyboard.begin();
  
  Serial.println("HeadFuzz FuzzPedal initialised");
}

void loop() {

  checkPot();  
  checkButtons();
  checkSerial();
 
  // a brief delay, so this runs "only" 200 times per second
//  delay(5);
}

void checkPot() {
  int potVal = analogRead(0);
  
  // read 6 analog inputs and use them for the joystick axis
  //Joystick.X(potVal);

  // Because setup configured the Joystick manual send,
  // the computer does not see any of the changes yet.
  // This send_now() transmits everything all at once.
  //Joystick.send_now();

  //Serial.println(potVal);  
}

void checkButtons() {
  byte pressed = 0;
  byte rising = 0;
  
  // read the button states
  button12.update();
   
  // Button Pressed
  if (button12.fallingEdge()) {
    pressed = 1;
    Serial.println("Button Press");
  }
  
  // button released
  if (button12.risingEdge()) {
    pressed = 1;
    rising = 1;
    Serial.println("Button Release");
  }
  
  if (pressed) {
    // send a fake data packet to ourselves for processing
    DataPacket packet;
    for (int n=0;n < MAX_KEY_SEQ; n++) {
      if (!rising) {
        packet.command = Conf.buttonCommand;
        packet.payload = (int)Conf.keyboardKeysPress[n];
        packet.payloadExtra = Conf.keyboardModifiersPress[n];
      }
      else {
        packet.payload = (int)Conf.keyboardKeysRelease[n];
        packet.payloadExtra = Conf.keyboardModifiersRelease[n];
        
        // if we have the press command set, we need the next command to be a release
        if (Conf.buttonCommand == CMD_BUTTON_ON_PRESS)
        {
          packet.command = CMD_BUTTON_ON_RELEASE;
        }
      }
            
      sendDataType(&packet);
    }
  }

}

void checkSerial() {
  byte ch;
  // do the serial checks to see if there is data from the other slave
  // devices.
  // we have each device slaved using 2 uarts per device. The incoming data
  // has an ID which is checked. If the ID is 0, it is sent to the USB
  // if the ID doesn't match ours, it is sent through the other serial
  if (Serial1.available() > 0) {
    ch = Serial1.read();
    parseSerial(&packetSerial1, ch);
  }

  if (Serial2.available() > 0) {
    ch = Serial2.read();
    parseSerial(&packetSerial2, ch);
  }
  
  
  // check the host USB interface has any data for us
  if (Serial.available() > 0) {
    ch = Serial.read();
    parseSerial(&packetSerialUSB, ch);
  }
}


// build up a serial packet until we get a complete data packet
void parseSerial(DataPacket *packet, byte data) {
  packet->buffer[packet->bytesReceived] = data;

  packet->bytesReceived++;

  if (packet->bytesReceived == PACKET_SIZE) {
    // we got the packet
    packet->nodeID = packet->buffer[ID_LOC];
    packet->sourceID = packet->buffer[SRC_LOC];
    packet->command = packet->buffer[CMD_LOC];

    // now do stuff with the packet
    packet->payload = packetToValue16(packet->buffer, DATA_LOC_0);
    packet->payloadExtra = packetToValue16(packet->buffer, DATA_LOC_A);
  
    // node is set to go to the USB host interface
    if (packet->nodeID == 0) {
      sendDataType(packet);
    }
    else if (packet->nodeID == Conf.ID) {
      // It matched our ID! process it
      processOwnCommand(packet);
    }
    else {
      // send the packet out the other Serial port
      serialRelay(packet);
    }

    packet->bytesReceived = 0;
  }
}

// process a command sent to our ID
void processOwnCommand(DataPacket *packet) {
  switch (packet->command) {
    case CMD_CONFIGURE_BUT_KEY_PRESS:
      Conf.keyboardKeysPress[packet->payload] = packet->payloadExtra;
      break;
    case CMD_CONFIGURE_BUT_KEY_RELEASE:
      Conf.keyboardKeysRelease[packet->payload] = packet->payloadExtra;
      break;
    case CMD_CONFIGURE_BUT_MODIFIER_PRESS:
      Conf.keyboardModifiersPress[packet->payload] = packet->payloadExtra;
      break;
    case CMD_CONFIGURE_BUT_MODIFIER_RELEASE:
      Conf.keyboardModifiersRelease[packet->payload] = packet->payloadExtra;
      break;    
    case CMD_CONFIGURE_BUT_CMD:
      Conf.buttonCommand = packet->payload;
      break;
    case CMD_CONFIGURE_ID:
      Conf.ID = (int)(packet->payload);
      break;

    case CMD_CONFIGURE_POT_CMD:
      break;
    case CMD_CONFIGURE_GET_KEYS_PRESS:
      for (int n = 0; n < MAX_KEY_SEQ; n++) {
         DataPacket packet;
         packet.command = CMD_CONFIGURE_GET_KEYS_PRESS;
         packet.payload = n;
         packet.payloadExtra = Conf.keyboardKeysPress[n];
         USBSend(&packet);
      }
      debugPrint("Stored Press Key Sent");
      break;
    case CMD_CONFIGURE_GET_MODIFIERS_PRESS:
      for (int n = 0; n < MAX_KEY_SEQ; n++) {
         DataPacket packet;
         packet.command = CMD_CONFIGURE_GET_MODIFIERS_PRESS;
         packet.payload = n;
         packet.payloadExtra = Conf.keyboardModifiersPress[n];
         USBSend(&packet);
      }
      debugPrint("Stored Press Modifiers Sent");
      break;
    case CMD_CONFIGURE_GET_KEYS_RELEASE:
      for (int n = 0; n < MAX_KEY_SEQ; n++) {
         DataPacket packet;
         packet.command = CMD_CONFIGURE_GET_KEYS_RELEASE;
         packet.payload = n;
         packet.payloadExtra = Conf.keyboardKeysRelease[n];
         USBSend(&packet);
      }
      debugPrint("Stored Release Key Sent");
      break;
    case CMD_CONFIGURE_GET_MODIFIERS_RELEASE:
      for (int n = 0; n < MAX_KEY_SEQ; n++) {
         DataPacket packet;
         packet.command = CMD_CONFIGURE_GET_MODIFIERS_RELEASE;
         packet.payload = n;
         packet.payloadExtra = Conf.keyboardModifiersRelease[n];
         USBSend(&packet);
      }
      debugPrint("Stored Release Modifiers Sent");
      break;
    case CMD_CONFIGURE_SAVE:
      saveEEPROM();
      break;
  }
}

// send a command via serial/usb-serial/joystick or other
// this is the main function that is triggered on a button press
void sendDataType(DataPacket *packet) {
  
  switch (packet->command) {
    case CMD_JOYSTICK_X:
      //Joystick.X(packet->payload);
      //Joystick.send_now();
      break;
    case CMD_JOYSTICK_Y:
      //Joystick.Y(packet->payload);
      //Joystick.send_now();
      break;
    case CMD_BUTTON1:
      // send any control etc keys too
      if (packet->payloadExtra > 0) {
        Keyboard.set_modifier((byte)packet->payloadExtra);
      }
      else {
        Keyboard.set_modifier(0);
      }
      Keyboard.send_now();
      Keyboard.set_key1(packet->payload);
      Keyboard.send_now();
      delay(10);
      Keyboard.set_modifier(0);
      Keyboard.set_key1(0);
      Keyboard.send_now();
      break;
    case CMD_BUTTON2:
      Keyboard.print(packet->payload, BYTE);
      break;
    // Press and hold a key instead of sending a stream of chars
    case CMD_BUTTON_ON_PRESS:
      if (packet->payloadExtra > 0) {
        Keyboard.set_modifier((byte)packet->payloadExtra);
      }
      else {
        Keyboard.set_modifier(0);
      }
      Keyboard.send_now();
      Keyboard.set_key1(packet->payload);
      Keyboard.send_now();
      break;
    // release all keys
    case CMD_BUTTON_ON_RELEASE:
      Keyboard.set_modifier(0);
      Keyboard.set_key1(0);
      Keyboard.send_now();
      break;
    case CMD_SERIAL_USB:
      USBSend(packet);
  }
  
}

// send the packet to the other serial port(s)
void serialRelay(DataPacket *packet) {
  if (packet->port == PORT_SER1) {
    packetSend(packet, &Serial2);
  }
  else if (packet->port == PORT_SER2) {
    packetSend(packet, &Serial1);
  }
  else if (packet->port == PORT_USB) {
    // if it comes from the USB host, send to both serial ports.
    // just in case we have chained devices on both sides
    packetSend(packet, &Serial1);
    packetSend(packet, &Serial2);
  }
}

int packetToValue16(byte *buffer, int offset) {
  int val = 0;
  val |= buffer[offset];
  val = val << 8;
  val |= buffer[offset+1];
  
  return val;
}

void packetSend(DataPacket *packet, HardwareSerial *port) {
  // assemble the packet
  /*
  8 bits destination ID
  8 bits source ID
  8 bits command
  16 bits command data
  */ 
  port->print(packet->nodeID, BYTE);
  port->print(packet->sourceID, BYTE);
  port->print(packet->command, BYTE);
  port->print((packet->payload >> 8) & 0xFF, BYTE);
  port->print(packet->payload & 0xFF, BYTE);
  port->print((packet->payloadExtra >> 8) & 0xFF, BYTE);
  port->print(packet->payloadExtra & 0xFF, BYTE);

  port->println();
}

// annoying we need this, but the HardwareSerial and USBSerial classes are
// not compatible without lots of code. easier to replicate those few lines
void USBSend(DataPacket *packet) {
  Serial.print(packet->nodeID, BYTE);
  Serial.print(packet->sourceID, BYTE);
  Serial.print(packet->command, BYTE);
  Serial.print((packet->payload >> 8) & 0xFF, BYTE);
  Serial.print(packet->payload & 0xFF, BYTE);
  Serial.print((packet->payloadExtra >> 8) & 0xFF, BYTE);
  Serial.print(packet->payloadExtra & 0xFF, BYTE);

  //Serial.println();  
}

// sends a magic packet to the host before sending serial data. For compatibility with 
// a host side packet parser (config etc)
void debugPrint(const char str[]) {
  DataPacket packet;
  packet.command = CMD_CONFIGURE_DEBUG_MSG;
  packet.payload = 0;
  packet.payloadExtra = 0;
  USBSend(&packet);
  Serial.println(str);  
}

void saveEEPROM() {
  for (int i = 0; i < MAX_KEY_SEQ; i++) {
    // write the keypress contents in
    EEPROM.write(i, Conf.keyboardKeysPress[i]);
  }
  for (int i = MAX_KEY_SEQ; i < 2*MAX_KEY_SEQ; i++) {
    // write the keypress contents in
    EEPROM.write(i, Conf.keyboardModifiersPress[i-MAX_KEY_SEQ-1]);
  }
  for (int i = 2*MAX_KEY_SEQ; i < 3*MAX_KEY_SEQ; i++) {
    // write the keypress contents in
    EEPROM.write(i, Conf.keyboardKeysRelease[i-2*MAX_KEY_SEQ-1]);
  }
  for (int i = 3*MAX_KEY_SEQ; i < 4*MAX_KEY_SEQ; i++) {
    // write the keypress contents in
    EEPROM.write(i, Conf.keyboardModifiersRelease[i-3*MAX_KEY_SEQ-1]);
  }
}

