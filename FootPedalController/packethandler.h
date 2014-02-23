/*
  FuzzPedal foot pedal host configurator.
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
#ifndef PACKETHANDLER_H
#define PACKETHANDLER_H

#include <QObject>

#define PACKET_SIZE 7

enum PACKET_FORMAT {
  ID_LOC = 0,
  SRC_LOC,
  CMD_LOC,
  DATA_LOC_0,
  DATA_LOC_1,
  DATA_LOC_A,
  DATA_LOC_A_1
};

enum PACKET_IDS {
    ID_UNCONFIGURED_SLAVE  = 252,
    ID_BROADCAST_CHAIN1,
    ID_BROADCAST_CHAIN2,
    ID_BROADCAST_BOTH
};

enum PacketCommandTypes {
    CMD_JOYSTICK_X = 0,                  // We can send a joystick command
    CMD_JOYSTICK_Y,                      // For X or Y
    CMD_BUTTON1,                         // When the switch is pressed or released, we send some keystrokes
    CMD_BUTTON2,                         // second button not implemented yet
    CMD_SERIAL_USB,                      // button press send a serial command
    CMD_BUTTON_ON_PRESS,                 // button press send a keystroke, and holds it down
    CMD_BUTTON_ON_RELEASE
};

enum PacketCommands {
    // Configuration section. Devices can send these commands to configure the pedal and keypresses
    CMD_CONFIGURE_ID = 30,
    CMD_CONFIGURE_BUT_CMD,
    CMD_CONFIGURE_BUT_KEY_PRESS,
    CMD_CONFIGURE_BUT_MODIFIER_PRESS,
    CMD_CONFIGURE_BUT_KEY_RELEASE,
    CMD_CONFIGURE_BUT_MODIFIER_RELEASE,
    CMD_CONFIGURE_POT_CMD,
    CMD_GET_ID = 50,
    CMD_GET_BUT_CMD,
    CMD_GET_POT_CMD,
    CMD_GET_POT_VALUE,
    CMD_CONFIGURE_GET_KEYS_PRESS,
    CMD_CONFIGURE_GET_MODIFIERS_PRESS,
    CMD_CONFIGURE_GET_KEYS_RELEASE,
    CMD_CONFIGURE_GET_MODIFIERS_RELEASE,
    CMD_CONFIGURE_SAVE,
    CMD_CONFIGURE_DEBUG_MSG = 70         // Sends a magic packet before sending serial data so the conf app doesn't break
};


class DataPacket
{
public:
    DataPacket();
    unsigned char buffer[PACKET_SIZE];
    unsigned int bytesReceived;
    u_int8_t nodeID;
    u_int8_t sourceID;
    u_int8_t command;
    u_int16_t payload;
    u_int16_t payloadExtra;
};


class PacketHandler : public QObject
{
    Q_OBJECT

public:
    PacketHandler();
    void sendPacket(DataPacket *packet);
    DataPacket *sendPacket(int toNode, int fromNode, int command, int payload, int payloadExtra);
    void buildPacket(DataPacket *packet, char data);
    DataPacket *packetHandle();
    void getPotValue(int toNode);

signals:
    void sendSerial(const QByteArray &data);
    void gotKeyConfig(int idx, char key);
    void gotModifierConfig(int idx, char mod);
    void gotKeyReleaseConfig(int idx, char key);
    void gotModifierReleaseConfig(int idx, char mod);
    void gotDebugMessage(QString message);
    void gotDeviceID(unsigned int id);
    void gotButtonMode(unsigned int mode);
    void gotPotAxisMode(unsigned int mode);
    void gotPotValue(unsigned int value);

private:
    void processReceivedPacket(DataPacket *packet);
    u_int16_t arrToValue16(unsigned char *buffer, int offset);

    DataPacket serialPacket;

    bool gotSlashN;
    bool receivingString;
};

#endif // PACKETHANDLER_H
