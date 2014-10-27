#ifndef JOYBUTTONMOUSEHELPER_H
#define JOYBUTTONMOUSEHELPER_H

#include <QObject>

class JoyButtonMouseHelper : public QObject
{
    Q_OBJECT
public:
    explicit JoyButtonMouseHelper(QObject *parent = 0);

signals:
    void mouseCursorMoved(int mouseX, int mouseY, int elapsed);
    void mouseSpringMoved(int mouseX, int mouseY);

public slots:
    void moveMouseCursor();
    void moveSpringMouse();
};

#endif // JOYBUTTONMOUSEHELPER_H
