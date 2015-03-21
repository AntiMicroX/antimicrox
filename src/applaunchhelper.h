#ifndef APPLAUNCHHELPER_H
#define APPLAUNCHHELPER_H

#include <QObject>
#include <QMap>

#include "inputdevice.h"
#include "joybutton.h"
#include "antimicrosettings.h"

class AppLaunchHelper : public QObject
{
    Q_OBJECT
public:
    explicit AppLaunchHelper(AntiMicroSettings *settings, bool graphical=false, QObject *parent=0);

    void initRunMethods();
    void printControllerList(QMap<SDL_JoystickID, InputDevice *> *joysticks);

protected:
    void enablePossibleMouseSmoothing();
    void changeMouseRefreshRate();
#ifdef Q_OS_WIN
    void checkPointerPrecision();
#endif

    AntiMicroSettings *settings;
    bool graphical;

signals:

public slots:
#ifdef Q_OS_WIN
    void appQuitPointerPrecision();
#endif

};

#endif // APPLAUNCHHELPER_H
