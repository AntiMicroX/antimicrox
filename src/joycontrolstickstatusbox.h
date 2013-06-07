#ifndef JOYCONTROLSTICKSTATUSBOX_H
#define JOYCONTROLSTICKSTATUSBOX_H

#include <QWidget>
#include <QSize>

#include "joycontrolstick.h"
#include "joyaxis.h"

class JoyControlStickStatusBox : public QWidget
{
    Q_OBJECT
public:
    explicit JoyControlStickStatusBox(QWidget *parent = 0);
    explicit JoyControlStickStatusBox(JoyControlStick *stick, QWidget *parent = 0);

    void setStick(JoyControlStick *stick);

    JoyControlStick* getStick();

    virtual int heightForWidth(int width) const;
    QSize sizeHint() const;

protected:
    virtual void paintEvent(QPaintEvent *event);

    JoyControlStick *stick;

signals:
    
public slots:
    
};

#endif // JOYCONTROLSTICKSTATUSBOX_H
