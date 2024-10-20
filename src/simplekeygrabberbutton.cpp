/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
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

#include "simplekeygrabberbutton.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QObject>

#include "antkeymapper.h"
#include "event.h"
#include "eventhandlerfactory.h"

#include <QApplication>
#include <QDebug>
#include <QMetaType>
#include <QWidget>

#ifdef Q_OS_WIN
    #include "winextras.h"
#endif

SimpleKeyGrabberButton::SimpleKeyGrabberButton(QWidget *parent)
    : QPushButton(parent)
{
    grabNextAction = false;
    grabbingWheel = false;
    edited = false;
    this->installEventFilter(this);
}

void SimpleKeyGrabberButton::keyPressEvent(QKeyEvent *event)
{ // Do not allow closing of dialog using Escape key
    if (event->key() == Qt::Key_Escape)
        return;

    QPushButton::keyPressEvent(event);
}

bool SimpleKeyGrabberButton::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);

    int controlcode = 0;

    if (grabNextAction && (event->type() == QEvent::MouseButtonRelease))
    {
        QMouseEvent *mouseEve = static_cast<QMouseEvent *>(event);

        if (mouseEve->button() == Qt::RightButton)
            controlcode = 3;
        else if (mouseEve->button() == Qt::MiddleButton)
            controlcode = 2;
        else
            controlcode = mouseEve->button();

        buttonslot.setSlotCode(controlcode);
        buttonslot.setSlotMode(JoyButtonSlot::JoyMouseButton);
        refreshButtonLabel();
        edited = true;
        releaseMouse();
        releaseKeyboard();

        grabNextAction = grabbingWheel = false;
        emit buttonCodeChanged(controlcode);
    } else if (grabNextAction && (event->type() == static_cast<QEvent::Type>(7)))
    {
        QKeyEvent *keyEve = static_cast<QKeyEvent *>(event);
        int tempcode = keyEve->nativeScanCode();
        int virtualactual = keyEve->nativeVirtualKey();

        BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();

        int finalvirtual = 0;
        int checkalias = 0;

#ifdef Q_OS_WIN
        BACKEND_ELSE_IF(handler->getIdentifier() == "sendinput")
        {
            // Find more specific virtual key (VK_SHIFT -> VK_LSHIFT)
            // by checking for extended bit in scan code.
            finalvirtual = WinExtras::correctVirtualKey(tempcode, virtualactual);
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual, tempcode);
        }
#elif defined(WITH_X11)

        finalvirtual = X11KeyCodeToX11KeySym(tempcode); // Obtain group 1 X11 keysym. Removes effects from modifiers.

    #ifdef WITH_UINPUT
        if (handler->getIdentifier() == "uinput")
        {
            QtKeyMapperBase *x11KeyMapper =
                AntKeyMapper::getInstance()->getNativeKeyMapper(); // Find Qt Key corresponding to X11 KeySym.
            checkalias = x11KeyMapper->returnQtKey(finalvirtual);
            finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(
                checkalias); // Find corresponding Linux input key for the Qt key.
        } else
    #endif

    #ifdef WITH_XTEST
            if (handler->getIdentifier() == "xtest")
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual); // Check for alias against group 1 keysym.
        else
    #endif
        { // Not using any known handler.
            finalvirtual = tempcode;
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
        }
#else
        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
            finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(keyEve->key());
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
        } else
        {
            // Not running on xcb platform.
            finalvirtual = tempcode;
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
        }

#endif

        controlcode = tempcode;
        bool valueUpdated = false;

        qDebug() << "slot string for simple key grab button is: " << buttonslot.getSlotString();

        if ((keyEve->modifiers() & Qt::ControlModifier) && (keyEve->key() == Qt::Key_X))
        {
            controlcode = 0;
            refreshButtonLabel();
        } else if (controlcode <= 0)
        {
            controlcode = 0;
            setText("");
            valueUpdated = true;
            edited = true;
        } else if ((checkalias > 0) && (finalvirtual > 0))
        {
            buttonslot.setSlotCode(finalvirtual, checkalias);
            buttonslot.setSlotMode(JoyButtonSlot::JoyKeyboard);
            setText(keysymToKeyString(finalvirtual, checkalias).toUpper());

            edited = true;
            valueUpdated = true;
        } else if (virtualactual > 0)
        {
            buttonslot.setSlotCode(virtualactual);
            buttonslot.setSlotMode(JoyButtonSlot::JoyKeyboard);
            setText(keysymToKeyString(virtualactual).toUpper());

            edited = true;
            valueUpdated = true;
        }

        grabNextAction = false;
        grabbingWheel = false;
        releaseMouse();
        releaseKeyboard();

        if (valueUpdated)
            emit buttonCodeChanged(controlcode);
    } else if (grabNextAction && (event->type() == QEvent::Wheel) && !grabbingWheel)
    {
        grabbingWheel = true;
    } else if (grabNextAction && (event->type() == QEvent::Wheel))
    {
        QWheelEvent *wheelEve = static_cast<QWheelEvent *>(event);
        QString text = QString(tr("Mouse")).append(" ");

        if ((wheelEve->angleDelta().y() >= 120))
        {
            controlcode = 4;
        } else if ((wheelEve->angleDelta().y() <= -120))
        {
            controlcode = 5;
        } else if ((wheelEve->angleDelta().x() >= 120))
        {
            controlcode = 6;
        } else if ((wheelEve->angleDelta().x() <= -120))
        {
            controlcode = 7;
        }

        if (controlcode > 0)
        {
            text = text.append(QString::number(controlcode));
            setText(text);

            grabNextAction = false;
            grabbingWheel = false;
            edited = true;
            releaseMouse();
            releaseKeyboard();
            buttonslot.setSlotCode(controlcode);
            buttonslot.setSlotMode(JoyButtonSlot::JoyMouseButton);
            emit buttonCodeChanged(controlcode);
        }
    } else if (event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *mouseEve = static_cast<QMouseEvent *>(event);

        if (mouseEve->button() == Qt::LeftButton)
        {
            grabNextAction = true;
            setText("...");
            setFocus();
            grabKeyboard();
            grabMouse();
        }
    }

    return false;
}

void SimpleKeyGrabberButton::setValue(int value, int alias, JoyButtonSlot::JoySlotInputAction mode)
{
    buttonslot.setSlotCode(value, alias);
    buttonslot.setSlotMode(mode);
    edited = true;

    setText(buttonslot.getSlotString());
}

void SimpleKeyGrabberButton::setValue(int value, JoyButtonSlot::JoySlotInputAction mode)
{
    buttonslot.setSlotCode(value);
    buttonslot.setSlotMode(mode);
    edited = true;

    setText(buttonslot.getSlotString());
}

void SimpleKeyGrabberButton::setValue(QString value, JoyButtonSlot::JoySlotInputAction mode)
{
    switch (mode)
    {
    case JoyButtonSlot::JoyLoadProfile:
    case JoyButtonSlot::JoyTextEntry:
    case JoyButtonSlot::JoyExecute: {
        buttonslot.setTextData(value);
        buttonslot.setSlotMode(mode);
        edited = true;
        break;
    }
    case JoyButtonSlot::JoyMix: {
        break;
    }
    default: {
        break;
    }
    }

    setText(buttonslot.getSlotString());
}

void SimpleKeyGrabberButton::setValues(QString value, JoyButtonSlot::JoySlotInputAction mode)
{
    switch (mode)
    {
    case JoyButtonSlot::JoyMix: {
        // buttonslot.setSlotCode(-1);
        buttonslot.setTextData(value);
        buttonslot.setSlotMode(JoyButtonSlot::JoyMix);
        edited = true;
        break;
    }
    default: {
        break;
    }
    }

    setText(buttonslot.getSlotString());
}

void SimpleKeyGrabberButton::setValues(QString value, QList<JoyButtonSlot *> *jbtn, JoyButtonSlot::JoySlotInputAction mode)
{
    switch (mode)
    {
    case JoyButtonSlot::JoyMix: {
        // buttonslot.setSlotCode(-1);
        buttonslot.setTextData(value);
        buttonslot.setSlotMode(JoyButtonSlot::JoyMix);
        buttonslot.setMixSlots(jbtn);
        edited = true;
        break;
    }
    default: {
        break;
    }
    }

    setText(buttonslot.getSlotString());
}

JoyButtonSlot *SimpleKeyGrabberButton::getValue() { return &buttonslot; }

JoyButtonSlot &SimpleKeyGrabberButton::getValueNonPointer() { return buttonslot; }

void SimpleKeyGrabberButton::setValue(JoyButtonSlot *jbS) { buttonslot = jbS; }

void SimpleKeyGrabberButton::refreshButtonLabel()
{
    setText(buttonslot.getSlotString());
    updateGeometry();
}

bool SimpleKeyGrabberButton::isEdited() { return edited; }

bool SimpleKeyGrabberButton::isGrabbing() { return grabNextAction; }
