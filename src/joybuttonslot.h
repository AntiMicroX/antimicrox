/* antimicroX Gamepad to KB+M event mapper
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


#ifndef JOYBUTTONSLOT_H
#define JOYBUTTONSLOT_H

#include <QObject>
#include <QElapsedTimer>
#include <QTime>
#include <QVariant>
#include <QPointer>
#include <QtWidgets/QApplication>

class QXmlStreamReader;
class QXmlStreamWriter;

class JoyButtonSlot : public QObject
{
    Q_OBJECT

public:
    enum JoySlotInputAction {JoyKeyboard=0, JoyMouseButton, JoyMouseMovement,
                             JoyPause, JoyHold, JoyCycle, JoyDistance,
                             JoyRelease, JoyMouseSpeedMod, JoyKeyPress, JoyDelay,
                             JoyLoadProfile, JoySetChange, JoyTextEntry, JoyExecute, JoyMix};

    enum JoySlotMouseDirection {MouseUp=1, MouseDown, MouseLeft, MouseRight};
    enum JoySlotMouseWheelButton {MouseWheelUp=4, MouseWheelDown=5,
                                  MouseWheelLeft=6, MouseWheelRight=7};
    enum JoySlotMouseButton {MouseLB=1, MouseMB, MouseRB};

    explicit JoyButtonSlot(QObject *parent = nullptr);
    explicit JoyButtonSlot(int code, JoySlotInputAction mode, QObject *parent=nullptr);
    explicit JoyButtonSlot(int code, int alias, JoySlotInputAction mode, QObject *parent=nullptr);
    explicit JoyButtonSlot(JoyButtonSlot *slot, QObject *parent=nullptr);
    explicit JoyButtonSlot(QString text, JoySlotInputAction mode, QObject *parent=nullptr);
    ~JoyButtonSlot();

    void setSlotCode(int code);
    int getSlotCode();
    void setSlotMode(JoySlotInputAction selectedMode);
    JoySlotInputAction getSlotMode();
    QString movementString();
    void setMouseSpeed(int value);
    void setDistance(double distance);
    double getMouseDistance();
    QElapsedTimer* getMouseInterval();
    void restartMouseInterval();
    QString getXmlName();
    QString getSlotString();
    void setSlotCode(int code,int alias);
    int getSlotCodeAlias();
    void setPreviousDistance(double distance);
    double getPreviousDistance();
    double getDistance() const;
    bool isModifierKey();

    bool isEasingActive();
    void setEasingStatus(bool isActive);
    QTime* getEasingTime();

    void setTextData(QString textData);
    QString getTextData();

    void setExtraData(QVariant data);
    QVariant getExtraData();

    void setMixSlots(QList<JoyButtonSlot*>* slots);
    QList<JoyButtonSlot*>* getMixSlots();

    template <typename T> void appendMiniSlot(T minislot)
    {
        secureMixSlotsInit();
        mix_slots->append(minislot);
    }

    void assignMixSlotsToNull();

    bool isValidSlot();

    void cleanMixSlots();

    JoyButtonSlot& operator=(JoyButtonSlot* slot);


private:
    void secureMixSlotsInit();

    int deviceCode;
    int qkeyaliasCode;
    JoySlotInputAction m_mode;
    QList<JoyButtonSlot*>* mix_slots;
    double m_distance;
    double previousDistance;
    QElapsedTimer mouseInterval;
    QTime easingTime;
    bool easingActive;
    QString m_textData;
    QVariant extraData;
    
};

Q_DECLARE_METATYPE(JoyButtonSlot*)
Q_DECLARE_METATYPE(JoyButtonSlot::JoySlotInputAction)


#endif // JOYBUTTONSLOT_H
