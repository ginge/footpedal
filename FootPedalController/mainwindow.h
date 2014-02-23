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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QtGlobal>
#include <QMainWindow>
#include <QtSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QListWidgetItem>
#include "packethandler.h"

namespace Ui {
class MainWindow;
}

class Console;
class KeyDisplayForm;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

private slots:
    void openSerialPort();
    void closeSerialPort();
    void writeData(const QByteArray &data);
    void readData();
    void sendSerial();
    void handleError(QSerialPort::SerialPortError error);
    void loadKeys();
    void saveKeys();
    void incomingDebug(QString message);
    void commandModeChanged(int index);
    void deviceSelected(QString device);
    void scanDevices();
    void devicesScanned();

    // from the parser module
    void gotKey(int idx, char key);
    void gotModifier(int idx, char mod);
    void gotReleaseKey(int idx, char key);
    void gotReleaseModifier(int idx, char mod);
    void gotDeviceID(unsigned int id);
    void gotButtonMode(unsigned int id);

private:
    Ui::MainWindow *ui;
    Console *console;
    QSerialPort *serial;
    void setControlsEnabled(bool enable);
    void sendPacket(char destNode, char srcNode, char command, int data0, int data1);
    void sendKeys(QMap<int, char> keys, int command);
    QListWidgetItem* getCurrentItem();
    void scanDevices(bool allChains);

    PacketHandler *packetHandler;

    KeyDisplayForm *pressButtonWidget;
    KeyDisplayForm *releaseButtonWidget;


    bool secondPass;
};

#endif // MAINWINDOW_H
