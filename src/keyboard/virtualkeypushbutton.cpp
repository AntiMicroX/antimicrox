#include <QDebug>
#include <QPainter>

#include "virtualkeypushbutton.h"
#include "event.h"

QHash<QString, QString> VirtualKeyPushButton::knownAliases = QHash<QString, QString> ();

VirtualKeyPushButton::VirtualKeyPushButton(JoyButton *button, QString xcodestring, QWidget *parent) :
    QPushButton(parent)
{
    populateKnownAliases();

    //qDebug() << "Question: " << keyToKeycode("KP_7") << endl;
    //qDebug() << "Question: " << keycodeToKey(79) << endl;
    this->keycode = 0;
    this->xcodestring = "";
    this->displayString = "";
    this->currentlyActive = false;
    this->onCurrentButton = false;
    this->button = button;

    int temp = 0;
    if (!xcodestring.isEmpty())
    {
        temp = keyToKeycode(xcodestring);
    }

    if (temp > 0)
    {
        this->keycode = temp;
        this->xcodestring = xcodestring;
        this->displayString = setDisplayString(xcodestring);
    }

    this->setText(this->displayString.replace("&", "&&"));

    connect(this, SIGNAL(clicked()), this, SLOT(processSingleSelection()));
}

void VirtualKeyPushButton::processSingleSelection()
{
    emit keycodeObtained(keycode);
}

QString VirtualKeyPushButton::setDisplayString(QString xcodestring)
{
    QString temp;
    if (knownAliases.contains(xcodestring))
    {
        temp = knownAliases.value(xcodestring);
    }
    else
    {
        temp = keycodeToKey(keyToKeycode(xcodestring));
    }

    if (temp.isEmpty() && !xcodestring.isEmpty())
    {
        temp = xcodestring;
    }

    return temp.toUpper();
}

void VirtualKeyPushButton::populateKnownAliases()
{
    if (knownAliases.isEmpty())
    {
        knownAliases.insert("space", tr("Space"));
        knownAliases.insert("Tab", tr("Tab"));
        knownAliases.insert("Shift_L", tr("Shift (L)"));
        knownAliases.insert("Shift_R", tr("Shift (R)"));
        knownAliases.insert("Control_L", tr("Ctrl (L)"));
        knownAliases.insert("Control_R", tr("Ctrl (R)"));
        knownAliases.insert("Alt_L", tr("Alt (L)"));
        knownAliases.insert("Alt_R", tr("Alt (R)"));
        knownAliases.insert("grave", tr("`"));
        knownAliases.insert("asciitilde", tr("~"));
        knownAliases.insert("minus", tr("-"));
        knownAliases.insert("equal", tr("="));
        knownAliases.insert("bracketleft", tr("["));
        knownAliases.insert("bracketright", tr("]"));
        knownAliases.insert("backslash", tr("\\"));
        knownAliases.insert("Caps_Lock", tr("Caps"));
        knownAliases.insert("semicolon", tr(";"));
        knownAliases.insert("apostrophe", tr("'"));
        knownAliases.insert("comma", tr(","));
        knownAliases.insert("period", tr("."));
        knownAliases.insert("slash", tr("/"));
        knownAliases.insert("Escape", tr("ESC"));
        knownAliases.insert("Print", tr("PRTSC"));
        knownAliases.insert("Scroll_Lock", tr("SCLK"));
        knownAliases.insert("Insert", tr("INS"));
        knownAliases.insert("Prior", tr("PGUP"));
        knownAliases.insert("Delete", tr("DEL"));
        knownAliases.insert("Next", tr("PGDN"));
        knownAliases.insert("KP_1", tr("1"));
        knownAliases.insert("KP_2", tr("2"));
        knownAliases.insert("KP_3", tr("3"));
        knownAliases.insert("KP_4", tr("4"));
        knownAliases.insert("KP_5", tr("5"));
        knownAliases.insert("KP_6", tr("6"));
        knownAliases.insert("KP_7", tr("7"));
        knownAliases.insert("KP_8", tr("8"));
        knownAliases.insert("KP_9", tr("9"));
        knownAliases.insert("KP_0", tr("0"));
        knownAliases.insert("Num_Lock", tr("NUM\nLK"));
        knownAliases.insert("KP_Divide", tr("/"));
        knownAliases.insert("KP_Multiply", tr("*"));
        knownAliases.insert("KP_Subtract", tr("-"));
        knownAliases.insert("KP_Add", tr("+"));
        knownAliases.insert("KP_Enter", tr("E\nN\nT\nE\nR"));
        knownAliases.insert("KP_Decimal", tr("."));
        knownAliases.insert("asterisk", tr("*"));
        knownAliases.insert("less", tr("<"));
        knownAliases.insert("colon", tr(":"));
        knownAliases.insert("Super_L", tr("Super (L)"));
        knownAliases.insert("Menu", tr("Menu"));
    }
}

void VirtualKeyPushButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    QFontMetrics fm = this->fontMetrics();
    QFont tempWidgetFont = this->font();
    QFont tempScaledFont = painter.font();

    while ((this->width() < fm.boundingRect(this->rect(), Qt::AlignCenter, this->text()).width()) && tempScaledFont.pointSize() >= 5)
    {
        tempScaledFont.setPointSize(painter.font().pointSize()-1);
        painter.setFont(tempScaledFont);
        fm = painter.fontMetrics();
    }

    this->setFont(tempScaledFont);
    QPushButton::paintEvent(event);
    this->setFont(tempWidgetFont);
}
