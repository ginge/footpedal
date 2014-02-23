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
#include "packethandler.h"

#include <QDebug>

DataPacket::DataPacket()
{
    memset(buffer, 0, sizeof(char) * PACKET_SIZE);
    bytesReceived = 0;
    nodeID = 0;
    sourceID = 0;
    command = 0;
    payload = 0;
    payloadExtra = 0;
}

PacketHandler::PacketHandler()
{
    gotSlashN = false;
    receivingString = false;
}

DataPacket *PacketHandler::packetHandle()
{
    return &serialPacket;
}

QString debugMessageStr;

void PacketHandler::buildPacket(DataPacket *packet, char data) {
    if (receivingString)
    {
        debugMessageStr.append(data);

        if (data == '\r')
        {
            gotSlashN = true;
        }

        if (data == '\n' && gotSlashN)
        {
            emit gotDebugMessage(debugMessageStr);
            debugMessageStr = "";
            receivingString = false;
        }
        return;
    }
    packet->buffer[packet->bytesReceived] = (unsigned char)data;

    packet->bytesReceived++;

    if (packet->bytesReceived == PACKET_SIZE)
    {
        // we got the packet
        packet->nodeID = packet->buffer[ID_LOC];
        packet->sourceID = packet->buffer[SRC_LOC];
        packet->command = packet->buffer[CMD_LOC];

        // now do stuff with the packet
        packet->payload = arrToValue16(packet->buffer, DATA_LOC_0);
        packet->payloadExtra = arrToValue16(packet->buffer, DATA_LOC_A);

        processReceivedPacket(packet);
        packet->bytesReceived = 0;
    }
}


void PacketHandler::processReceivedPacket(DataPacket *packet)
{
    switch (packet->command)
    {
    case CMD_CONFIGURE_GET_KEYS_PRESS:
        // device is returning some key information
        emit gotKeyConfig(packet->payload, packet->payloadExtra);
        break;
    case CMD_CONFIGURE_GET_MODIFIERS_PRESS:
        // device is returning some mod information
        emit gotModifierConfig(packet->payload, packet->payloadExtra);
        break;
    case CMD_CONFIGURE_GET_KEYS_RELEASE:
        // device is returning some key information
        emit gotKeyReleaseConfig(packet->payload, packet->payloadExtra);
        break;
    case CMD_CONFIGURE_GET_MODIFIERS_RELEASE:
        // device is returning some mod information
        emit gotModifierReleaseConfig(packet->payload, packet->payloadExtra);
        break;
    case CMD_CONFIGURE_DEBUG_MSG: // we are getting an incoming string message after this. we then wait for \r\n
        receivingString = true;
        break;
    case CMD_GET_ID:
        emit gotDeviceID(packet->payload);
        break;
    case CMD_GET_BUT_CMD:
        emit gotButtonMode(packet->payload);
        break;
    default:
        break;
    }
}



void PacketHandler::sendPacket(DataPacket *packet)
{
    char tpacket[7];

    tpacket[0] = packet->nodeID;
    tpacket[1] = packet->sourceID;
    tpacket[2] = packet->command;
    tpacket[3] = (packet->payload >> 8) & 0xFF;
    tpacket[4] = packet->payload & 0xFF;
    tpacket[5] = (packet->payloadExtra >> 8) & 0xFF;
    tpacket[6] = packet->payloadExtra & 0xFF;

    emit sendSerial(QByteArray(tpacket, sizeof(tpacket)));
}

DataPacket *PacketHandler::sendPacket(int toNode, int fromNode, int command, int payload, int payloadExtra)
{
    DataPacket *p = new DataPacket();

    p->nodeID = toNode;
    p->sourceID = fromNode;
    p->command = command;
    p->payload = payload;
    p->payloadExtra = payloadExtra;

    sendPacket(p);

    return p;
}



u_int16_t PacketHandler::arrToValue16(unsigned char *buffer, int offset) {
  u_int16_t val = 0;
  val |= buffer[offset];
  val = val << 8;
  val |= buffer[offset+1];

  return val;
}


