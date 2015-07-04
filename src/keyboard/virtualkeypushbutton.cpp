/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//#include <QDebug>
#include <QPainter>

#include "virtualkeypushbutton.h"
#include <event.h>
#include <antkeymapper.h>
#include <eventhandlerfactory.h>

QHash<QString, QString> VirtualKeyPushButton::knownAliases = QHash<QString, QString> ();

VirtualKeyPushButton::VirtualKeyPushButton(JoyButton *button, QString xcodestring, QWidget *parent) :
    QPushButton(parent)
{
    populateKnownAliases();

    //qDebug() << "Question: " << X11KeySymToKeycode("KP_7") << endl;
    //qDebug() << "Question: " << X11KeySymToKeycode(79) << endl;
    this->keycode = 0;
    this->qkeyalias = 0;
    this->xcodestring = "";
    this->displayString = "";
    this->currentlyActive = false;
    this->onCurrentButton = false;
    this->button = button;

    int temp = 0;
    if (!xcodestring.isEmpty())
    {
        temp = X11KeySymToKeycode(xcodestring);
#ifdef Q_OS_UNIX
        BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();
        if (handler->getIdentifier() == "xtest")
        {
            temp = X11KeyCodeToX11KeySym(temp);
        }
#endif
    }

    if (temp > 0)
    {
#ifdef Q_OS_WIN
        //static QtWinKeyMapper nativeWinKeyMapper;
        BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();

  #ifdef WITH_VMULTI
        if (handler->getIdentifier() == "vmulti")
        {
            QtKeyMapperBase *nativeWinKeyMapper = AntKeyMapper::getInstance()->getNativeKeyMapper();
            this->qkeyalias = nativeWinKeyMapper->returnQtKey(temp);
            this->keycode = AntKeyMapper::getInstance()->returnVirtualKey(qkeyalias);
        }
  #endif
        BACKEND_ELSE_IF (handler->getIdentifier() == "sendinput")
        {
            this->keycode = temp;
            this->qkeyalias = AntKeyMapper::getInstance()->returnQtKey(this->keycode);
        }

        // Special exception for Numpad Enter on Windows.
        if (xcodestring == "KP_Enter")
        {
            this->qkeyalias = Qt::Key_Enter;
        }
#else
        this->keycode = temp;
        //this->keycode = X11KeyCodeToX11KeySym(temp);
        this->qkeyalias = AntKeyMapper::getInstance()->returnQtKey(this->keycode);
        //this->keycode = temp;
#endif
        this->xcodestring = xcodestring;
        this->displayString = setDisplayString(xcodestring);
    }

    this->setText(this->displayString.replace("&", "&&"));

    connect(this, SIGNAL(clicked()), this, SLOT(processSingleSelection()));
}

void VirtualKeyPushButton::processSingleSelection()
{
    emit keycodeObtained(keycode, qkeyalias);
}

QString VirtualKeyPushButton::setDisplayString(QString xcodestring)
{
    QString temp;
    if (knownAliases.contains(xcodestring))
    {
        temp = knownAliases.value(xcodestring);
    }
    else
    {
        temp = keycodeToKeyString(X11KeySymToKeycode(xcodestring));
        //temp = keycodeToKeyString(X11KeySymToKeycode(xcodestring));
    }

    if (temp.isEmpty() && !xcodestring.isEmpty())
    {
        temp = xcodestring;
    }

    return temp.toUpper();
}

// Define display strings that will be used for various keys on the
// virtual keyboard.
void VirtualKeyPushButton::populateKnownAliases()
{
    if (knownAliases.isEmpty())
    {
        knownAliases.insert("space", tr("Space"));
        knownAliases.insert("Tab", tr("Tab"));
        knownAliases.insert("Shift_L", tr("Shift (L)"));
        knownAliases.insert("Shift_R", tr("Shift (R)"));
        knownAliases.insert("Control_L", tr("Ctrl (L)"));
        knownAliases.insert("Control_R", tr("Ctrl (R)"));
        knownAliases.insert("Alt_L", tr("Alt (L)"));
        knownAliases.insert("Alt_R", tr("Alt (R)"));
        knownAliases.insert("Multi_key", tr("Alt (R)"));
        knownAliases.insert("grave", tr("`"));
        knownAliases.insert("asciitilde", tr("~"));
        knownAliases.insert("minus", tr("-"));
        knownAliases.insert("equal", tr("="));
        knownAliases.insert("bracketleft", tr("["));
        knownAliases.insert("bracketright", tr("]"));
        knownAliases.insert("backslash", tr("\\"));
        knownAliases.insert("Caps_Lock", tr("Caps"));
        knownAliases.insert("semicolon", tr(";"));
        knownAliases.insert("apostrophe", tr("'"));
        knownAliases.insert("comma", tr(","));
        knownAliases.insert("period", tr("."));
        knownAliases.insert("slash", tr("/"));
        knownAliases.insert("Escape", tr("ESC"));
        knownAliases.insert("Print", tr("PRTSC"));
        knownAliases.insert("Scroll_Lock", tr("SCLK"));
        knownAliases.insert("Insert", tr("INS"));
        knownAliases.insert("Prior", tr("PGUP"));
        knownAliases.insert("Delete", tr("DEL"));
        knownAliases.insert("Next", tr("PGDN"));
        knownAliases.insert("KP_1", tr("1"));
        knownAliases.insert("KP_2", tr("2"));
        knownAliases.insert("KP_3", tr("3"));
        knownAliases.insert("KP_4", tr("4"));
        knownAliases.insert("KP_5", tr("5"));
        knownAliases.insert("KP_6", tr("6"));
        knownAliases.insert("KP_7", tr("7"));
        knownAliases.insert("KP_8", tr("8"));
        knownAliases.insert("KP_9", tr("9"));
        knownAliases.insert("KP_0", tr("0"));
        knownAliases.insert("Num_Lock", tr("NUM\nLK"));
        knownAliases.insert("KP_Divide", tr("/"));
        knownAliases.insert("KP_Multiply", tr("*"));
        knownAliases.insert("KP_Subtract", tr("-"));
        knownAliases.insert("KP_Add", tr("+"));
        knownAliases.insert("KP_Enter", tr("E\nN\nT\nE\nR"));
        knownAliases.insert("KP_Decimal", tr("."));
        knownAliases.insert("asterisk", tr("*"));
        knownAliases.insert("less", tr("<"));
        knownAliases.insert("colon", tr(":"));
        knownAliases.insert("Super_L", tr("Super (L)"));
        knownAliases.insert("Menu", tr("Menu"));
        knownAliases.insert("Up", tr("Up"));
        knownAliases.insert("Down", tr("Down"));
        knownAliases.insert("Left", tr("Left"));
        knownAliases.insert("Right", tr("Right"));
    }
}

int VirtualKeyPushButton::calculateFontSize()
{
    QFont tempScaledFont(this->font());
    tempScaledFont.setPointSize(10);
    QFontMetrics fm(tempScaledFont);

    while (((this->width()-4) < fm.boundingRect(this->rect(), Qt::AlignCenter, this->text()).width()) && tempScaledFont.pointSize() >= 6)
    {
        tempScaledFont.setPointSize(tempScaledFont.pointSize()-1);
        fm = QFontMetrics(tempScaledFont);
    }

    return tempScaledFont.pointSize();
}
