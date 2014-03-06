#ifndef GAMECONTROLLEREXAMPLE_H
#define GAMECONTROLLEREXAMPLE_H

#include <QWidget>
#include <QPaintEvent>

class GameControllerExample : public QWidget
{
    Q_OBJECT
public:
    explicit GameControllerExample(QWidget *parent = 0);

    enum ButtonType {
        Button, AxisX, AxisY,
    };

    static const unsigned int MAXBUTTONINDEX = 20;

protected:
    virtual void paintEvent(QPaintEvent *event);

    QImage controllerimage;
    QImage buttonimage;
    QImage axisimage;
    QImage rotatedaxisimage;
    int currentIndex;

signals:
    void indexUpdated(int index);

public slots:
    void setActiveButton(int button);
};

#endif // GAMECONTROLLEREXAMPLE_H
