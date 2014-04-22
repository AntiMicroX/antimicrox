#ifndef LOCALANTIMICROSERVER_H
#define LOCALANTIMICROSERVER_H

#include <QObject>
#include <QLocalServer>

class LocalAntiMicroServer : public QObject
{
    Q_OBJECT
public:
    explicit LocalAntiMicroServer(QObject *parent = 0);

protected:
    QLocalServer *localServer;

signals:
    void clientdisconnect();

public slots:
    void startLocalServer();
    void handleOutsideConnection();
    void handleSocketDisconnect();
};

#endif // LOCALANTIMICROSERVER_H
