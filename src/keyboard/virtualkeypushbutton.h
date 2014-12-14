#ifndef VIRTUALKEYPUSHBUTTON_H
#define VIRTUALKEYPUSHBUTTON_H

#include <QPushButton>
#include <QString>
#include <QHash>

#include <joybutton.h>

class VirtualKeyPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit VirtualKeyPushButton(JoyButton *button, QString xcodestring, QWidget *parent = 0);
    int calculateFontSize();
    
protected:
    int keycode;
    unsigned int qkeyalias;
    QString xcodestring;
    QString displayString;
    bool currentlyActive;
    bool onCurrentButton;
    JoyButton *button;
    static QHash<QString, QString> knownAliases;

    QString setDisplayString(QString xcodestring);
    void populateKnownAliases();

signals:
    void keycodeObtained(int code, unsigned int alias);

public slots:

private slots:
    void processSingleSelection();
};

#endif // VIRTUALKEYPUSHBUTTON_H
