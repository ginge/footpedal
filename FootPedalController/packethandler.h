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

enum PacketCommands {
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
    CMD_CONFIGURE_DEBUG_MSG = 70
};


class DataPacket
{
public:
    DataPacket();
    char buffer[PACKET_SIZE];
    int bytesReceived;
    int nodeID;
    int sourceID;
    int command;
    int payload;
    int payloadExtra;
};


class PacketHandler : public QObject
{
    Q_OBJECT

public:
    PacketHandler();
    void sendPacket(DataPacket *packet);
    void buildPacket(DataPacket *packet, char data);
    DataPacket *packetHandle();

signals:
    void sendSerial(const QByteArray &data);
    void gotKeyConfig(int idx, char key);
    void gotModifierConfig(int idx, char mod);
    void gotKeyReleaseConfig(int idx, char key);
    void gotModifierReleaseConfig(int idx, char mod);
    void gotDebugMessage(QString message);

private:
    void processReceivedPacket(DataPacket *packet);
    int arrToValue16(char *buffer, int offset);

    DataPacket serialPacket;

    bool gotSlashN;
    bool receivingString;
};

#endif // PACKETHANDLER_H
