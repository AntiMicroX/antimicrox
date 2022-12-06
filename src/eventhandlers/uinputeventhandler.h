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

#ifndef UINPUTEVENTHANDLER_H
#define UINPUTEVENTHANDLER_H

#include "baseeventhandler.h"

/**
 * @brief Input event handler class using uinput files
 *
 * Proper udev rule may be needed for usage
 * General info
 * https://kernel.org/doc/html/v5.15/input/uinput.html
 * Event types
 * https://www.kernel.org/doc/html/v5.15/input/event-codes.html
 *
 */
class UInputEventHandler : public BaseEventHandler
{
    Q_OBJECT

  public:
    explicit UInputEventHandler(QObject *parent = nullptr);
    ~UInputEventHandler();

    virtual bool init() override;
    virtual bool cleanup() override;
    virtual void sendKeyboardEvent(JoyButtonSlot *slot, bool pressed) override;
    virtual void sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed) override;
    virtual void sendMouseEvent(int xDis, int yDis) override;
    virtual void sendMouseAbsEvent(int xDis, int yDis, int screen) override;

    virtual void sendMouseSpringEvent(int xDis, int yDis, int width, int height) override;

    virtual QString getName() override;
    virtual QString getIdentifier() override;
    virtual void printPostMessages() override;

    virtual void sendTextEntryEvent(QString maintext) override;

    int getKeyboardFileHandler();
    int getMouseFileHandler();
    int getSpringMouseFileHandler();
    const QString getUinputDeviceLocation();

  protected:
    int openUInputHandle();
    void setKeyboardEvents(int filehandle);
    void setRelMouseEvents(int filehandle);
    void setSpringMouseEvents(int filehandle);
    void populateKeyCodes(int filehandle);
    void createUInputKeyboardDevice(int filehandle);
    void createUInputMouseDevice(int filehandle);
    void createUInputSpringMouseDevice(int filehandle);
    void closeUInputDevice(int filehandle);
    /**
     * @brief Write uinput event to selected file uinput file
     *
     * @param filehandle - C-style linux file handle obtained by open()
     * @param type type of event described in input-event-codes.h (for example EV_ABS )
     * @param code Additional code like ABS_X for type EV_ABS
     * @param value
     * @param syn synchronize after event (emit additional event used for separation of events EV_SYN)
     */
    void write_uinput_event(int filehandle, int type, int code, int value, bool syn = true);

  private slots:
#ifdef WITH_X11
    void x11ResetMouseAccelerationChange();
#endif

  private:
    int keyboardFileHandler;
    int mouseFileHandler;
    int springMouseFileHandler;
    QString uinputDeviceLocation;
#if defined(Q_OS_UNIX)
    bool is_problem_with_opening_uinput_present;
#endif

    bool cleanupUinputEvHand();
    void testAndAppend(bool tested, QList<unsigned int> &tempList, unsigned int key);
    void initDevice(int &device, QString name, bool &result);
};

#endif // UINPUTEVENTHANDLER_H
