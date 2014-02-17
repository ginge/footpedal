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
#ifndef KEYDISPLAYFORM_H
#define KEYDISPLAYFORM_H

#include <QWidget>

namespace Ui {
class KeyDisplayForm;
}

class KeyDisplayForm : public QWidget
{
    Q_OBJECT
    
public:
    explicit KeyDisplayForm(QWidget *parent = 0);
    ~KeyDisplayForm();
    void addKey(int idx, char key);
    bool isBitSet(char val, int bit);
    void addModifier(int idx, char mod);
    QMap<int, char> getKeys();
    QMap<int, char> getModifiers();

signals:
    void loadClicked();

private slots:
    void addToTable();
    void updateItem();
    void cellClicked(int row, int col);

private:
    Ui::KeyDisplayForm *ui;
    void dropKeysInit(void);
    void addCheck(int row, int col, bool val);

    void addTableRow(QString text, int code, bool ctrl, bool alt, bool shift, bool win, bool ctrlR, bool altR, bool shiftR, bool WinR);
    void updateOrInsertTableRow(int idx, QString text, int code, bool ctrl, bool alt, bool shift, bool win, bool ctrlR, bool altR, bool shiftR, bool winR);

    int updatingIndex;
};

#endif // KEYDISPLAYFORM_H
