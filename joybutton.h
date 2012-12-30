#ifndef JOYBUTTON_H
#define JOYBUTTON_H

#include <QObject>
#include <QTimer>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class JoyButton : public QObject
{
    Q_OBJECT
public:
    explicit JoyButton(QObject *parent = 0);
    explicit JoyButton(int index, QObject *parent=0);
    ~JoyButton();

    void joyEvent (bool pressed);
    int getJoyNumber ();
    virtual int getRealJoyNumber ();
    void setJoyNumber (int index);
    void setKey(int keycode);
    int getKey();
    void setMouse(int mouse);
    int getMouse();
    bool getToggleState();
    int getTurboInterval();
    void reset();
    void reset(int index);
    void setUseMouse(bool useMouse);
    bool isUsingMouse();
    void setUseTurbo(bool useTurbo);
    bool isUsingTurbo();
    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

    virtual QString getPartialName();
    virtual QString getName();
    virtual QString getXmlName();

    static const QString xmlName;

protected:
    void createDeskEvent();

    bool isButtonPressed;
    bool isKeyPressed;
    bool toggle;
    int index;
    int turboInterval;
    int keycode;
    int mousecode;
    QTimer *timer;
    bool isDown;
    bool useMouse;
    bool useTurbo;

signals:
    void clicked (int index);
    void released (int index);
    void keyChanged(int keycode);
    void mouseChanged(int mousecode);

public slots:
    void setTurboInterval (int interval);
    void setToggle (bool toggle);

private slots:
    void turboEvent();
};


#endif // JOYBUTTON_H
