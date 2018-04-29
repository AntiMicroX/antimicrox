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


#include "logger.h"

#include "messagehandler.h"

#include <QTime>
#include <QDebug>

Logger* Logger::instance = nullptr;

/**
 * @brief Outputs log messages to a given text stream. Client code
 *     should determine whether it points to a console stream or
 *     to a file.
 * @param Stream used to output text
 * @param Messages based of a given output level or lower will be logged
 * @param Parent object
 */
Logger::Logger(QTextStream *stream, LogLevel outputLevel, QObject *parent) :
    QObject(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    instance = this;
    instance->outputStream = stream;
    instance->outputLevel = outputLevel;
    instance->errorStream = nullptr;
    instance->pendingTimer.setInterval(1);
    instance->pendingTimer.setSingleShot(true);
    instance->writeTime = false;

    connect(instance, SIGNAL(pendingMessage()), instance, SLOT(startPendingTimer()));
    connect(&(instance->pendingTimer), SIGNAL(timeout()), instance, SLOT(Log()));
}

/**
 * @brief Outputs log messages to a given text stream. Client code
 *     should determine whether it points to a console stream or
 *     to a file.
 * @param Stream used to output standard text
 * @param Stream used to output error text
 * @param Messages based of a given output level or lower will be logged
 * @param Parent object
 */
Logger::Logger(QTextStream *stream, QTextStream *errorStream,
               LogLevel outputLevel, QObject *parent) :
    QObject(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    instance = this;
    instance->outputStream = stream;
    instance->outputLevel = outputLevel;
    instance->errorStream = errorStream;
    instance->pendingTimer.setInterval(1);
    instance->pendingTimer.setSingleShot(true);
    instance->writeTime = false;

    connect(instance, SIGNAL(pendingMessage()), instance, SLOT(startPendingTimer()));
    connect(&(instance->pendingTimer), SIGNAL(timeout()), instance, SLOT(Log()));
}

/**
 * @brief Close output stream and set instance to 0.
 */
Logger::~Logger()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    closeLogger();
    closeErrorLogger();
}

/**
 * @brief Set the highest logging level. Determines which messages
 *     are output to the output stream.
 * @param Highest log level utilized.
 */
void Logger::setLogLevel(LogLevel level)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_ASSERT(instance != nullptr);

    return instance->outputLevel;
}

void Logger::setCurrentStream(QTextStream *stream)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_ASSERT(instance != nullptr);

    QMutexLocker locker(&instance->logMutex);
    Q_UNUSED(locker);

    instance->outputStream->flush();
    instance->outputStream = stream;
}

QTextStream* Logger::getCurrentStream()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_ASSERT(instance != nullptr);

    return instance->outputStream;
}

void Logger::setCurrentErrorStream(QTextStream *stream)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_ASSERT(instance != nullptr);

    QMutexLocker locker(&instance->logMutex);
    Q_UNUSED(locker);

    if (instance->errorStream)
    {
        instance->errorStream->flush();
    }

    instance->errorStream = stream;
}

QTextStream* Logger::getCurrentErrorStream()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_ASSERT(instance != nullptr);

    return instance->errorStream;
}

/**
 * @brief Go through a list of pending messages and check if message should be
 *     logged according to the set log level. Log the message to the output
 *     stream.
 * @param Log level
 * @param String to write to output stream if appropriate to the current
 *     log level.
 */
void Logger::Log()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QMutexLocker locker(&logMutex);
    Q_UNUSED(locker);

    QListIterator<LogMessage> iter(getPendingMessages());
    while (iter.hasNext())
    {
        LogMessage pendingMessage = iter.next();
        logMessage(pendingMessage);
    }

    pendingMessages.clear();
    instance->pendingTimer.stop();
}

/**
 * @brief Flushes output stream and closes stream if requested.
 * @param Whether to close the current stream. Defaults to true.
 */
void Logger::closeLogger(bool closeStream)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
 * @brief Flushes output stream and closes stream if requested.
 * @param Whether to close the current stream. Defaults to true.
 */
void Logger::closeErrorLogger(bool closeStream)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (errorStream != nullptr)
    {
        errorStream->flush();

        if (closeStream && (errorStream->device() != nullptr))
        {
            QIODevice *device = errorStream->device();
            if (device->isOpen())
            {
                device->close();
            }
        }
    }

    instance->pendingTimer.stop();
    instance = nullptr;
}

/**
 * @brief Append message to list of messages that might get placed in the
 *     log. Messages will be written later.
 * @param Log level
 * @param String to write to output stream if appropriate to the current
 *     log level.
 * @param Whether the logger should add a newline to the end of the message.
 */
void Logger::appendLog(LogLevel level, const QString &message, bool newline)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_ASSERT(instance != nullptr);

    QMutexLocker locker(&instance->logMutex);
    Q_UNUSED(locker);

    LogMessage temp;
    temp.level = level;
    temp.message = QString(message);
    temp.newline = newline;

    instance->pendingMessages.append(temp);

    emit instance->pendingMessage();
}

/**
 * @brief Immediately write a message to a text stream.
 * @param Log level
 * @param String to write to output stream if appropriate to the current
 *   log level.
 * @param Whether the logger should add a newline to the end of the message.
 */
void Logger::directLog(LogLevel level, const QString &message, bool newline)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_ASSERT(instance != nullptr);

    QMutexLocker locker(&instance->logMutex);
    Q_UNUSED(locker);

    LogMessage temp;
    temp.level = level;
    temp.message = QString(message);
    temp.newline = newline;

    instance->logMessage(temp);
}

/**
 * @brief Write an individual message to the text stream.
 * @param LogMessage instance for a single message
 */
void Logger::logMessage(LogMessage msg)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    LogLevel level = msg.level;
    QString message = msg.message;
    bool newline = msg.newline;

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

        QTextStream *writeStream = outputStream;
        if ((level < LOG_INFO) && (errorStream != nullptr))
        {
            writeStream = errorStream;
        }

        finalMessage.append(initialPrefix).append(message);

        if (newline)
        {
            finalMessage.append("\n");
        }

        *writeStream << finalMessage;
        writeStream->flush();
        emit stringWritten(finalMessage);
    }
}

/**
 * @brief Get the associated timer used by the logger.
 * @return QTimer instance
 */
QTimer* Logger::getLogTimer()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return &pendingTimer;
}

/**
 * @brief Stop the logger's timer if it is currently active.
 */
void Logger::stopLogTimer()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (pendingTimer.isActive())
    {
        pendingTimer.stop();
    }
}

/**
 * @brief Set whether the current time should be written with a message.
 *   This property is only used if outputLevel is set to LOG_INFO.
 * @param status
 */
void Logger::setWriteTime(bool status)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_ASSERT(instance != nullptr);

    QMutexLocker locker(&instance->logMutex);
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_ASSERT(instance != nullptr);

    return writeTime;
}

void Logger::startPendingTimer()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_ASSERT(instance != nullptr);

    if (!instance->pendingTimer.isActive())
    {
        instance->pendingTimer.start();
    }
}

void Logger::setCurrentLogFile(QString filename) {

    qInstallMessageHandler(MessageHandler::myMessageOutput);

  Q_ASSERT(instance != nullptr);
  
  if( instance->outputFile.isOpen() ) {
    instance->closeLogger(true);
  }
  instance->outputFile.setFileName( filename );
  instance->outputFile.open( QIODevice::WriteOnly | QIODevice::Append );
  instance->outFileStream.setDevice( &instance->outputFile );
  instance->setCurrentStream( &instance->outFileStream );
  instance->LogInfo(QObject::trUtf8("Logging started"), true, true);
}

void Logger::setCurrentErrorLogFile(QString filename) {

    qInstallMessageHandler(MessageHandler::myMessageOutput);

  Q_ASSERT(instance != nullptr);

  if( instance->errorFile.isOpen() ) {
    instance->closeErrorLogger(true);
  }
  instance->errorFile.setFileName( filename );
  instance->errorFile.open( QIODevice::WriteOnly | QIODevice::Append );
  instance->outErrorFileStream.setDevice( &instance->errorFile );
  instance->setCurrentErrorStream( &instance->outErrorFileStream );
}

QList<Logger::LogMessage> const& Logger::getPendingMessages() {

    return pendingMessages;
}

