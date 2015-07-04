#ifndef JOYBUTTONMOUSEHELPER_H
#define JOYBUTTONMOUSEHELPER_H

#include <QObject>

class JoyButtonMouseHelper : public QObject
{
    Q_OBJECT
public:
    explicit JoyButtonMouseHelper(QObject *parent = 0);
    void resetButtonMouseDistances();
    void setFirstSpringStatus(bool status);
    bool getFirstSpringStatus();
    void carryGamePollRateUpdate(unsigned int pollRate);
    void carryMouseRefreshRateUpdate(unsigned int refreshRate);

protected:
    bool firstSpringEvent;

signals:
    void mouseCursorMoved(int mouseX, int mouseY, int elapsed);
    void mouseSpringMoved(int mouseX, int mouseY);
    void gamepadRefreshRateUpdated(unsigned int pollRate);
    void mouseRefreshRateUpdated(unsigned int refreshRate);

public slots:
    void moveMouseCursor();
    void moveSpringMouse();
    void mouseEvent();
};

#endif // JOYBUTTONMOUSEHELPER_H
