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


#include "simplekeygrabberbutton.h"
#include "event.h"
#include "antkeymapper.h"
#include "eventhandlerfactory.h"

#ifdef Q_OS_WIN
  #include "winextras.h"
#endif

#ifdef Q_OS_UNIX
#include <QApplication>
#endif

#include <QKeyEvent>
#include <QMouseEvent>
#include <QMetaType>
#include <QWidget>
#include <QDebug>

SimpleKeyGrabberButton::SimpleKeyGrabberButton(QWidget *parent) :
    QPushButton(parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    grabNextAction = false;
    grabbingWheel = false;
    edited = false;
    this->installEventFilter(this);
}

void SimpleKeyGrabberButton::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    // Do not allow closing of dialog using Escape key
    if (event->key() == Qt::Key_Escape)
    {
        return;
    }

    QPushButton::keyPressEvent(event);
}

bool SimpleKeyGrabberButton::eventFilter(QObject *obj, QEvent *event)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    Q_UNUSED(obj);

    int controlcode = 0;
    if (grabNextAction && (event->type() == QEvent::MouseButtonRelease))
    {
        QMouseEvent *mouseEve = static_cast<QMouseEvent*>(event);
        if (mouseEve->button() == Qt::RightButton)
        {
            controlcode = 3;
        }
        else if (mouseEve->button() == Qt::MiddleButton)
        {
            controlcode = 2;
        }
        else {
            controlcode = mouseEve->button();
        }

        buttonslot.setSlotCode(controlcode);
        buttonslot.setSlotMode(JoyButtonSlot::JoyMouseButton);
        refreshButtonLabel();
        edited = true;
        releaseMouse();
        releaseKeyboard();

        grabNextAction = grabbingWheel = false;
        emit buttonCodeChanged(controlcode);
    }
    else if (grabNextAction && (event->type() == QEvent::KeyRelease))
    {
        QKeyEvent *keyEve = static_cast<QKeyEvent*>(event);
        int tempcode = keyEve->nativeScanCode();
        int virtualactual = keyEve->nativeVirtualKey();

        BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();

        int finalvirtual = 0;
        int checkalias = 0;

#ifdef Q_OS_WIN
  #ifdef WITH_VMULTI
        if (handler->getIdentifier() == "vmulti")
        {
            finalvirtual = WinExtras::correctVirtualKey(tempcode, virtualactual);
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
            QtKeyMapperBase *nativeWinKeyMapper = AntKeyMapper::getInstance()->getNativeKeyMapper();
            int tempQtKey = 0;
            if (nativeWinKeyMapper)
            {
                tempQtKey = nativeWinKeyMapper->returnQtKey(finalvirtual);
            }

            if (tempQtKey > 0)
            {
                finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(tempQtKey);
                checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
            }
            else
            {
                finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(keyEve->key());
            }
        }

  #endif

        BACKEND_ELSE_IF (handler->getIdentifier() == "sendinput")
        {
            // Find more specific virtual key (VK_SHIFT -> VK_LSHIFT)
            // by checking for extended bit in scan code.
            finalvirtual = WinExtras::correctVirtualKey(tempcode, virtualactual);
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual, tempcode);
        }

#elif defined(Q_OS_UNIX)

    #if defined(WITH_X11)

        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
        // Obtain group 1 X11 keysym. Removes effects from modifiers.
        finalvirtual = X11KeyCodeToX11KeySym(tempcode);

        #ifdef WITH_UINPUT
        if (handler->getIdentifier() == "uinput")
        {
            // Find Qt Key corresponding to X11 KeySym.
            QtKeyMapperBase *x11KeyMapper = AntKeyMapper::getInstance()->getNativeKeyMapper();
            checkalias = x11KeyMapper->returnQtKey(finalvirtual);
            // Find corresponding Linux input key for the Qt key.
            finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(checkalias);
        }
        #endif

        #ifdef WITH_XTEST
        if (handler->getIdentifier() == "xtest")
        {
            // Check for alias against group 1 keysym.
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
        }
        #endif

        }
        else
        {
            // Not running on xcb platform.
            finalvirtual = tempcode;
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
        }

    #else
        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
        finalvirtual = AntKeyMapper::getInstance()->returnVirtualKey(keyEve->key());
        checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
        }
        else
        {
            // Not running on xcb platform.
            finalvirtual = tempcode;
            checkalias = AntKeyMapper::getInstance()->returnQtKey(finalvirtual);
        }

    #endif
#endif

        controlcode = tempcode;
        bool valueUpdated = false;

        if ((keyEve->modifiers() & Qt::ControlModifier) && (keyEve->key() == Qt::Key_X))
        {
            controlcode = 0;
            refreshButtonLabel();
        }
        else if (controlcode <= 0)
        {
            controlcode = 0;
            setText("");
            valueUpdated = true;
            edited = true;
        }
        else
        {
            if ((checkalias > 0) && (finalvirtual > 0))
            {
                buttonslot.setSlotCode(finalvirtual, checkalias);
                buttonslot.setSlotMode(JoyButtonSlot::JoyKeyboard);
                setText(keysymToKeyString(finalvirtual, checkalias).toUpper());

                edited = true;
                valueUpdated = true;
            }
            else if (virtualactual > 0)
            {
                buttonslot.setSlotCode(virtualactual);
                buttonslot.setSlotMode(JoyButtonSlot::JoyKeyboard);
                setText(keysymToKeyString(virtualactual).toUpper());

                edited = true;
                valueUpdated = true;
            }
        }

        grabNextAction = false;
        grabbingWheel = false;
        releaseMouse();
        releaseKeyboard();

        if (valueUpdated)
        {
            emit buttonCodeChanged(controlcode);
        }
    }
    else if (grabNextAction && (event->type() == QEvent::Wheel) && !grabbingWheel)
    {
        grabbingWheel = true;
    }
    else if (grabNextAction && (event->type() == QEvent::Wheel))
    {
        QWheelEvent *wheelEve = static_cast<QWheelEvent*>(event);
        QString text = QString(trUtf8("Mouse")).append(" ");

        if ((wheelEve->orientation() == Qt::Vertical) && (wheelEve->delta() >= 120))
        {
            controlcode = 4;
        }
        else if ((wheelEve->orientation() == Qt::Vertical) && (wheelEve->delta() <= -120))
        {
            controlcode = 5;
        }
        else if ((wheelEve->orientation() == Qt::Horizontal) && (wheelEve->delta() >= 120))
        {
            controlcode = 6;
        }
        else if ((wheelEve->orientation() == Qt::Horizontal) && (wheelEve->delta() <= -120))
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
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *mouseEve = static_cast<QMouseEvent*>(event);
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    buttonslot.setSlotCode(value, alias);
    buttonslot.setSlotMode(mode);
    edited = true;

    setText(buttonslot.getSlotString());
}

void SimpleKeyGrabberButton::setValue(int value, JoyButtonSlot::JoySlotInputAction mode)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    buttonslot.setSlotCode(value);
    buttonslot.setSlotMode(mode);
    edited = true;

    setText(buttonslot.getSlotString());
}

void SimpleKeyGrabberButton::setValue(QString value, JoyButtonSlot::JoySlotInputAction mode)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    switch (mode)
    {
        case JoyButtonSlot::JoyLoadProfile:
        case JoyButtonSlot::JoyTextEntry:
        case JoyButtonSlot::JoyExecute:
        {
            buttonslot.setTextData(value);
            buttonslot.setSlotMode(mode);
            edited = true;
            break;
        }
    case JoyButtonSlot::JoyKeyboard:
    {
        break;
    }
    case JoyButtonSlot::JoyMouseButton:
    {
        break;
    }
    case JoyButtonSlot::JoyMouseMovement:
    {
        break;
    }
    case JoyButtonSlot::JoyPause:
    {
        break;
    }
    case JoyButtonSlot::JoyHold:
    {
        break;
    }
    case JoyButtonSlot::JoyCycle:
    {
        break;
    }
    case JoyButtonSlot::JoyDistance:
    {
        break;
    }
    case JoyButtonSlot::JoyRelease:
    {
        break;
    }
    case JoyButtonSlot::JoyMouseSpeedMod:
    {
        break;
    }
    case JoyButtonSlot::JoyKeyPress:
    {
        break;
    }
    case JoyButtonSlot::JoyDelay:
    {
        break;
    }
    case JoyButtonSlot::JoySetChange:
    {
        break;
    }
    default:
    {
        break;
    }
    }

    setText(buttonslot.getSlotString());
}

JoyButtonSlot* SimpleKeyGrabberButton::getValue()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return &buttonslot;
}

void SimpleKeyGrabberButton::refreshButtonLabel()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    setText(buttonslot.getSlotString());
}

bool SimpleKeyGrabberButton::isEdited()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return edited;
}

bool SimpleKeyGrabberButton::isGrabbing()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return grabNextAction;
}
