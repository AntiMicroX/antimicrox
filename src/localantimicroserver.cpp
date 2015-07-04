/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
