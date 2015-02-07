#include <QTextStream>
#include <QLocalSocket>

#include "localantimicroserver.h"
#include "common.h"

LocalAntiMicroServer::LocalAntiMicroServer(QObject *parent) :
    QObject(parent)
{
    localServer = new QLocalServer(this);
}

void LocalAntiMicroServer::startLocalServer()
{
    QLocalServer::removeServer(PadderCommon::localSocketKey);
    localServer->setMaxPendingConnections(1);
    if (!localServer->listen(PadderCommon::localSocketKey))
    {
        QTextStream errorstream(stderr);
        QString message("Could not start signal server. Profiles cannot be reloaded\n");
        message.append("from command-line");
        errorstream << tr(message.toStdString().c_str()) << endl;
    }
    else
    {
        connect(localServer, SIGNAL(newConnection()), this, SLOT(handleOutsideConnection()));
    }
}

void LocalAntiMicroServer::handleOutsideConnection()
{
    QLocalSocket *socket = localServer->nextPendingConnection();
    if (socket)
    {
        connect(socket, SIGNAL(disconnected()), this, SLOT(handleSocketDisconnect()));
        connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
    }
}

void LocalAntiMicroServer::handleSocketDisconnect()
{
    emit clientdisconnect();
}

void LocalAntiMicroServer::close()
{
    localServer->close();
}
