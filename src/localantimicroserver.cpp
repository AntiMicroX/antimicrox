/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
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

#include "localantimicroserver.h"

#include "common.h"

#include <QDebug>
#include <QLocalServer>
#include <QLocalSocket>

LocalAntiMicroServer::LocalAntiMicroServer(QObject *parent)
    : QObject(parent)
{
    localServer = new QLocalServer(this);
}

void LocalAntiMicroServer::startLocalServer()
{
    if (localServer != nullptr)
    {
        bool removedServer = QLocalServer::removeServer(PadderCommon::localSocketKey);

        if (!removedServer)
            qDebug() << "Couldn't remove local server named " << PadderCommon::localSocketKey;

        if (localServer->maxPendingConnections() != 1)
            localServer->setMaxPendingConnections(1);

        if (!localServer->isListening())
        {
            if (!localServer->listen(PadderCommon::localSocketKey))
            {
                QString message("Could not start signal server. Profiles cannot be reloaded\n");
                message.append("from command-line");
                PRINT_STDERR() << tr(message.toStdString().c_str()) << "\n";
                qDebug() << "Could not start signal server. Profiles cannot be reloaded\n"
                         << " \nfrom command-line\n " << tr(message.toStdString().c_str());
            } else
            {
                connect(localServer, &QLocalServer::newConnection, this, &LocalAntiMicroServer::handleOutsideConnection);
            }
        }
    } else
    {
        qDebug() << "LocalAntiMicroXServer::startLocalServer(): localServer is nullptr";
    }
}

void LocalAntiMicroServer::handleOutsideConnection()
{
    if (localServer != nullptr)
    {
        QLocalSocket *socket = localServer->nextPendingConnection();

        if (socket != nullptr)
        {
            qDebug() << "There is next pending connection: " << socket->socketDescriptor();
            connect(socket, &QLocalSocket::disconnected, this, &LocalAntiMicroServer::handleSocketDisconnect);
            connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
        } else
        {
            qDebug() << "There isn't next pending connection: ";
        }
    } else
    {
        qDebug() << "LocalAntiMicroXServer::handleOutsideConnection(): localServer is nullptr";
    }
}

void LocalAntiMicroServer::handleSocketDisconnect() { emit clientdisconnect(); }

void LocalAntiMicroServer::close() { localServer->close(); }

QLocalServer *LocalAntiMicroServer::getLocalServer() const { return localServer; }
