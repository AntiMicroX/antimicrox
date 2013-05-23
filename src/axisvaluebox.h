#ifndef AXISVALUEBOX_H
#define AXISVALUEBOX_H

#include <QWidget>

class AxisValueBox : public QWidget
{
    Q_OBJECT
public:
    explicit AxisValueBox(QWidget *parent = 0);

    int getDeadZone();
    int getMaxZone();
    int getJoyValue();
    int getThrottle();

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void paintEvent(QPaintEvent *event);

    int deadZone;
    int maxZone;
    int joyValue;
    int throttle;
    int boxwidth;
    int boxheight;
    int lboxstart;
    int lboxend;
    int rboxstart;
    int rboxend;
    int singlewidth;
    int singleend;

signals:
    
public slots:
    void setThrottle(int throttle);
    void setValue(int value);
    void setDeadZone(int deadZone);
    void setMaxZone(int maxZone);

};

#endif // AXISVALUEBOX_H
