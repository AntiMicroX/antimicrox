#include "virtualmousepushbutton.h"

VirtualMousePushButton::VirtualMousePushButton(QString displayText, int code, JoyButtonSlot::JoySlotInputAction mode, QWidget *parent) :
    QPushButton(parent)
{
    if (mode == JoyButtonSlot::JoyMouseButton || mode == JoyButtonSlot::JoyMouseMovement)
    {
        this->setText(displayText);

        if (mode == JoyButtonSlot::JoyMouseMovement)
        {
            switch (code)
            {
                case JoyButtonSlot::MouseUp:
                case JoyButtonSlot::MouseDown:
                case JoyButtonSlot::MouseLeft:
                case JoyButtonSlot::MouseRight:
                {
                    this->code = code;
                    break;
                }
                default:
                {
                    this->code = 0;
                    break;
                }
            }
        }
        else
        {
            this->code = code;
        }
        this->mode = mode;
    }
    else
    {
        this->setText("INVALID");
        this->code = 0;
        this->mode = JoyButtonSlot::JoyMouseButton;
    }

    connect(this, SIGNAL(clicked()), this, SLOT(createTempSlot()));
}

unsigned int VirtualMousePushButton::getMouseCode()
{
    return code;
}

JoyButtonSlot::JoySlotInputAction VirtualMousePushButton::getMouseMode()
{
    return mode;
}

void VirtualMousePushButton::createTempSlot()
{
    JoyButtonSlot *tempslot = new JoyButtonSlot(this->code, this->mode, this);
    emit mouseSlotCreated(tempslot);
}
