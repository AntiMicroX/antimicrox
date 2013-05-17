#include <QDebug>
#include <QFont>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QListIterator>

#include "virtualkeyboardmousewidget.h"
#include "event.h"

QHash<QString, QString> VirtualKeyboardMouseWidget::topRowKeys = QHash<QString, QString> ();

VirtualKeyboardMouseWidget::VirtualKeyboardMouseWidget(JoyButton *button, QWidget *parent) :
    QTabWidget(parent)
{
    populateTopRowKeys();

    qDebug() << "Floating in a tin can" << endl;

    this->button = button;
    keyboardTab = new QWidget(this);
    mouseTab = new QWidget(this);
    mouseHorizSpeedLabel = new QLabel("1 = 20pps", this);
    mouseHorizSpeedLabel->setAlignment(Qt::AlignRight);
    mouseVertSpeedLabel = new QLabel("1 = 20pps", this);
    mouseVertSpeedLabel->setAlignment(Qt::AlignRight);

    mouseHorizSpeedSpinBox = new QSpinBox(this);
    mouseHorizSpeedSpinBox->setMinimum(1);
    mouseHorizSpeedSpinBox->setMaximum(300);
    mouseHorizSpeedSpinBox->setValue(50);
    mouseVertSpeedSpinBox = new QSpinBox(this);
    mouseVertSpeedSpinBox->setMinimum(1);
    mouseVertSpeedSpinBox->setMaximum(300);
    mouseVertSpeedSpinBox->setValue(50);

    noneButton = createNoneKey();
    mouseChangeTogether = new QCheckBox("Change Together", this);

    this->addTab(keyboardTab, "Keyboard");
    this->addTab(mouseTab, "Mouse");

    this->setTabPosition(QTabWidget::South);

    setupVirtualKeyboardLayout();
    setupMouseControlLayout();
    establishVirtualKeyboardSingleSignalConnections();
    establishVirtualMouseSignalConnections();

    mouseHorizSpeedSpinBox->setValue(button->getMouseSpeedX());
    mouseVertSpeedSpinBox->setValue(button->getMouseSpeedY());
    updateHorizontalSpeedConvertLabel(button->getMouseSpeedX());
    updateVerticalSpeedConvertLabel(button->getMouseSpeedY());

    connect(mouseChangeTogether, SIGNAL(clicked()), this, SLOT(syncSpeedSpinBoxes()));
    connect(mouseHorizSpeedSpinBox, SIGNAL(valueChanged(int)), this, SLOT(moveSpeedsTogether(int)));
    connect(mouseVertSpeedSpinBox, SIGNAL(valueChanged(int)), this, SLOT(moveSpeedsTogether(int)));
    connect(mouseHorizSpeedSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateHorizontalSpeedConvertLabel(int)));
    connect(mouseVertSpeedSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateVerticalSpeedConvertLabel(int)));
    connect(mouseHorizSpeedSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setButtonMouseHorizSpeed(int)));
    connect(mouseVertSpeedSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setButtonMouseVertiSpeed(int)));
    //QList<VirtualKeyPushButton*> newlist = keyboardTab->findChildren<VirtualKeyPushButton*> ();
    //qDebug() << "COUNT: " << newlist.count() << endl;
}

VirtualKeyboardMouseWidget::VirtualKeyboardMouseWidget(QWidget *parent) :
    QTabWidget(parent)
{
    populateTopRowKeys();

    keyboardTab = new QWidget(this);
    mouseTab = new QWidget(this);
    mouseHorizSpeedLabel = new QLabel("1 = 20pps", this);
    mouseVertSpeedLabel = new QLabel("1 = 20pps", this);

    mouseHorizSpeedSpinBox = new QSpinBox(this);
    mouseHorizSpeedSpinBox->setMinimum(1);
    mouseHorizSpeedSpinBox->setMaximum(300);
    mouseVertSpeedSpinBox = new QSpinBox(this);
    mouseVertSpeedSpinBox->setMinimum(1);
    mouseVertSpeedSpinBox->setMaximum(300);

    noneButton = createNoneKey();
    mouseChangeTogether = new QCheckBox("Change Together", this);

    this->addTab(keyboardTab, "Keyboard");
    this->addTab(mouseTab, "Mouse");

    this->setTabPosition(QTabWidget::South);
}

void VirtualKeyboardMouseWidget::setupVirtualKeyboardLayout()
{
    QVBoxLayout *finalVBoxLayout = new QVBoxLayout(keyboardTab);

    QVBoxLayout *tempMainKeyLayout = setupMainKeyboardLayout();
    QVBoxLayout *tempAuxKeyLayout = setupAuxKeyboardLayout();
    QVBoxLayout *tempNumKeyPadLayout = setupKeyboardNumPadLayout();
    QHBoxLayout *tempHBoxLayout = new QHBoxLayout();

    tempHBoxLayout->addLayout(tempMainKeyLayout);
    tempHBoxLayout->addLayout(tempAuxKeyLayout);
    tempHBoxLayout->addLayout(tempNumKeyPadLayout);

    finalVBoxLayout->addLayout(tempHBoxLayout);
}

QVBoxLayout *VirtualKeyboardMouseWidget::setupMainKeyboardLayout()
{
    QHBoxLayout *tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);
    QVBoxLayout *tempVBoxLayout = new QVBoxLayout();
    QVBoxLayout *finalVBoxLayout = new QVBoxLayout();

    tempHBoxLayout->addWidget(createNewKey("Escape"));
    tempHBoxLayout->addSpacerItem(new QSpacerItem(40, 10, QSizePolicy::Expanding));
    tempHBoxLayout->addWidget(createNewKey("F1"));
    tempHBoxLayout->addWidget(createNewKey("F2"));
    tempHBoxLayout->addWidget(createNewKey("F3"));
    tempHBoxLayout->addWidget(createNewKey("F4"));
    tempHBoxLayout->addSpacerItem(new QSpacerItem(40, 10, QSizePolicy::Expanding));
    tempHBoxLayout->addWidget(createNewKey("F5"));
    tempHBoxLayout->addWidget(createNewKey("F6"));
    tempHBoxLayout->addWidget(createNewKey("F7"));
    tempHBoxLayout->addWidget(createNewKey("F8"));\
    tempHBoxLayout->addSpacerItem(new QSpacerItem(40, 10, QSizePolicy::Expanding));

    tempHBoxLayout->addWidget(createNewKey("F9"));
    tempHBoxLayout->addWidget(createNewKey("F10"));
    tempHBoxLayout->addWidget(createNewKey("F11"));
    tempHBoxLayout->addWidget(createNewKey("F12"));
    finalVBoxLayout->addLayout(tempHBoxLayout);

    finalVBoxLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed));

    tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);
    tempHBoxLayout->addWidget(createNewKey("grave"));
    for (int i=1; i <= 9; i++)
    {
        tempHBoxLayout->addWidget(createNewKey(QString::number(i)));
    }
    tempHBoxLayout->addWidget(createNewKey("0"));
    tempHBoxLayout->addWidget(createNewKey("minus"));
    tempHBoxLayout->addWidget(createNewKey("equal"));
    tempHBoxLayout->addWidget(createNewKey("BackSpace"));
    tempVBoxLayout->addLayout(tempHBoxLayout);

    QVBoxLayout *tempMiddleVLayout = new QVBoxLayout();
    QHBoxLayout *tempMiddleHLayout = new QHBoxLayout();
    tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);
    tempHBoxLayout->addWidget(createNewKey("Tab"));
    tempHBoxLayout->addSpacerItem(new QSpacerItem(10, 30, QSizePolicy::Fixed));
    tempHBoxLayout->addWidget(createNewKey("q"));
    tempHBoxLayout->addWidget(createNewKey("w"));
    tempHBoxLayout->addWidget(createNewKey("e"));
    tempHBoxLayout->addWidget(createNewKey("r"));
    tempHBoxLayout->addWidget(createNewKey("t"));
    tempHBoxLayout->addWidget(createNewKey("y"));
    tempHBoxLayout->addWidget(createNewKey("u"));
    tempHBoxLayout->addWidget(createNewKey("i"));\
    tempHBoxLayout->addWidget(createNewKey("o"));
    tempHBoxLayout->addWidget(createNewKey("p"));
    tempHBoxLayout->addWidget(createNewKey("bracketleft"));
    tempHBoxLayout->addWidget(createNewKey("bracketright"));
    tempHBoxLayout->addWidget(createNewKey("backslash"));
    tempMiddleVLayout->addLayout(tempHBoxLayout);

    tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);
    tempHBoxLayout->addWidget(createNewKey("Caps_Lock"));
    tempHBoxLayout->addWidget(createNewKey("a"));
    tempHBoxLayout->addWidget(createNewKey("s"));
    tempHBoxLayout->addWidget(createNewKey("d"));
    tempHBoxLayout->addWidget(createNewKey("f"));
    tempHBoxLayout->addWidget(createNewKey("g"));
    tempHBoxLayout->addWidget(createNewKey("h"));
    tempHBoxLayout->addWidget(createNewKey("j"));
    tempHBoxLayout->addWidget(createNewKey("k"));\
    tempHBoxLayout->addWidget(createNewKey("l"));
    tempHBoxLayout->addWidget(createNewKey("semicolon"));
    tempHBoxLayout->addWidget(createNewKey("apostrophe"));
    tempMiddleVLayout->addLayout(tempHBoxLayout);
    tempMiddleHLayout->addLayout(tempMiddleVLayout);
    tempMiddleHLayout->addWidget(createNewKey("Return"));
    tempVBoxLayout->addLayout(tempMiddleHLayout);

    tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);
    tempHBoxLayout->addWidget(createNewKey("Shift_L"));
    tempHBoxLayout->addWidget(createNewKey("z"));
    tempHBoxLayout->addWidget(createNewKey("x"));
    tempHBoxLayout->addWidget(createNewKey("c"));
    tempHBoxLayout->addWidget(createNewKey("v"));
    tempHBoxLayout->addWidget(createNewKey("b"));
    tempHBoxLayout->addWidget(createNewKey("n"));
    tempHBoxLayout->addWidget(createNewKey("m"));
    tempHBoxLayout->addWidget(createNewKey("comma"));\
    tempHBoxLayout->addWidget(createNewKey("period"));
    tempHBoxLayout->addWidget(createNewKey("slash"));
    tempHBoxLayout->addWidget(createNewKey("Shift_R"));
    tempVBoxLayout->addLayout(tempHBoxLayout);

    tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);
    tempHBoxLayout->addWidget(createNewKey("Control_L"));
    tempHBoxLayout->addWidget(createNewKey("Super_L"));
    tempHBoxLayout->addWidget(createNewKey("Alt_L"));
    tempHBoxLayout->addWidget(createNewKey("space"));
    tempHBoxLayout->addWidget(createNewKey("Alt_R"));
    tempHBoxLayout->addWidget(createNewKey("Menu"));
    tempHBoxLayout->addWidget(createNewKey("Control_R"));
    tempVBoxLayout->addLayout(tempHBoxLayout);

    tempVBoxLayout->setStretch(0, 1);
    tempVBoxLayout->setStretch(1, 2);
    tempVBoxLayout->setStretch(2, 1);
    tempVBoxLayout->setStretch(3, 1);

    finalVBoxLayout->addLayout(tempVBoxLayout);
    finalVBoxLayout->setStretch(0, 1);
    finalVBoxLayout->setStretch(1, 0);
    finalVBoxLayout->setStretch(2, 2);
    return finalVBoxLayout;
}

QVBoxLayout* VirtualKeyboardMouseWidget::setupAuxKeyboardLayout()
{
    QHBoxLayout *tempHBoxLayout = new QHBoxLayout();
    QVBoxLayout *tempVBoxLayout = new QVBoxLayout();
    QGridLayout *tempGridLayout = new QGridLayout();

    tempHBoxLayout->setSpacing(0);
    tempHBoxLayout->addWidget(createNewKey("Print"));
    tempHBoxLayout->addWidget(createNewKey("Scroll_Lock"));
    tempHBoxLayout->addWidget(createNewKey("Pause"));
    tempVBoxLayout->addLayout(tempHBoxLayout);

    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed));

    tempGridLayout->setSpacing(0);
    tempGridLayout->addWidget(createNewKey("Insert"), 1, 1, 1, 1);
    tempGridLayout->addWidget(createNewKey("Home"), 1, 2, 1, 1);
    tempGridLayout->addWidget(createNewKey("Prior"), 1, 3, 1, 1);
    tempGridLayout->addWidget(createNewKey("Delete"), 2, 1, 1, 1);
    tempGridLayout->addWidget(createNewKey("End"), 2, 2, 1, 1);
    tempGridLayout->addWidget(createNewKey("Next"), 2, 3, 1, 1);
    tempVBoxLayout->addLayout(tempGridLayout);

    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Fixed));

    tempGridLayout = new QGridLayout();
    tempGridLayout->setSpacing(0);
    tempGridLayout->addWidget(createNewKey("Up"), 1, 2, 1, 1);
    tempGridLayout->addWidget(createNewKey("Left"), 2, 1, 1, 1);
    tempGridLayout->addWidget(createNewKey("Down"), 2, 2, 1, 1);
    tempGridLayout->addWidget(createNewKey("Right"), 2, 3, 1, 1);
    tempVBoxLayout->addLayout(tempGridLayout);

    return tempVBoxLayout;
}

QVBoxLayout* VirtualKeyboardMouseWidget::setupKeyboardNumPadLayout()
{
    QHBoxLayout *tempHBoxLayout = new QHBoxLayout();
    QVBoxLayout *tempVBoxLayout = new QVBoxLayout();
    QGridLayout *tempGridLayout = new QGridLayout();
    QVBoxLayout *finalVBoxLayout = new QVBoxLayout();

    finalVBoxLayout->addWidget(noneButton);
    finalVBoxLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed));

    tempHBoxLayout->setSpacing(0);
    tempHBoxLayout->addWidget(createNewKey("Num_Lock"));
    tempHBoxLayout->addWidget(createNewKey("KP_Divide"));
    tempHBoxLayout->addWidget(createNewKey("KP_Multiply"));
    tempHBoxLayout->addWidget(createNewKey("KP_Subtract"));
    tempVBoxLayout->addLayout(tempHBoxLayout);

    tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);
    tempGridLayout->addWidget(createNewKey("KP_7"), 1, 1, 1, 1);
    tempGridLayout->addWidget(createNewKey("KP_8"), 1, 2, 1, 1);
    tempGridLayout->addWidget(createNewKey("KP_9"), 1, 3, 1, 1);
    tempGridLayout->addWidget(createNewKey("KP_4"), 2, 1, 1, 1);
    tempGridLayout->addWidget(createNewKey("KP_5"), 2, 2, 1, 1);
    tempGridLayout->addWidget(createNewKey("KP_6"), 2, 3, 1, 1);
    tempHBoxLayout->addLayout(tempGridLayout);
    tempHBoxLayout->addWidget(createNewKey("KP_Add"));
    tempVBoxLayout->addLayout(tempHBoxLayout);

    tempHBoxLayout = new QHBoxLayout();
    tempHBoxLayout->setSpacing(0);
    tempGridLayout = new QGridLayout();
    tempGridLayout->setSpacing(0);
    tempGridLayout->addWidget(createNewKey("KP_1"), 1, 1, 1, 1);
    tempGridLayout->addWidget(createNewKey("KP_2"), 1, 2, 1, 1);
    tempGridLayout->addWidget(createNewKey("KP_3"), 1, 3, 1, 1);
    tempGridLayout->addWidget(createNewKey("KP_0"), 2, 1, 1, 2);
    tempGridLayout->addWidget(createNewKey("KP_Decimal"), 2, 3, 1, 1);
    tempHBoxLayout->addLayout(tempGridLayout);
    tempHBoxLayout->addWidget(createNewKey("KP_Enter"));

    tempVBoxLayout->addLayout(tempHBoxLayout);
    finalVBoxLayout->addLayout(tempVBoxLayout);

    return finalVBoxLayout;
}

void VirtualKeyboardMouseWidget::setupMouseControlLayout()
{
    QHBoxLayout *tempHBoxLayout = new QHBoxLayout();
    QVBoxLayout *tempVBoxLayout = new QVBoxLayout();
    QGridLayout *tempGridLayout = new QGridLayout();
    QVBoxLayout *finalVBoxLayout = new QVBoxLayout(mouseTab);
    VirtualMousePushButton *pushButton = 0;
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    pushButton = new VirtualMousePushButton("Left", JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(50);
    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tempVBoxLayout->addWidget(pushButton);
    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tempHBoxLayout->addLayout(tempVBoxLayout);

    tempHBoxLayout->addSpacerItem(new QSpacerItem(10, 20, QSizePolicy::Fixed));

    tempVBoxLayout = new QVBoxLayout();
    pushButton = new VirtualMousePushButton("Up", JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(50);
    tempVBoxLayout->addWidget(pushButton);

    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

    QHBoxLayout *tempInnerHBoxLayout = new QHBoxLayout();
    pushButton = new VirtualMousePushButton("1", 1, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    tempInnerHBoxLayout->addWidget(pushButton);
    pushButton = new VirtualMousePushButton("2", 2, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    tempInnerHBoxLayout->addWidget(pushButton);
    pushButton = new VirtualMousePushButton("3", 3, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    tempInnerHBoxLayout->addWidget(pushButton);
    tempVBoxLayout->addLayout(tempInnerHBoxLayout);

    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

    pushButton = new VirtualMousePushButton("Wheel Up", 4, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(30);
    tempGridLayout->addWidget(pushButton, 1, 2, 1, 1);
    pushButton = new VirtualMousePushButton("Wheel Left", 6, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(30);
    tempGridLayout->addWidget(pushButton, 2, 1, 1, 1);
    pushButton = new VirtualMousePushButton("Wheel Right", 7, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(30);
    tempGridLayout->addWidget(pushButton, 2, 3, 1, 1);
    pushButton = new VirtualMousePushButton("Wheel Down", 5, JoyButtonSlot::JoyMouseButton, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(30);
    tempGridLayout->addWidget(pushButton, 3, 2, 1, 1);
    tempVBoxLayout->addLayout(tempGridLayout);

    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

    pushButton = new VirtualMousePushButton("Down", JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(50);
    tempVBoxLayout->addWidget(pushButton);
    tempVBoxLayout->setStretch(0, 1);
    tempVBoxLayout->setStretch(2, 1);
    tempVBoxLayout->setStretch(4, 3);
    tempVBoxLayout->setStretch(6, 1);

    tempHBoxLayout->addLayout(tempVBoxLayout);

    tempHBoxLayout->addSpacerItem(new QSpacerItem(10, 20, QSizePolicy::Fixed));

    tempVBoxLayout = new QVBoxLayout();
    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Expanding));
    pushButton = new VirtualMousePushButton("Right", JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setMinimumHeight(50);
    tempVBoxLayout->addWidget(pushButton);
    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 50, QSizePolicy::Minimum, QSizePolicy::Expanding));
    tempHBoxLayout->addLayout(tempVBoxLayout);

    tempHBoxLayout->addSpacerItem(new QSpacerItem(10, 20, QSizePolicy::Fixed));

    tempVBoxLayout = new QVBoxLayout();
    tempVBoxLayout->setSpacing(20);
    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

    tempVBoxLayout->addWidget(mouseChangeTogether);

    QVBoxLayout *tempInnerVBoxLayout = new QVBoxLayout();
    tempInnerHBoxLayout = new QHBoxLayout();
    QLabel *label = new QLabel("Mouse Horiz. Speed:", this);
    QFont font1;
    font1.setBold(true);
    font1.setPointSize(8);
    label->setFont(font1);

    tempInnerHBoxLayout->addWidget(label);
    tempInnerHBoxLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Fixed));

    tempInnerHBoxLayout->addWidget(mouseHorizSpeedSpinBox);
    tempInnerVBoxLayout->addLayout(tempInnerHBoxLayout);
    tempInnerVBoxLayout->addWidget(mouseHorizSpeedLabel);
    tempVBoxLayout->addLayout(tempInnerVBoxLayout);

    tempInnerVBoxLayout = new QVBoxLayout();
    tempInnerHBoxLayout = new QHBoxLayout();
    label = new QLabel("Mouse Vert. Speed:", this);
    QFont font2;
    font2.setBold(true);
    font2.setPointSize(8);
    label->setFont(font2);

    tempInnerHBoxLayout->addWidget(label);
    tempInnerHBoxLayout->addSpacerItem(new QSpacerItem(16, 10, QSizePolicy::Fixed));
    tempInnerHBoxLayout->addWidget(mouseVertSpeedSpinBox);
    tempInnerVBoxLayout->addLayout(tempInnerHBoxLayout);
    tempInnerVBoxLayout->addWidget(mouseVertSpeedLabel);
    tempVBoxLayout->addLayout(tempInnerVBoxLayout);

    tempVBoxLayout->addSpacerItem(new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));

    tempHBoxLayout->addLayout(tempVBoxLayout);
    finalVBoxLayout->addLayout(tempHBoxLayout);
}

VirtualKeyPushButton* VirtualKeyboardMouseWidget::createNewKey(QString xcodestring)
{
    int width = 30;
    int height = 30;
    QFont font1;
    font1.setPointSize(8);
    font1.setBold(true);

    VirtualKeyPushButton *pushButton = new VirtualKeyPushButton(button, xcodestring, this);

    if (xcodestring == "space")
    {
        width = 100;
    }
    else if (xcodestring == "Tab")
    {
        width = 40;
    }
    else if (xcodestring == "Shift_L")
    {
        width = 84;
    }
    else if (xcodestring == "Control_L")
    {
        width = 70;
    }
    else if (xcodestring == "Return")
    {
        width = 60;
        height = 60;
        pushButton->setMaximumWidth(100);
    }
    else if (xcodestring == "BackSpace")
    {
        width = 72;
    }
    else if (topRowKeys.contains(xcodestring))
    {
        width = 30;
        height = 36;
        pushButton->setMaximumSize(100, 100);
    }
    else if (xcodestring == "Print" || xcodestring == "Scroll_Lock" || xcodestring == "Pause")
    {
        width = 40;
        height = 36;
        pushButton->setMaximumSize(100, 100);
        font1.setPointSize(6);
    }
    else if (xcodestring == "KP_Add" || xcodestring == "KP_Enter")
    {
        width = 34;
        font1.setPointSize(6);
    }
    else if (xcodestring == "Num_Lock")
    {
        font1.setPointSize(6);
    }
    else if (xcodestring.startsWith("KP_"))
    {
        width = 36;
        height = 32;
    }

    pushButton->setObjectName(xcodestring);
    pushButton->setMinimumSize(width, height);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    pushButton->setSizePolicy(sizePolicy);
    pushButton->setFont(font1);

    return pushButton;
}

QPushButton* VirtualKeyboardMouseWidget::createNoneKey()
{
    QPushButton *pushButton = new QPushButton("NONE", this);
    pushButton->setMinimumSize(120, 40);
    pushButton->setMaximumHeight(100);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    pushButton->setSizePolicy(sizePolicy);
    QFont font1;
    font1.setBold(true);
    pushButton->setFont(font1);

    return pushButton;
}

void VirtualKeyboardMouseWidget::processSingleKeyboardSelection(int keycode)
{
    qDebug() << "KEYCODE: " << keycode << endl;
    qDebug() << "String: " << keycodeToKey(keycode) << endl;
    button->clearSlotsEventReset();
    button->setAssignedSlot(keycode);

    emit selectionFinished();
}

void VirtualKeyboardMouseWidget::processAdvancedKeyboardSelection(int keycode)
{
    emit selectionMade(keycode);
}

void VirtualKeyboardMouseWidget::processSingleMouseSelection(JoyButtonSlot *tempslot)
{
    button->clearSlotsEventReset();
    button->setAssignedSlot(tempslot->getSlotCode(), tempslot->getSlotMode());
    emit selectionFinished();
}

void VirtualKeyboardMouseWidget::processAdvancedMouseSelection(JoyButtonSlot *tempslot)
{
    emit selectionMade(tempslot);
}

void VirtualKeyboardMouseWidget::populateTopRowKeys()
{
    if (topRowKeys.isEmpty())
    {
        topRowKeys.insert("Escape", "Escape");
        topRowKeys.insert("F1", "F1");
        topRowKeys.insert("F2", "F2");
        topRowKeys.insert("F3", "F3");
        topRowKeys.insert("F4", "F4");
        topRowKeys.insert("F5", "F5");
        topRowKeys.insert("F6", "F6");
        topRowKeys.insert("F7", "F7");
        topRowKeys.insert("F8", "F8");
        topRowKeys.insert("F9", "F9");
        topRowKeys.insert("F10", "F10");
        topRowKeys.insert("F11", "F11");
        topRowKeys.insert("F12", "F12");
    }
}

void VirtualKeyboardMouseWidget::establishVirtualKeyboardSingleSignalConnections()
{
    QList<VirtualKeyPushButton*> newlist = keyboardTab->findChildren<VirtualKeyPushButton*> ();
    QListIterator<VirtualKeyPushButton*> iter(newlist);
    while (iter.hasNext())
    {
        VirtualKeyPushButton *keybutton = iter.next();
        disconnect(keybutton, SIGNAL(keycodeObtained(int)), 0, 0);
        connect(keybutton, SIGNAL(keycodeObtained(int)), this, SLOT(processSingleKeyboardSelection(int)));
    }

    disconnect(noneButton, SIGNAL(clicked()), 0, 0);
    connect(noneButton, SIGNAL(clicked()), this, SLOT(clearButtonSlotsFinish()));
    //qDebug() << "COUNT: " << newlist.count() << endl;
}

void VirtualKeyboardMouseWidget::establishVirtualKeyboardAdvancedSignalConnections()
{
    QList<VirtualKeyPushButton*> newlist = keyboardTab->findChildren<VirtualKeyPushButton*> ();
    QListIterator<VirtualKeyPushButton*> iter(newlist);
    while (iter.hasNext())
    {
        VirtualKeyPushButton *keybutton = iter.next();
        disconnect(keybutton, SIGNAL(keycodeObtained(int)), 0, 0);
        connect(keybutton, SIGNAL(keycodeObtained(int)), this, SLOT(processAdvancedKeyboardSelection(int)));
    }

    disconnect(noneButton, SIGNAL(clicked()), 0, 0);
    connect(noneButton, SIGNAL(clicked()), this, SLOT(clearButtonSlots()));
}

void VirtualKeyboardMouseWidget::establishVirtualMouseSignalConnections()
{
    QList<VirtualMousePushButton*> newlist = mouseTab->findChildren<VirtualMousePushButton*>();
    QListIterator<VirtualMousePushButton*> iter(newlist);
    while (iter.hasNext())
    {
        VirtualMousePushButton *mousebutton = iter.next();
        disconnect(mousebutton, SIGNAL(mouseSlotCreated(JoyButtonSlot*)), 0, 0);
        connect(mousebutton, SIGNAL(mouseSlotCreated(JoyButtonSlot*)), this, SLOT(processSingleMouseSelection(JoyButtonSlot*)));
    }
}

void VirtualKeyboardMouseWidget::establishVirtualMouseAdvancedSignalConnections()
{
    QList<VirtualMousePushButton*> newlist = mouseTab->findChildren<VirtualMousePushButton*>();
    QListIterator<VirtualMousePushButton*> iter(newlist);
    while (iter.hasNext())
    {
        VirtualMousePushButton *mousebutton = iter.next();
        disconnect(mousebutton, SIGNAL(mouseSlotCreated(JoyButtonSlot*)), 0, 0);
        connect(mousebutton, SIGNAL(mouseSlotCreated(JoyButtonSlot*)), this, SLOT(processAdvancedMouseSelection(JoyButtonSlot*)));
    }
}

void VirtualKeyboardMouseWidget::clearButtonSlots()
{
    button->clearSlotsEventReset();
    emit selectionCleared();
}

void VirtualKeyboardMouseWidget::clearButtonSlotsFinish()
{
    button->clearSlotsEventReset();
    emit selectionFinished();
}

void VirtualKeyboardMouseWidget::syncSpeedSpinBoxes()
{
    int temp = mouseHorizSpeedSpinBox->value();
    if (temp > mouseVertSpeedSpinBox->value())
    {
        mouseVertSpeedSpinBox->setValue(temp);
    }
    else
    {
        temp = mouseVertSpeedSpinBox->value();
        mouseHorizSpeedSpinBox->setValue(temp);
    }
}

void VirtualKeyboardMouseWidget::moveSpeedsTogether(int value)
{
    if (mouseChangeTogether->isChecked())
    {
        mouseHorizSpeedSpinBox->setValue(value);
        mouseVertSpeedSpinBox->setValue(value);
    }
}

void VirtualKeyboardMouseWidget::updateHorizontalSpeedConvertLabel(int value)
{
    QString label = QString (QString::number(value));
    label = label.append(" = ").append(QString::number(JoyButtonSlot::JOYSPEED * value)).append(" pps");
    mouseHorizSpeedLabel->setText(label);
}

void VirtualKeyboardMouseWidget::updateVerticalSpeedConvertLabel(int value)
{
    QString label = QString (QString::number(value));
    label = label.append(" = ").append(QString::number(JoyButtonSlot::JOYSPEED * value)).append(" pps");
    mouseVertSpeedLabel->setText(label);
}

void VirtualKeyboardMouseWidget::setButtonMouseHorizSpeed(int value)
{
    button->setMouseSpeedX(value);
}

void VirtualKeyboardMouseWidget::setButtonMouseVertiSpeed(int value)
{
    button->setMouseSpeedY(value);
}
