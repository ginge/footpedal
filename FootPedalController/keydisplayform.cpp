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
#include "keydisplayform.h"
#include "ui_keydisplayform.h"

#include <QDebug>
#include "USBKeyboard.h"

KeyDisplayForm::KeyDisplayForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KeyDisplayForm)
{
    ui->setupUi(this);


    ui->keysWidget->setColumnCount(11);

    QStringList labels;
    labels << tr("No") << tr("Key") << tr("Code") << tr("Ctrl") << tr("Alt") << tr("Shift") << tr("Win") << tr("CrtlR") << tr("AltR") << tr("ShiftR") << tr("WinR");
    ui->keysWidget->setHorizontalHeaderLabels(labels);
    //ui->keysWidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->keysWidget->verticalHeader()->hide();
    ui->keysWidget->setShowGrid(false);
    ui->keysWidget->setColumnWidth(0, 50);
    ui->keysWidget->setColumnWidth(1, 80);
    ui->keysWidget->setColumnWidth(2, 50);
    for (int i = 3; i <= 10; i++)
    {
        ui->keysWidget->setColumnWidth(i, 45);
    }

    connect(ui->btnAdd, SIGNAL(clicked()), this, SLOT(addToTable()));
    connect(ui->btnUpdate, SIGNAL(clicked()), this, SLOT(updateItem()));
    connect(ui->keysWidget, SIGNAL(cellClicked(int,int)), this, SLOT(cellClicked(int,int)));

    dropKeysInit();

    updatingIndex = -1;
}

KeyDisplayForm::~KeyDisplayForm()
{
    delete ui;
}

QMap<int, char> KeyDisplayForm::getKeys()
{
    QMap<int, char> map;

    int rowCount = ui->keysWidget->rowCount();

    for (int n = 0; n < rowCount; n++)
    {
        map[n] = ui->keysWidget->item(n, 2)->data(Qt::DisplayRole).toUInt();
    }

    return map;
}

QMap<int, char> KeyDisplayForm::getModifiers()
{
    QMap<int, char> map;

    int rowCount = ui->keysWidget->rowCount();

    for (int n = 0; n < rowCount; n++)
    {
        int ctrl = (ui->keysWidget->item(n, 3)->checkState() == Qt::Checked ? KEY_CTRL : 0);
        int alt = (ui->keysWidget->item(n, 4)->checkState() == Qt::Checked ? KEY_ALT : 0);
        int shift = (ui->keysWidget->item(n, 5)->checkState() == Qt::Checked ? KEY_SHIFT : 0);
        int win = (ui->keysWidget->item(n, 6)->checkState() == Qt::Checked ? KEY_GUI : 0);
        int ctrlR = (ui->keysWidget->item(n, 7)->checkState() == Qt::Checked ? KEY_RIGHT_CTRL : 0);
        int altR = (ui->keysWidget->item(n, 8)->checkState() == Qt::Checked ? KEY_RIGHT_ALT : 0);
        int shiftR = (ui->keysWidget->item(n, 9)->checkState() == Qt::Checked ? KEY_RIGHT_SHIFT : 0);
        int winR = (ui->keysWidget->item(n, 10)->checkState() == Qt::Checked ? KEY_RIGHT_GUI : 0);

        map[n] =  ctrl | alt | shift | win | ctrlR | altR | shiftR | winR;
    }

    return map;
}

void KeyDisplayForm::addKey(int idx, char key)
{
    QString skey;
    qDebug() << "Got a KEY! idx" << idx << " key " << key;

    // get the nice name for this key from the combobox (ugh)
    for (int i = 0; i < ui->dropKeys->count(); ++i) {
        if (ui->dropKeys->itemData(i) == (int)key) {
            skey = ui->dropKeys->itemText(i);
            break;
        }
    }
    updateOrInsertTableRow(idx, skey, (int)key, false,false, false, false, false, false, false, false);
}

void KeyDisplayForm::addModifier(int idx, char mod)
{
    updateOrInsertTableRow(idx, NULL, -1, isBitSet(mod, KEY_CTRL), isBitSet(mod, KEY_ALT), isBitSet(mod, KEY_SHIFT),
                                          isBitSet(mod, KEY_GUI), isBitSet(mod, KEY_RIGHT_CTRL), isBitSet(mod, KEY_RIGHT_ALT),
                                          isBitSet(mod, KEY_RIGHT_SHIFT), isBitSet(mod, KEY_RIGHT_GUI));
}


void KeyDisplayForm::cellClicked(int row, int col)
{
    col = col;

    updatingIndex = row;

    // get the key code
    int code = ui->keysWidget->item(row, 2)->data(Qt::DisplayRole).toInt();

    // get the nice name for this key from the combobox (ugh)
    for (int i = 0; i < ui->dropKeys->count(); ++i) {
        if (ui->dropKeys->itemData(i) == code) {
            ui->dropKeys->setCurrentIndex(i);
            break;
        }
    }

    ui->chkCtrl->setCheckState(ui->keysWidget->item(row, 3)->checkState());
    ui->chkAlt->setCheckState(ui->keysWidget->item(row, 4)->checkState());
    ui->chkShift->setCheckState(ui->keysWidget->item(row, 5)->checkState());
    ui->chkWin->setCheckState(ui->keysWidget->item(row, 6)->checkState());
    ui->chkCtrlR->setCheckState(ui->keysWidget->item(row, 7)->checkState());
    ui->chkAltR->setCheckState(ui->keysWidget->item(row, 8)->checkState());
    ui->chkShiftR->setCheckState(ui->keysWidget->item(row, 9)->checkState());
    ui->chkWinR->setCheckState(ui->keysWidget->item(row, 10)->checkState());
}

void KeyDisplayForm::addToTable()
{
    addTableRow(ui->dropKeys->currentText(), ui->dropKeys->itemData(ui->dropKeys->currentIndex()).toInt(), ui->chkCtrl->isChecked(), ui->chkAlt->isChecked(), ui->chkShift->isChecked(),
                ui->chkWin->isChecked(), ui->chkCtrlR->isChecked(), ui->chkAltR->isChecked(), ui->chkShiftR->isChecked(), ui->chkWinR->isChecked());
}

void KeyDisplayForm::updateItem()
{
    if (updatingIndex == -1)
        return;

    updateOrInsertTableRow(updatingIndex, ui->dropKeys->currentText(), ui->dropKeys->itemData(ui->dropKeys->currentIndex()).toInt(), ui->chkCtrl->isChecked(), ui->chkAlt->isChecked(), ui->chkShift->isChecked(),
                           ui->chkWin->isChecked(), ui->chkCtrlR->isChecked(), ui->chkAltR->isChecked(), ui->chkShiftR->isChecked(), ui->chkWinR->isChecked());
    updatingIndex = -1;
}

void KeyDisplayForm::addTableRow(QString text, int code, bool ctrl, bool alt, bool shift, bool win, bool ctrlR, bool altR, bool shiftR, bool winR)
{
    int row = ui->keysWidget->rowCount();
    ui->keysWidget->insertRow(row);
    ui->keysWidget->setItem(row, 0, new QTableWidgetItem(QString::number(row)));
    ui->keysWidget->setItem(row, 1, new QTableWidgetItem(text));
    ui->keysWidget->setItem(row, 2, new QTableWidgetItem(QString::number(code)));
    addCheck(row, 3, ctrl);
    addCheck(row, 4, alt);
    addCheck(row, 5, shift);
    addCheck(row, 6, win);
    addCheck(row, 7, ctrlR);
    addCheck(row, 8, altR);
    addCheck(row, 9, shiftR);
    addCheck(row, 10, winR);
}

void KeyDisplayForm::updateOrInsertTableRow(int idx, QString text, int code, bool ctrl, bool alt, bool shift, bool win, bool ctrlR, bool altR, bool shiftR, bool winR)
{
    int row = ui->keysWidget->rowCount();

    // check if the key exists

    // do an insert
    if (idx >= row)
    {
        addTableRow(text, code, ctrl, alt, shift, win, ctrlR, altR, shiftR, winR);
        return;
    }

    // update the existing one
    ui->keysWidget->setItem(idx, 0, new QTableWidgetItem(QString::number(idx)));
    if (text != NULL)   // special case where we know only partial data, so leave existing
    {
        ui->keysWidget->setItem(idx, 1, new QTableWidgetItem(text));
        ui->keysWidget->setItem(idx, 2, new QTableWidgetItem(QString::number(code)));
    }
    ui->keysWidget->item(idx, 3)->setCheckState(ctrl ? Qt::Checked : Qt::Unchecked);
    ui->keysWidget->item(idx, 4)->setCheckState(alt ? Qt::Checked : Qt::Unchecked);
    ui->keysWidget->item(idx, 5)->setCheckState(shift ? Qt::Checked : Qt::Unchecked);
    ui->keysWidget->item(idx, 6)->setCheckState(win ? Qt::Checked : Qt::Unchecked);
    ui->keysWidget->item(idx, 7)->setCheckState(ctrlR ? Qt::Checked : Qt::Unchecked);
    ui->keysWidget->item(idx, 8)->setCheckState(altR ? Qt::Checked : Qt::Unchecked);
    ui->keysWidget->item(idx, 9)->setCheckState(shiftR ? Qt::Checked : Qt::Unchecked);
    ui->keysWidget->item(idx, 10)->setCheckState(winR ? Qt::Checked : Qt::Unchecked);
}

void KeyDisplayForm::addCheck(int row, int col, bool val)
{
    QTableWidgetItem *item = new QTableWidgetItem(tr(""));
    item->setCheckState(val ? Qt::Checked : Qt::Unchecked);
    ui->keysWidget->setItem(row, col, item);
}


void KeyDisplayForm::dropKeysInit(void)
{
    for (int i = 97; i <= 122; i++)
    {
        ui->dropKeys->addItem(QString((char)i), KEY_A+i-97);
    }

    for (int i = 49; i <= 57; i++)
    {
        ui->dropKeys->addItem(QString((char)i), KEY_1+i-49);
    }
    ui->dropKeys->addItem("0", KEY_0);

    ui->dropKeys->addItem("Enter", KEY_ENTER);
    ui->dropKeys->addItem("Esc", KEY_ESC);
    ui->dropKeys->addItem("Backspace", KEY_BACKSPACE);
    ui->dropKeys->addItem("Tab", KEY_TAB);
    ui->dropKeys->addItem("Space", KEY_SPACE);
    ui->dropKeys->addItem("-", KEY_MINUS);
    ui->dropKeys->addItem("=", KEY_EQUAL);
    ui->dropKeys->addItem("(", KEY_LEFT_BRACE);
    ui->dropKeys->addItem(")", KEY_RIGHT_BRACE);
    ui->dropKeys->addItem("\\", KEY_BACKSLASH);
    ui->dropKeys->addItem("Num Lock", KEY_NUMBER);
    ui->dropKeys->addItem(";", KEY_SEMICOLON);
    ui->dropKeys->addItem("\"", KEY_QUOTE);
    ui->dropKeys->addItem("~", KEY_TILDE);
    ui->dropKeys->addItem(",", KEY_COMMA);
    ui->dropKeys->addItem(".", KEY_PERIOD);
    ui->dropKeys->addItem("/", KEY_SLASH);
    ui->dropKeys->addItem("Caps", KEY_CAPS_LOCK);

    for (int i = 58; i <= 69; i++)
    {
        ui->dropKeys->addItem(QString("F%1").arg(i-57), KEY_F1 + i - 58);
    }

    ui->dropKeys->addItem("PrtScr", KEY_PRINTSCREEN);
    ui->dropKeys->addItem("Scroll Lock", KEY_SCROLL_LOCK);
    ui->dropKeys->addItem("Pause", KEY_PAUSE);
    ui->dropKeys->addItem("Insert", KEY_INSERT);
    ui->dropKeys->addItem("Home", KEY_HOME);
    ui->dropKeys->addItem("Pg Up", KEY_PAGE_UP);
    ui->dropKeys->addItem("Delete", KEY_DELETE);
    ui->dropKeys->addItem("End", KEY_END);
    ui->dropKeys->addItem("Pg Dn", KEY_PAGE_DOWN);
    ui->dropKeys->addItem("Right Arrow", KEY_RIGHT);
    ui->dropKeys->addItem("Left Arrow", KEY_LEFT);
    ui->dropKeys->addItem("Down Arrow", KEY_DOWN);
    ui->dropKeys->addItem("Up Arrow", KEY_UP);
    ui->dropKeys->addItem("Num Lock", KEY_NUM_LOCK);
    ui->dropKeys->addItem("Keypad /", KEYPAD_SLASH);
    ui->dropKeys->addItem("Keypad *", KEYPAD_ASTERIX);
    ui->dropKeys->addItem("Keypad -", KEYPAD_MINUS);
    ui->dropKeys->addItem("Keypad +", KEYPAD_PLUS);
    ui->dropKeys->addItem("Keypad Enter", KEYPAD_ENTER);

    for (int i = 89; i <= 97; i++)
    {
      ui->dropKeys->addItem(QString("Keypad %1").arg(i-88), KEY_F1 + i - 89);
    }
    ui->dropKeys->addItem("Keypad 0", KEYPAD_0);

    ui->dropKeys->addItem("Keypad .", KEYPAD_PERIOD);
}

bool KeyDisplayForm::isBitSet(char val, int bit)
{
    return (val & bit) != 0;
}

