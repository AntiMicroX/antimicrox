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
    
protected:
    int keycode;
    QString xcodestring;
    QString displayString;
    bool currentlyActive;
    bool onCurrentButton;
    JoyButton *button;
    static QHash<QString, QString> knownAliases;

    QString setDisplayString(QString xcodestring);
    void populateKnownAliases();
    virtual void paintEvent(QPaintEvent *event);

signals:
    void keycodeObtained(int code);

public slots:

private slots:
    void processSingleSelection();
};

#endif // VIRTUALKEYPUSHBUTTON_H
