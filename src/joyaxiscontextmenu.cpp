#include "joyaxiscontextmenu.h"
#include "mousedialog/mouseaxissettingsdialog.h"
#include "antkeymapper.h"

JoyAxisContextMenu::JoyAxisContextMenu(JoyAxis *axis, QWidget *parent) :
    QMenu(parent)
{
    this->axis = axis;

    connect(this, SIGNAL(aboutToHide()), this, SLOT(deleteLater()));
}

void JoyAxisContextMenu::buildMenu()
{
    bool actAsTrigger = false;
    if (axis->getThrottle() == JoyAxis::PositiveThrottle ||
        axis->getThrottle() == JoyAxis::PositiveHalfThrottle)
    {
        actAsTrigger = true;
    }

    if (actAsTrigger)
    {
        buildTriggerMenu();
    }
    else
    {
        buildAxisMenu();
    }
}

void JoyAxisContextMenu::buildAxisMenu()
{
    QAction *action = 0;

    QActionGroup *presetGroup = new QActionGroup(this);
    int presetMode = 0;
    int currentPreset = getPresetIndex();

    action = this->addAction(tr("Mouse (Horizontal)"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode+1);
    action->setData(QVariant(presetMode));
    connect(action, SIGNAL(triggered()), this, SLOT(setAxisPreset()));
    presetGroup->addAction(action);

    presetMode++;
    action = this->addAction(tr("Mouse (Inverted Horizontal)"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode+1);
    action->setData(QVariant(presetMode));
    connect(action, SIGNAL(triggered()), this, SLOT(setAxisPreset()));
    presetGroup->addAction(action);

    presetMode++;
    action = this->addAction(tr("Mouse (Vertical)"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode+1);
    action->setData(QVariant(presetMode));
    connect(action, SIGNAL(triggered()), this, SLOT(setAxisPreset()));
    presetGroup->addAction(action);

    presetMode++;
    action = this->addAction(tr("Mouse (Inverted Vertical)"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode+1);
    action->setData(QVariant(presetMode));
    connect(action, SIGNAL(triggered()), this, SLOT(setAxisPreset()));
    presetGroup->addAction(action);

    presetMode++;
    action = this->addAction(tr("Arrows: Up | Down"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode+1);
    action->setData(QVariant(presetMode));
    connect(action, SIGNAL(triggered()), this, SLOT(setAxisPreset()));
    presetGroup->addAction(action);

    presetMode++;
    action = this->addAction(tr("Arrows: Left | Right"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode+1);
    action->setData(QVariant(presetMode));
    connect(action, SIGNAL(triggered()), this, SLOT(setAxisPreset()));
    presetGroup->addAction(action);

    presetMode++;
    action = this->addAction(tr("Keys: W | S"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode+1);
    action->setData(QVariant(presetMode));
    connect(action, SIGNAL(triggered()), this, SLOT(setAxisPreset()));
    presetGroup->addAction(action);

    presetMode++;
    action = this->addAction(tr("Keys: A | D"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode+1);
    action->setData(QVariant(presetMode));
    connect(action, SIGNAL(triggered()), this, SLOT(setAxisPreset()));
    presetGroup->addAction(action);

    presetMode++;
    action = this->addAction(tr("NumPad: KP_8 | KP_2"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode+1);
    action->setData(QVariant(presetMode));
    connect(action, SIGNAL(triggered()), this, SLOT(setAxisPreset()));
    presetGroup->addAction(action);

    presetMode++;
    action = this->addAction(tr("NumPad: KP_4 | KP_6"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode+1);
    action->setData(QVariant(presetMode));
    connect(action, SIGNAL(triggered()), this, SLOT(setAxisPreset()));
    presetGroup->addAction(action);

    presetMode++;
    action = this->addAction(tr("None"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode+1);
    action->setData(QVariant(presetMode));
    connect(action, SIGNAL(triggered()), this, SLOT(setAxisPreset()));
    presetGroup->addAction(action);

    this->addSeparator();

    action = this->addAction(tr("Mouse Settings"));
    action->setCheckable(false);
    connect(action, SIGNAL(triggered()), this, SLOT(openMouseSettingsDialog()));
}

int JoyAxisContextMenu::getPresetIndex()
{
    int result = 0;

    JoyAxisButton *naxisbutton = axis->getNAxisButton();
    QList<JoyButtonSlot*> *naxisslots = naxisbutton->getAssignedSlots();
    JoyAxisButton *paxisbutton = axis->getPAxisButton();
    QList<JoyButtonSlot*> *paxisslots = paxisbutton->getAssignedSlots();

    if (naxisslots->length() == 1 && paxisslots->length() == 1)
    {
        JoyButtonSlot *nslot = naxisslots->at(0);
        JoyButtonSlot *pslot = paxisslots->at(0);
        if (nslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && nslot->getSlotCode() == JoyButtonSlot::MouseLeft &&
            pslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && pslot->getSlotCode() == JoyButtonSlot::MouseRight)
        {
            result = 1;
        }
        else if (nslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && nslot->getSlotCode() == JoyButtonSlot::MouseRight &&
            pslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && pslot->getSlotCode() == JoyButtonSlot::MouseLeft)
        {
            result = 2;
        }
        else if (nslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && nslot->getSlotCode() == JoyButtonSlot::MouseUp &&
            pslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && pslot->getSlotCode() == JoyButtonSlot::MouseDown)
        {
            result = 3;
        }
        else if (nslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && nslot->getSlotCode() == JoyButtonSlot::MouseDown &&
            pslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && pslot->getSlotCode() == JoyButtonSlot::MouseUp)
        {
            result = 4;
        }
        else if (nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)nslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Up) &&
                 pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)pslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Down))
        {
            result = 5;
        }
        else if (nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)nslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Left) &&
                 pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)pslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Right))
        {
            result = 6;
        }
        else if (nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)nslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_W) &&
                 pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)pslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_S))
        {
            result = 7;
        }
        else if (nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)nslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_A) &&
                 pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)pslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_D))
        {
            result = 8;
        }
        else if (nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)nslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_8) &&
                 pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)pslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_2))
        {
            result = 9;
        }
        else if (nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)nslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_4) &&
                 pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)pslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_6))
        {
            result = 10;
        }
    }
    else if (naxisslots->length() == 0 && paxisslots->length() == 0)
    {
        result = 11;
    }

    return result;
}

void JoyAxisContextMenu::setAxisPreset()
{
    QAction *action = static_cast<QAction*>(sender());
    int item = action->data().toInt();

    JoyButtonSlot *nbuttonslot = 0;
    JoyButtonSlot *pbuttonslot = 0;

    if (item == 0)
    {
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
    }
    else if (item == 1)
    {
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
    }
    else if (item == 2)
    {
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
    }
    else if (item == 3)
    {
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
    }
    else if (item == 4)
    {
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Up), Qt::Key_Up, JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Down), Qt::Key_Down, JoyButtonSlot::JoyKeyboard, this);
    }
    else if (item == 5)
    {
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Left), Qt::Key_Left, JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Right), Qt::Key_Right, JoyButtonSlot::JoyKeyboard, this);
    }
    else if (item == 6)
    {
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_W), Qt::Key_W, JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_S), Qt::Key_S, JoyButtonSlot::JoyKeyboard, this);
    }
    else if (item == 7)
    {
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_A), Qt::Key_A, JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_D), Qt::Key_D, JoyButtonSlot::JoyKeyboard, this);
    }
    else if (item == 8)
    {
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_8), QtKeyMapperBase::AntKey_KP_8, JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_2), QtKeyMapperBase::AntKey_KP_2, JoyButtonSlot::JoyKeyboard, this);
    }
    else if (item == 9)
    {
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_4), QtKeyMapperBase::AntKey_KP_4, JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_6), QtKeyMapperBase::AntKey_KP_6, JoyButtonSlot::JoyKeyboard, this);
    }
    else if (item == 10)
    {
        JoyAxisButton *nbutton = axis->getNAxisButton();
        JoyAxisButton *pbutton = axis->getPAxisButton();
        nbutton->clearSlotsEventReset();
        pbutton->clearSlotsEventReset();
    }

    if (nbuttonslot)
    {
        JoyAxisButton *button = axis->getNAxisButton();
        button->clearSlotsEventReset(false);
        button->setAssignedSlot(nbuttonslot->getSlotCode(), nbuttonslot->getSlotCodeAlias(), nbuttonslot->getSlotMode());
        nbuttonslot->deleteLater();
    }

    if (pbuttonslot)
    {
        JoyAxisButton *button = axis->getPAxisButton();
        button->clearSlotsEventReset(false);
        button->setAssignedSlot(pbuttonslot->getSlotCode(), pbuttonslot->getSlotCodeAlias(), pbuttonslot->getSlotMode());
        pbuttonslot->deleteLater();
    }
}

void JoyAxisContextMenu::openMouseSettingsDialog()
{
    MouseAxisSettingsDialog *dialog = new MouseAxisSettingsDialog(this->axis, parentWidget());
    dialog->show();
}

void JoyAxisContextMenu::buildTriggerMenu()
{
    QAction *action = 0;

    QActionGroup *presetGroup = new QActionGroup(this);
    int presetMode = 0;
    int currentPreset = getTriggerPresetIndex();

    action = this->addAction(tr("Left Mouse Button"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode+1);
    action->setData(QVariant(presetMode));
    connect(action, SIGNAL(triggered()), this, SLOT(setTriggerPreset()));
    presetGroup->addAction(action);

    presetMode++;
    action = this->addAction(tr("Right Mouse Button"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode+1);
    action->setData(QVariant(presetMode));
    connect(action, SIGNAL(triggered()), this, SLOT(setTriggerPreset()));
    presetGroup->addAction(action);

    presetMode++;
    action = this->addAction(tr("None"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode+1);
    action->setData(QVariant(presetMode));
    connect(action, SIGNAL(triggered()), this, SLOT(setTriggerPreset()));
    presetGroup->addAction(action);

    this->addSeparator();

    action = this->addAction(tr("Mouse Settings"));
    action->setCheckable(false);
    connect(action, SIGNAL(triggered()), this, SLOT(openMouseSettingsDialog()));
}

int JoyAxisContextMenu::getTriggerPresetIndex()
{
    int result = 0;

    JoyAxisButton *paxisbutton = axis->getPAxisButton();
    QList<JoyButtonSlot*> *paxisslots = paxisbutton->getAssignedSlots();

    if (paxisslots->length() == 1)
    {
        JoyButtonSlot *pslot = paxisslots->at(0);
        if (pslot->getSlotMode() == JoyButtonSlot::JoyMouseButton && pslot->getSlotCode() == JoyButtonSlot::MouseLB)
        {
            result = 1;
        }
        else if (pslot->getSlotMode() == JoyButtonSlot::JoyMouseButton && pslot->getSlotCode() == JoyButtonSlot::MouseRB)
        {
            result = 2;
        }
    }
    else if (paxisslots->length() == 0)
    {
        result = 3;
    }

    return result;
}

void JoyAxisContextMenu::setTriggerPreset()
{
    QAction *action = static_cast<QAction*>(sender());
    int item = action->data().toInt();

    JoyButtonSlot *pbuttonslot = 0;

    if (item == 0)
    {
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseLB, JoyButtonSlot::JoyMouseButton, this);
    }
    else if (item == 1)
    {
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseRB, JoyButtonSlot::JoyMouseButton, this);
    }
    else if (item == 2)
    {
        JoyAxisButton *pbutton = axis->getPAxisButton();
        pbutton->clearSlotsEventReset();
    }

    if (pbuttonslot)
    {
        JoyAxisButton *button = axis->getPAxisButton();
        button->clearSlotsEventReset(false);
        button->setAssignedSlot(pbuttonslot->getSlotCode(), pbuttonslot->getSlotCodeAlias(), pbuttonslot->getSlotMode());
        pbuttonslot->deleteLater();
    }
}
