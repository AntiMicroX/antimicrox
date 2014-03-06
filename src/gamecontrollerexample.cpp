#include <QPainter>
#include <QPixmap>
#include <QTransform>

#include "gamecontrollerexample.h"

struct ButtonImagePlacement {
    int x;
    int y;
    GameControllerExample::ButtonType buttontype;
};

static ButtonImagePlacement buttonLocations[] = {
    {225, 98, GameControllerExample::Button}, // SDL_CONTROLLER_BUTTON_A
    {252, 77, GameControllerExample::Button}, // SDL_CONTROLLER_BUTTON_B
    {200, 77, GameControllerExample::Button}, // SDL_CONTROLLER_BUTTON_X
    {227, 59, GameControllerExample::Button}, // SDL_CONTROLLER_BUTTON_Y
    {102, 77, GameControllerExample::Button}, // SDL_CONTROLLER_BUTTON_BACK
    {169, 77, GameControllerExample::Button}, // SDL_CONTROLLER_BUTTON_START
    {137, 77, GameControllerExample::Button}, // SDL_CONTROLLER_BUTTON_GUIDE
    {45, 23, GameControllerExample::Button}, // SDL_CONTROLLER_BUTTON_LEFTSHOULDER
    {232, 21, GameControllerExample::Button}, // SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
    {44, 90, GameControllerExample::Button}, // SDL_CONTROLLER_BUTTON_LEFTSTICK
    {179, 135, GameControllerExample::Button}, // SDL_CONTROLLER_BUTTON_RIGHTSTICK

    {44, 90, GameControllerExample::AxisX}, // SDL_CONTROLLER_AXIS_LEFTX
    {44, 90, GameControllerExample::AxisY}, // SDL_CONTROLLER_AXIS_LEFTY
    {179, 135, GameControllerExample::AxisX}, // SDL_CONTROLLER_AXIS_RIGHTX
    {179, 135, GameControllerExample::AxisY}, // SDL_CONTROLLER_AXIS_RIGHTY

    {53, 0, GameControllerExample::Button}, // SDL_CONTROLLER_AXIS_TRIGGERLEFT
    {220, 0, GameControllerExample::Button}, // SDL_CONTROLLER_AXIS_TRIGGERRIGHT

    {90, 110, GameControllerExample::Button}, // SDL_CONTROLLER_BUTTON_DPAD_UP
    {68, 127, GameControllerExample::Button}, // SDL_CONTROLLER_BUTTON_DPAD_DOWN
    {90, 146, GameControllerExample::Button}, // SDL_CONTROLLER_BUTTON_DPAD_LEFT
    {109, 127, GameControllerExample::Button}, // SDL_CONTROLLER_BUTTON_DPAD_RIGHT
};

GameControllerExample::GameControllerExample(QWidget *parent) :
    QWidget(parent)
{
    controllerimage = QImage(":/images/controllermap.png");
    buttonimage = QImage(":/images/button.png");
    axisimage = QImage(":/images/axis.png");

    QTransform myTransform;
    myTransform.rotate(90);
    rotatedaxisimage = axisimage.transformed(myTransform);
    currentIndex = 0;

    connect(this, SIGNAL(indexUpdated(int)), this, SLOT(update()));
}

void GameControllerExample::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter paint(this);
    paint.drawImage(controllerimage.rect(), controllerimage);
    ButtonImagePlacement current = buttonLocations[currentIndex];

    paint.setOpacity(0.85);
    if (current.buttontype == Button)
    {
        paint.drawImage(QRect(current.x, current.y, buttonimage.width(), buttonimage.height()), buttonimage);
    }
    else if (current.buttontype == AxisX)
    {
        paint.drawImage(QRect(current.x, current.y, axisimage.width(), axisimage.height()), axisimage);
    }
    else if (current.buttontype == AxisY)
    {
        paint.drawImage(QRect(current.x, current.y, rotatedaxisimage.width(), rotatedaxisimage.height()), rotatedaxisimage);
    }

    paint.setOpacity(1.0);
}

void GameControllerExample::setActiveButton(int button)
{
    if (button >= 0 && button <= MAXBUTTONINDEX)
    {
        currentIndex = button;
        emit indexUpdated(button);
    }
}
