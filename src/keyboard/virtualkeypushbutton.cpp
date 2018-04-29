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

#include "virtualkeypushbutton.h"

#include "messagehandler.h"
#include "joybutton.h"
#include "event.h"
#include "antkeymapper.h"
#include "eventhandlerfactory.h"

#include <QDebug>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>


QHash<QString, QString> VirtualKeyPushButton::knownAliases = QHash<QString, QString> ();

VirtualKeyPushButton::VirtualKeyPushButton(QString xcodestring, QWidget *parent) :
    QPushButton(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    populateKnownAliases();

    //qDebug() << "Question: " << X11KeySymToKeycode("KP_7") << endl;
    //qDebug() << "Question: " << X11KeySymToKeycode(79) << endl;
    this->keycode = 0;
    this->qkeyalias = 0;
    this->xcodestring = "";
    this->displayString = "";
    this->currentlyActive = false;
    this->onCurrentButton = false;

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
#elif defined(Q_OS_UNIX)
        this->keycode = temp;
        //this->keycode = X11KeyCodeToX11KeySym(temp);
        this->qkeyalias = AntKeyMapper::getInstance()->returnQtKey(this->keycode);

        //this->keycode = temp;
#endif
        this->xcodestring = xcodestring;
        this->displayString = setDisplayString(xcodestring);

    }

    #ifndef QT_DEBUG_NO_OUTPUT
    qDebug() << "qkeyalias after returnQtKey: " << this->qkeyalias;
    qDebug() << "keycode: " << this->keycode;
    qDebug() << "xcodestring: " << this->xcodestring;
    qDebug() << "displayString: " << this->displayString;
    #endif

    this->setText(this->displayString.replace("&", "&&"));

    connect(this, SIGNAL(clicked()), this, SLOT(processSingleSelection()));
}

void VirtualKeyPushButton::processSingleSelection()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    emit keycodeObtained(keycode, qkeyalias);
}

QString VirtualKeyPushButton::setDisplayString(QString xcodestring)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();
    if (knownAliases.contains(xcodestring))
    {
        temp = knownAliases.value(xcodestring);
    }
    else
    {
        temp = keycodeToKeyString(X11KeySymToKeycode(xcodestring));
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (knownAliases.isEmpty())
    {
        knownAliases.insert("space", trUtf8("Space"));
        knownAliases.insert("Tab", trUtf8("Tab"));
        knownAliases.insert("Shift_L", trUtf8("Shift (L)"));
        knownAliases.insert("Shift_R", trUtf8("Shift (R)"));
        knownAliases.insert("Control_L", trUtf8("Ctrl (L)"));
        knownAliases.insert("Control_R", trUtf8("Ctrl (R)"));
        knownAliases.insert("Alt_L", trUtf8("Alt (L)"));
        knownAliases.insert("Alt_R", trUtf8("Alt (R)"));
        knownAliases.insert("ISO_Level3_Shift", trUtf8("Alt Gr"));
        knownAliases.insert("Multi_key", trUtf8("Alt (R)"));
        knownAliases.insert("grave", trUtf8("`"));
        knownAliases.insert("asciitilde", trUtf8("~"));
        knownAliases.insert("minus", trUtf8("-"));
        knownAliases.insert("equal", trUtf8("="));
        knownAliases.insert("bracketleft", trUtf8("["));
        knownAliases.insert("bracketright", trUtf8("]"));
        knownAliases.insert("backslash", trUtf8("\\"));
        knownAliases.insert("Caps_Lock", trUtf8("Caps"));
        knownAliases.insert("semicolon", trUtf8(";"));
        knownAliases.insert("apostrophe", trUtf8("'"));
        knownAliases.insert("comma", trUtf8(","));
        knownAliases.insert("period", trUtf8("."));
        knownAliases.insert("slash", trUtf8("/"));
        knownAliases.insert("Escape", trUtf8("ESC"));
        knownAliases.insert("Print", trUtf8("PRTSC"));
        knownAliases.insert("Scroll_Lock", trUtf8("SCLK"));
        knownAliases.insert("Insert", trUtf8("INS"));
        knownAliases.insert("Prior", trUtf8("PGUP"));
        knownAliases.insert("Delete", trUtf8("DEL"));
        knownAliases.insert("Next", trUtf8("PGDN"));
        knownAliases.insert("KP_1", trUtf8("1"));
        knownAliases.insert("KP_2", trUtf8("2"));
        knownAliases.insert("KP_3", trUtf8("3"));
        knownAliases.insert("KP_4", trUtf8("4"));
        knownAliases.insert("KP_5", trUtf8("5"));
        knownAliases.insert("KP_6", trUtf8("6"));
        knownAliases.insert("KP_7", trUtf8("7"));
        knownAliases.insert("KP_8", trUtf8("8"));
        knownAliases.insert("KP_9", trUtf8("9"));
        knownAliases.insert("KP_0", trUtf8("0"));
        knownAliases.insert("Num_Lock", trUtf8("NUM\nLK"));
        knownAliases.insert("KP_Divide", trUtf8("/"));
        knownAliases.insert("KP_Multiply", trUtf8("*"));
        knownAliases.insert("KP_Subtract", trUtf8("-"));
        knownAliases.insert("KP_Add", trUtf8("+"));
        knownAliases.insert("KP_Enter", trUtf8("Enter"));
        knownAliases.insert("KP_Decimal", trUtf8("."));
        knownAliases.insert("asterisk", trUtf8("*"));
        knownAliases.insert("less", trUtf8("<"));
        knownAliases.insert("colon", trUtf8(":"));
        knownAliases.insert("Super_L", trUtf8("Super (L)"));
        knownAliases.insert("Menu", trUtf8("Menu"));
        knownAliases.insert("Up", trUtf8("Up"));
        knownAliases.insert("Down", trUtf8("Down"));
        knownAliases.insert("Left", trUtf8("Left"));
        knownAliases.insert("Right", trUtf8("Right"));
    }
}

int VirtualKeyPushButton::calculateFontSize()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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

int VirtualKeyPushButton::getKeycode() const {

    return keycode;
}

int VirtualKeyPushButton::getQkeyalias() const {

    return qkeyalias;
}

QString VirtualKeyPushButton::getXcodestring() const {

    return xcodestring;
}

QString VirtualKeyPushButton::getDisplayString() const {

    return displayString;
}

bool VirtualKeyPushButton::getCurrentlyActive() const {

    return currentlyActive;
}

bool VirtualKeyPushButton::getOnCurrentButton() const {

    return onCurrentButton;
}
