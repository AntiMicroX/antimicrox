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

#include "logger.h"

#include <QDebug>
#include <QMetaObject>
#include <QTime>

Logger *Logger::instance = nullptr;

/**
 * @brief Outputs log messages to a given text stream. Client code
 *     should determine whether it points to a console stream or
 *     to a file.
 * @param Stream used to output text
 * @param Messages based of a given output level or lower will be logged
 * @param Parent object
 */
Logger::Logger(QTextStream *stream, LogLevel outputLevel, QObject *parent)
    : QObject(parent)
{
    // needed to allow sending LogLevel using signals and slots
    qRegisterMetaType<Logger::LogLevel>("Logger::LogLevel");
    loggingThread = new QThread(this);
    outputStream = stream;
    outputLevel = outputLevel;
    writeTime = false;

    this->moveToThread(loggingThread);
    loggingThread->start();
}

/**
 * @brief Close output stream and set instance to 0.
 */
Logger::~Logger()
{
    loggingThread->quit();
    loggingThread->wait();
    closeLogger();
}

/**
 * @brief Set the highest logging level. Determines which messages
 *     are output to the output stream.
 * @param Highest log level utilized.
 */
void Logger::setLogLevel(LogLevel level)
{
    Q_ASSERT(instance != nullptr);

    QMutexLocker locker(&instance->logMutex);
    Q_UNUSED(locker);

    instance->outputLevel = level;
}

/**
 * @brief Get the current output level associated with the logger.
 * @return Current output level
 */
Logger::LogLevel Logger::getCurrentLogLevel()
{
    Q_ASSERT(instance != nullptr);
    return instance->outputLevel;
}

void Logger::setCurrentStream(QTextStream *stream)
{
    Q_ASSERT(instance != nullptr);

    QMutexLocker locker(&instance->logMutex);
    Q_UNUSED(locker);

    instance->outputStream->flush();
    instance->outputStream = stream;
}

QTextStream *Logger::getCurrentStream()
{
    Q_ASSERT(instance != nullptr);

    return instance->outputStream;
}

/**
 * @brief Flushes output stream and closes stream if requested.
 * @param Whether to close the current stream. Defaults to true.
 */
void Logger::closeLogger(bool closeStream)
{
    if (outputStream != nullptr)
    {
        outputStream->flush();

        if (closeStream && (outputStream->device() != nullptr))
        {
            QIODevice *device = outputStream->device();
            if (device->isOpen())
            {
                device->close();
            }
        }
    }
}

/**
 * @brief Write an individual message to the text stream.
 */
void Logger::logMessage(const QString &message, const Logger::LogLevel level, const uint lineno, const QString &filename)
{
    if ((outputLevel != LOG_NONE) && (level <= outputLevel))
    {
        QString displayTime = "";
        QString initialPrefix = "";
        QString finalMessage = QString();
        if ((outputLevel > LOG_INFO) || writeTime)
        {
            displayTime = QString("[%1] - ").arg(QTime::currentTime().toString("hh:mm:ss.zzz"));
            initialPrefix = displayTime;
        }
        finalMessage.append(initialPrefix).append(message);

        *outputStream << finalMessage;
        outputStream->flush();
    }
    // TODO printing to file
}

/**
 * @brief Set whether the current time should be written with a message.
 *   This property is only used if outputLevel is set to LOG_INFO.
 * @param status
 */
void Logger::setWriteTime(bool status)
{
    QMutexLocker locker(&logMutex);
    Q_UNUSED(locker);

    writeTime = status;
}

/**
 * @brief Get whether the current time should be written with a LOG_INFO
 *   message.
 * @return Whether the current time is written with a LOG_INFO message
 */
bool Logger::getWriteTime()
{
    Q_ASSERT(instance != nullptr);

    return writeTime;
}

void Logger::setCurrentLogFile(QString filename)
{
    Q_ASSERT(instance != nullptr);

    if (instance->outputFile.isOpen())
    {
        instance->closeLogger(true);
    }
    instance->outputFile.setFileName(filename);
    instance->outputFile.open(QIODevice::WriteOnly | QIODevice::Append);
    instance->outFileStream.setDevice(&instance->outputFile);
    instance->setCurrentStream(&instance->outFileStream);
}

void Logger::loggerMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (Logger::instance != nullptr)
    {
        switch (type)
        {
        case QtDebugMsg:
            if (Logger::instance->getCurrentLogLevel() == Logger::LOG_DEBUG ||
                Logger::instance->getCurrentLogLevel() == Logger::LOG_MAX)
                LogHelper(LogLevel::LOG_DEBUG, context.line, context.file, msg).sendMessage();
            break;
        case QtInfoMsg:
            if (Logger::instance->getCurrentLogLevel() == Logger::LOG_INFO ||
                Logger::instance->getCurrentLogLevel() == Logger::LOG_MAX)
                LogHelper(LogLevel::LOG_INFO, context.line, context.file, msg).sendMessage();
            break;
        case QtWarningMsg:
            if (Logger::instance->getCurrentLogLevel() == Logger::LOG_WARNING ||
                Logger::instance->getCurrentLogLevel() == Logger::LOG_MAX)
                LogHelper(LogLevel::LOG_WARNING, context.line, context.file, msg).sendMessage();
            break;
        case QtCriticalMsg:
            if (Logger::instance->getCurrentLogLevel() == Logger::LOG_ERROR ||
                Logger::instance->getCurrentLogLevel() == Logger::LOG_MAX)
                LogHelper(LogLevel::LOG_ERROR, context.line, context.file, msg).sendMessage();
            break;
        case QtFatalMsg:
            if (Logger::instance->getCurrentLogLevel() == Logger::LOG_ERROR ||
                Logger::instance->getCurrentLogLevel() == Logger::LOG_MAX)
                LogHelper(LogLevel::LOG_ERROR, context.line, context.file, msg).sendMessage();
            abort();
        default:
            break;
        }
    }
}
