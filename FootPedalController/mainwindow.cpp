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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"

#include "USBKeyboard.h"

#include <QMessageBox>
#include <QtSerialPort/QSerialPortInfo>

#include "packethandler.h"
#include "keydisplayform.h"

QT_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // create and add the key tab boxes
    pressButtonWidget = new KeyDisplayForm();
    releaseButtonWidget = new KeyDisplayForm();

    ui->tabWidget->addTab(pressButtonWidget, "When Button is Pressed");
    ui->tabWidget->addTab(releaseButtonWidget, "When Button is Released");

    // load the terminal
    console = new Console;
    console->setEnabled(false);

    ui->gridLayout->addWidget(console);

    // setup the serial port
    serial = new QSerialPort(this);

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
            ui->serialPortComboBox->addItem(info.portName());

    packetHandler = new PacketHandler();

    // Add commands
    ui->dropCmdType->addItem("Press and Release", CMD_BUTTON1);
    ui->dropCmdType->addItem("Press and Hold", CMD_BUTTON_ON_PRESS);
    ui->dropCmdType->addItem("Echo to USB serial", CMD_SERIAL_USB);

    // do QT signals and slots
    connect(ui->btnConnect, SIGNAL(clicked()), this, SLOT(openSerialPort()));
    connect(ui->edtText, SIGNAL(returnPressed()), this, SLOT(sendSerial()));
    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(console, SIGNAL(getData(QByteArray)), this, SLOT(writeData(QByteArray)));
    connect(ui->btnLoadKeys, SIGNAL(clicked()), this, SLOT(loadKeys()));
    connect(ui->btnSaveKeys, SIGNAL(clicked()), this, SLOT(saveKeys()));
    connect(packetHandler, SIGNAL(sendSerial(QByteArray)), this, SLOT(writeData(QByteArray)));
    connect(packetHandler, SIGNAL(gotDebugMessage(QString)), this, SLOT(incomingDebug(QString)));
    connect(packetHandler, SIGNAL(gotKeyConfig(int,char)), this, SLOT(gotKey(int,char)));
    connect(packetHandler, SIGNAL(gotModifierConfig(int,char)), this, SLOT(gotModifier(int,char)));
    connect(packetHandler, SIGNAL(gotKeyReleaseConfig(int,char)), this, SLOT(gotReleaseKey(int,char)));
    connect(packetHandler, SIGNAL(gotModifierReleaseConfig(int,char)), this, SLOT(gotReleaseModifier(int,char)));
}




MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::loadKeys()
{
    //tell the serial to get the configuration for the USB master device
    DataPacket p;
    p.nodeID = 1;
    p.sourceID = 0;
    p.command = CMD_CONFIGURE_GET_KEYS_PRESS;
    p.payload = 0;
    p.payloadExtra = 0;
    packetHandler->sendPacket(&p);

    // ask for the modifiers
    p.command = CMD_CONFIGURE_GET_MODIFIERS_PRESS;
    packetHandler->sendPacket(&p);

    // now the same but on release
    p.command = CMD_CONFIGURE_GET_KEYS_RELEASE;
    packetHandler->sendPacket(&p);

    // now the same but on release
    p.command = CMD_CONFIGURE_GET_MODIFIERS_RELEASE;
    packetHandler->sendPacket(&p);
}

void MainWindow::saveKeys()
{
    sendKeys(pressButtonWidget->getKeys(), CMD_CONFIGURE_BUT_KEY_PRESS);
    sendKeys(pressButtonWidget->getModifiers(), CMD_CONFIGURE_BUT_MODIFIER_PRESS);
    sendKeys(releaseButtonWidget->getKeys(), CMD_CONFIGURE_BUT_KEY_RELEASE);
    sendKeys(releaseButtonWidget->getModifiers(), CMD_CONFIGURE_BUT_MODIFIER_RELEASE);

    // send eeprom save
    DataPacket p;
    p.nodeID = 1;
    p.sourceID = 0;
    p.command = CMD_CONFIGURE_SAVE;
    p.payload = 0;
    p.payloadExtra = 0;
    packetHandler->sendPacket(&p);
}

void MainWindow::sendKeys(QMap<int, char> keys, int command)
{
    // send each key to the board
    foreach( int k, keys.keys())
    {
        DataPacket p;
        p.nodeID = 1;
        p.sourceID = 0;
        p.command = command;
        p.payload = k;
        p.payloadExtra = (char)keys.value(k);
        packetHandler->sendPacket(&p);
    }
}

void MainWindow::gotKey(int idx, char key)
{
    pressButtonWidget->addKey(idx, key);
}

void MainWindow::gotReleaseKey(int idx, char key)
{
    releaseButtonWidget->addKey(idx, key);
}

void MainWindow::gotModifier(int idx, char mod)
{
    qDebug() << "Got a MOD! idx" << idx << " mod " << mod;

    pressButtonWidget->addModifier(idx, mod);
}

void MainWindow::gotReleaseModifier(int idx, char mod)
{
    qDebug() << "Got a MOD! idx" << idx << " mod " << mod;

    releaseButtonWidget->addModifier(idx, mod);
}


void MainWindow::incomingDebug(QString message)
{
    qDebug() << "Got a Debug!";
    console->putData(message.toLocal8Bit());
}


void MainWindow::sendSerial()
{
    writeData(ui->edtText->text().toLocal8Bit());
}

void MainWindow::openSerialPort()
{
    serial->setPortName(ui->serialPortComboBox->currentText());
    if (serial->open(QIODevice::ReadWrite)) {
        if (serial->setBaudRate(QSerialPort::Baud9600)
                && serial->setDataBits(QSerialPort::Data8)
                && serial->setParity(QSerialPort::NoParity)
                && serial->setStopBits(QSerialPort::OneStop)
                && serial->setFlowControl(QSerialPort::NoFlowControl)) {
            console->setEnabled(true);
            console->setLocalEchoEnabled(true);
            ui->statusBar->showMessage(tr("Connected"));

        } else {
            serial->close();
            QMessageBox::critical(this, tr("Error"), serial->errorString());
        }
    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

    }
}

void MainWindow::closeSerialPort()
{
    serial->close();
    console->setEnabled(false);

    ui->statusBar->showMessage(tr("Disconnected"));
}

void MainWindow::writeData(const QByteArray &data)
{
    serial->write(data);
}

void MainWindow::readData()
{
    QByteArray data = serial->readAll();

    char *cdata = data.data();

    for (int i = 0; i < data.length(); i++)
    {
        packetHandler->buildPacket(packetHandler->packetHandle(), cdata[i]);
    }
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::setControlsEnabled(bool enable)
{
    //runButton->setEnabled(enable);
    //serialPortComboBox->setEnabled(enable);
    //waitResponseSpinBox->setEnabled(enable);
    //requestLineEdit->setEnabled(enable);
}
