#include <QDebug>

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

    this->setText(this->displayString);

    connect(this, SIGNAL(clicked()), this, SLOT(processSingleSelection()));
}

void VirtualKeyPushButton::processSingleSelection()
{
    emit keycodeObtained(keycode);
}

QString VirtualKeyPushButton::setDisplayString(QString xcodestring)
{
    QString temp = xcodestring;
    if (knownAliases.contains(xcodestring))
    {
        temp = knownAliases.value(xcodestring);
    }

    return temp.toUpper();
}

void VirtualKeyPushButton::populateKnownAliases()
{
    if (knownAliases.isEmpty())
    {
        knownAliases.insert("space", "Space");
        knownAliases.insert("Tab", "Tab");
        knownAliases.insert("Shift_L", "Shift (L)");
        knownAliases.insert("Shift_R", "Shift (R)");
        knownAliases.insert("Control_L", "Ctrl (L)");
        knownAliases.insert("Control_R", "Ctrl (R)");
        knownAliases.insert("Alt_L", "Alt (L)");
        knownAliases.insert("Alt_R", "Alt (R)");
        knownAliases.insert("grave", "`");
        knownAliases.insert("asciitilde", "~");
        knownAliases.insert("minus", "-");
        knownAliases.insert("equal", "=");
        knownAliases.insert("bracketleft", "[");
        knownAliases.insert("bracketright", "]");
        knownAliases.insert("backslash", "\\");
        knownAliases.insert("Caps_Lock", "Caps");
        knownAliases.insert("semicolon", ";");
        knownAliases.insert("apostrophe", "'");
        knownAliases.insert("comma", ",");
        knownAliases.insert("period", ".");
        knownAliases.insert("slash", "/");
        knownAliases.insert("Escape", "ESC");
        knownAliases.insert("Print", "PRTSC");
        knownAliases.insert("Scroll_Lock", "SCLK");
        knownAliases.insert("Insert", "INS");
        knownAliases.insert("Prior", "PGUP");
        knownAliases.insert("Delete", "DEL");
        knownAliases.insert("Next", "PGDN");
        knownAliases.insert("KP_1", "1");
        knownAliases.insert("KP_2", "2");
        knownAliases.insert("KP_3", "3");
        knownAliases.insert("KP_4", "4");
        knownAliases.insert("KP_5", "5");
        knownAliases.insert("KP_6", "6");
        knownAliases.insert("KP_7", "7");
        knownAliases.insert("KP_8", "8");
        knownAliases.insert("KP_9", "9");
        knownAliases.insert("KP_0", "0");
        knownAliases.insert("Num_Lock", "NUM\nLK");
        knownAliases.insert("KP_Divide", "/");
        knownAliases.insert("KP_Multiply", "*");
        knownAliases.insert("KP_Subtract", "-");
        knownAliases.insert("KP_Add", "+");
        knownAliases.insert("KP_Enter", "E\nN\nT\nE\nR");
        knownAliases.insert("KP_Decimal", ".");
    }
}
