/* antimicroX Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.com>
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

#include "gamecontroller.h"

#include "globalvariables.h"
#include "gamecontrollerdpad.h"
#include "gamecontrollerset.h"
#include "antimicrosettings.h"
#include "joycontrolstick.h"
#include "joybuttontypes/joycontrolstickbutton.h"
#include "common.h"
#include "messagehandler.h"
//#include "logger.h"

#include <cmath>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>


GameController::GameController(SDL_GameController *controller, int deviceIndex,
                               AntiMicroSettings *settings, int counterUniques, QObject *parent) :
    InputDevice(SDL_GameControllerGetJoystick(controller), deviceIndex, settings, parent)
{

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->controller = controller;
    this->counterUniques = counterUniques;

    SDL_Joystick *joyhandle = SDL_GameControllerGetJoystick(controller);
    joystickID = SDL_JoystickInstanceID(joyhandle);

    for (int i = 0; i < GlobalVariables::InputDevice::NUMBER_JOYSETS; i++)
    {
        GameControllerSet *controllerset = new GameControllerSet(this, i, this);
        getJoystick_sets().insert(i, controllerset);
        enableSetConnections(controllerset);
    }
}


QString GameController::getName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return QString(tr("Game Controller")).append(" ").append(QString::number(getRealJoyNumber()));
}


QString GameController::getSDLName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();

    if (controller != nullptr)
    {
        temp = SDL_GameControllerName(controller);
    }

    return temp;
}

QString GameController::getXmlName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return GlobalVariables::GameController::xmlName;
}


QString GameController::getGUIDString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getRawGUIDString();
}


QString GameController::getVendorString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getRawVendorString();
}


QString GameController::getProductIDString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getRawProductIDString();
}


QString GameController::getUniqueIDString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getRawUniqueIDString();
}

QString GameController::getProductVersion()
{
    return getRawProductVersion();
}


QString GameController::getRawGUIDString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();

    if (controller != nullptr)
    {
        SDL_Joystick *joyhandle = SDL_GameControllerGetJoystick(controller);

        if (joyhandle != nullptr)
        {
            SDL_JoystickGUID tempGUID = SDL_JoystickGetGUID(joyhandle);
            char guidString[65] = {'0'};
            SDL_JoystickGetGUIDString(tempGUID, guidString, sizeof(guidString));
            temp = QString(guidString);
        }
    }

    return temp;
}


QString GameController::getRawVendorString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();

    if (controller != nullptr)
    {
            Uint16 tempVendor = SDL_GameControllerGetVendor(controller);
            char buffer [50];
            sprintf (buffer, "%u", tempVendor);

            temp = QString(buffer);
    }

    return temp;
}


QString GameController::getRawProductIDString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();

    if (controller != nullptr)
    {
            Uint16 tempProduct = SDL_GameControllerGetProduct(controller) + counterUniques;
            char buffer [50];
            sprintf (buffer, "%u", tempProduct);

            temp = QString(buffer);
    }

    return temp;
}

QString GameController::getRawProductVersion()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();

    if (controller != nullptr)
    {
            Uint16 tempProductVersion = SDL_GameControllerGetProductVersion(controller);
            char buffer [50];
            sprintf (buffer, "%u", tempProductVersion);

            temp = QString(buffer);
    }

    return temp;
}


QString GameController::getRawUniqueIDString()
{
    return (getRawGUIDString() + getRawVendorString() + getRawProductIDString());
}


void GameController::closeSDLDevice()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((controller != nullptr) && SDL_GameControllerGetAttached(controller))
    {
        SDL_GameControllerClose(controller);
        controller = nullptr;
    }
}


int GameController::getNumberRawButtons()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return SDL_CONTROLLER_BUTTON_MAX;
}


int GameController::getNumberRawAxes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);


    qDebug() << "Controller has " << SDL_CONTROLLER_AXIS_MAX << " raw axes";


    return SDL_CONTROLLER_AXIS_MAX;
}


int GameController::getNumberRawHats()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return 0;
}

void GameController::setCounterUniques(int counter)
{
    counterUniques = counter;
}


void GameController::fillContainers(QHash<int, SDL_GameControllerButton> &buttons, QHash<int, SDL_GameControllerAxis> &axes, QList<SDL_GameControllerButtonBind> &hatButtons)
{

    for (int i = 0; i < SDL_JoystickNumHats(getJoyHandle()); i++)
    {
        SDL_GameControllerButton currentButton = static_cast<SDL_GameControllerButton>(i);
        SDL_GameControllerButtonBind bound = SDL_GameControllerGetBindForButton(this->controller, currentButton);


        qDebug() << "Hat " << (i + 1);


        if (bound.bindType == SDL_CONTROLLER_BINDTYPE_HAT)
        {
            hatButtons.append(bound);
        }
    }

    for (int i = 0; i < SDL_JoystickNumButtons(getJoyHandle()); i++)
    {

        qDebug() << "Button " << (i + 1);


        SDL_GameControllerButton currentButton = static_cast<SDL_GameControllerButton>(i);
        SDL_GameControllerButtonBind bound = SDL_GameControllerGetBindForButton(this->controller, currentButton);

        if (bound.bindType == SDL_CONTROLLER_BINDTYPE_BUTTON)
        {
            buttons.insert(bound.value.button, currentButton);
        }
    }

    for (int i = 0; i < SDL_JoystickNumAxes(getJoyHandle()); i++)
    {

        qDebug() << "Axis " << (i + 1);


        SDL_GameControllerAxis currentAxis = static_cast<SDL_GameControllerAxis>(i);
        SDL_GameControllerButtonBind bound = SDL_GameControllerGetBindForAxis(this->controller, currentAxis);

        if (bound.bindType == SDL_CONTROLLER_BINDTYPE_AXIS)
        {
            axes.insert(bound.value.axis, currentAxis);
        }
    }
}


QString GameController::getBindStringForAxis(int index, bool)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();

    SDL_GameControllerButtonBind bind =
            SDL_GameControllerGetBindForAxis(controller,
                                             static_cast<SDL_GameControllerAxis>(index));


    if (bind.bindType == SDL_CONTROLLER_BINDTYPE_BUTTON)
    {
        temp.append(QString("Button %1").arg(bind.value.button));
    }
    else if (bind.bindType == SDL_CONTROLLER_BINDTYPE_AXIS)
    {
        temp.append(QString("Axis %1").arg(bind.value.axis + 1));
    }

    return temp;
}


QString GameController::getBindStringForButton(int index, bool trueIndex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();

    SDL_GameControllerButtonBind bind =
            SDL_GameControllerGetBindForButton(controller,
                                               static_cast<SDL_GameControllerButton>(index));


    int offset = trueIndex ? 0 : 1;
    int bindInt = static_cast<int>(bind.bindType);

    switch(bindInt) {

            case SDL_CONTROLLER_BINDTYPE_BUTTON:
                temp.append(QString("Button %1").arg(bind.value.button + offset));
            break;


            case SDL_CONTROLLER_BINDTYPE_AXIS:
                temp.append(QString("Axis %1").arg(bind.value.axis + offset));
            break;


            case SDL_CONTROLLER_BINDTYPE_HAT:
                temp.append(QString("Hat %1.%2").arg(bind.value.hat.hat + offset)
                    .arg(bind.value.hat.hat_mask));
            break;
    }

    return temp;
}


SDL_GameControllerButtonBind GameController::getBindForAxis(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return SDL_GameControllerGetBindForAxis(controller, static_cast<SDL_GameControllerAxis>(index));
}


SDL_GameControllerButtonBind GameController::getBindForButton(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return SDL_GameControllerGetBindForButton(controller, static_cast<SDL_GameControllerButton>(index));
}


void GameController::buttonClickEvent(int)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
}


void GameController::buttonReleaseEvent(int)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
}


void GameController::axisActivatedEvent(int, int, int)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
}


SDL_JoystickID GameController::getSDLJoystickID()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return joystickID;
}

/**
 * @brief Check if device is using the SDL Game Controller API
 * @return Status showing if device is using the Game Controller API
 */
bool GameController::isGameController()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return true;
}


/**
 * @brief Check if GUID passed matches the expected GUID for a device.
 *     Needed for xinput GUID abstraction.
 * @param GUID string
 * @return if GUID is considered a match.
 */
//bool GameController::isRelevantGUID(QString tempGUID)
//{
//    qInstallMessageHandler(MessageHandler::myMessageOutput);

//    return InputDevice::isRelevantGUID(tempGUID);
//}


bool GameController::isRelevantUniqueID(QString tempUniqueID)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return InputDevice::isRelevantUniqueID(tempUniqueID);
}


void GameController::rawButtonEvent(int index, bool pressed)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool knownbutton = getRawbuttons().contains(index);

    if (!knownbutton && pressed)
    {
        rawbuttons.insert(index, pressed);
        emit rawButtonClick(index);
    }
    else if (knownbutton && !pressed)
    {
        rawbuttons.remove(index);
        emit rawButtonRelease(index);
    }
}


void GameController::rawAxisEvent(int index, int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool knownaxis = getAxisvalues().contains(index);

    if (!knownaxis && (fabs(value) > rawAxisDeadZone))
    {
        axisvalues.insert(index, value);
        emit rawAxisActivated(index, value);
    }
    else if (knownaxis && (fabs(value) < rawAxisDeadZone))
    {
        axisvalues.remove(index);
        emit rawAxisReleased(index, value);
    }

    emit rawAxisMoved(index, value);
}


void GameController::rawDPadEvent(int index, int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool knowndpad = getDpadvalues().contains(index);

    if (!knowndpad && (value != 0))
    {
        dpadvalues.insert(index, value);
        emit rawDPadButtonClick(index, value);
    }
    else if (knowndpad && (value == 0))
    {
        dpadvalues.remove(index);
        emit rawDPadButtonRelease(index, value);
    }
}


QHash<int, bool> const& GameController::getRawbuttons()  {

    return rawbuttons;
}


QHash<int, int> const& GameController::getAxisvalues()  {

    return axisvalues;
}


QHash<int, int> const& GameController::getDpadvalues()  {

    return dpadvalues;
}


SDL_GameController* GameController::getController() const  {

    return controller;
}
