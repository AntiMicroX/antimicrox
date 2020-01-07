#pragma once
#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QtGlobal>
#include <QString>

namespace MessageHandler // prevents polluting the global namespace
{
   extern void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);
}

#endif // MESSAGEHANDLER_H
