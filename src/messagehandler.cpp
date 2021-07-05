/* antimicrox Gamepad to KB+M event mapper
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

#include "messagehandler.h"
#include "logger.h"

#include <QtGlobal>
#include <stdio.h>
#include <stdlib.h>

namespace MessageHandler {
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();

    if (Logger::instance != nullptr)
    {
        switch (type)
        {
        case QtDebugMsg:
            if (Logger::instance->getCurrentLogLevel() == Logger::LOG_DEBUG ||
                Logger::instance->getCurrentLogLevel() == Logger::LOG_MAX)
                fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line,
                        context.function);
            break;
        case QtInfoMsg:
            if (Logger::instance->getCurrentLogLevel() == Logger::LOG_INFO ||
                Logger::instance->getCurrentLogLevel() == Logger::LOG_MAX)
                fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line,
                        context.function);
            break;
        case QtWarningMsg:
            if (Logger::instance->getCurrentLogLevel() == Logger::LOG_WARNING ||
                Logger::instance->getCurrentLogLevel() == Logger::LOG_MAX)
                fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line,
                        context.function);
            break;
        case QtCriticalMsg:
            if (Logger::instance->getCurrentLogLevel() == Logger::LOG_ERROR ||
                Logger::instance->getCurrentLogLevel() == Logger::LOG_MAX)
                fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line,
                        context.function);
            break;
        case QtFatalMsg:
            if (Logger::instance->getCurrentLogLevel() == Logger::LOG_ERROR ||
                Logger::instance->getCurrentLogLevel() == Logger::LOG_MAX)
                fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line,
                        context.function);
            abort();
        default:
            break;
        }
    }
}
} // namespace MessageHandler
