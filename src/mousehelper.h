#ifndef MOUSEHELPER_H
#define MOUSEHELPER_H

#include <QObject>
#include <QTimer>

class MouseHelper : public QObject
{
    Q_OBJECT
public:
    explicit MouseHelper(QObject *parent = 0);

    bool springMouseMoving;
    int previousCursorLocation[2];
    QTimer mouseTimer;
    
signals:
    
private slots:
    void resetSpringMouseMoving();
};

#endif // MOUSEHELPER_H
