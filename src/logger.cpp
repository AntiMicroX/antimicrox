/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
 * Copyright (C) 2021 Paweł Kotiuk
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

// only for QT 6
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #include <QStringConverter>
#endif
#include <chrono>
#include <thread>

Logger *Logger::instance = nullptr;

/**
 * @brief Outputs log messages to a given text stream. Client code
 *     should determine whether it points to a console stream or
 *     to a file.
 * @param stream used to output text
 * @param output_lvl Messages based of a given output level or lower will be logged
 * @param parent object
 */
Logger::Logger(QTextStream *stream, LogLevel output_lvl, QObject *parent)
    : QObject(parent)
{
    // needed to allow sending LogLevel using signals and slots
    qRegisterMetaType<Logger::LogLevel>("Logger::LogLevel");
    loggingThread = new QThread(this);
    loggingThread->setObjectName("loggingThread");
    outputStream = stream;
    outputLevel = output_lvl;

    this->moveToThread(loggingThread);
    loggingThread->start();
}

/**
 * @brief Close output stream and set instance to 0.
 */
Logger::~Logger()
{
    VERBOSE() << "Closing logger";
    // To be sure about proper processing logs before deleting logger
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    loggingThread->quit();
    loggingThread->wait();
    closeLogger();
    instance = nullptr;
}

/**
 * @brief Set the highest logging level. Determines which messages
 *     are output to the output stream.
 * @param level Highest log level utilized.
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
 * @param closeStream Whether to close the current stream. Defaults to true.
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
 *
 * This socket method is executed in separate logging thread
 */
void Logger::logMessage(const QString &message, const Logger::LogLevel level, const uint lineno, const QString &filename)
{
    const static QMap<Logger::LogLevel, QString> TYPE_NAMES = {
        {LogLevel::LOG_DEBUG, "🐞DEBUG"},  {LogLevel::LOG_VERBOSE, "⚪VERBOSE"}, {LogLevel::LOG_INFO, "🟢INFO"},
        {LogLevel::LOG_WARNING, "❗WARN"}, {LogLevel::LOG_ERROR, "❌ERROR"},     {LogLevel::LOG_NONE, "NONE"}};
    QString displayTime = QString("[%1] ").arg(QTime::currentTime().toString("hh:mm:ss.zzz"));
    if ((outputLevel != LOG_NONE) && (level <= outputLevel))
    {
        bool extendedLogs = (outputLevel == LOG_DEBUG);
        if (extendedLogs)
            *outputStream << displayTime;

        QString finalMessage = message;
        finalMessage = finalMessage.replace("\n", "\n\t\t\t");
        *outputStream << TYPE_NAMES[level] << "\t" << finalMessage;

        if (extendedLogs)
        {
            static int filename_offset = -1;
            if (filename_offset < 0)
            {
                filename_offset = filename.lastIndexOf("/src/");
            }
            if (lineno != 0)
                *outputStream << " (file " << filename.mid(filename_offset) << ":" << lineno << ")";
        }

        *outputStream << "\n";
        outputStream->flush();
    }
}

void Logger::setCurrentLogFile(QString filename)
{
    if (filename.isEmpty())
        return;
    Q_ASSERT(instance != nullptr);

    if (instance->outputFile.isOpen())
    {
        instance->closeLogger(true);
    }
    instance->outputFile.setFileName(filename);
    if (!instance->outputFile.open(QIODevice::WriteOnly))
    {
        qCritical() << "Couldn't open log file: " << filename;
        return;
    }
    instance->outFileStream.setDevice(&instance->outputFile);
#if defined(Q_OS_WIN)
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    instance->outFileStream.setEncoding(QStringConverter::Utf8);
    #else
    instance->outFileStream.setCodec("UTF-8"); // to properly print special characters in files
    #endif
#endif
    instance->setCurrentStream(&instance->outFileStream);
}

bool Logger::isWritingToFile() { return outputFile.isOpen(); }

/**
 * @brief log message handling function
 *
 * It is meant to be registered via qInstallMessageHandler() at the beginning of application
 *
 * @param type
 * @param context
 * @param msg
 */
void Logger::loggerMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (Logger::instance != nullptr)
    {
        Logger::LogLevel level = Logger::instance->getCurrentLogLevel();
        if (level == LogLevel::LOG_NONE)
            return;
        switch (type)
        {
        case QtDebugMsg:
            if (level >= Logger::LOG_DEBUG || level == Logger::LOG_MAX)
                LogHelper(LogLevel::LOG_DEBUG, context.line, context.file, msg);
            break;
        case QtInfoMsg:
            if (level >= Logger::LOG_INFO)
                LogHelper(LogLevel::LOG_INFO, context.line, context.file, msg);
            break;
        case QtWarningMsg:
            if (level >= Logger::LOG_WARNING)
                LogHelper(LogLevel::LOG_WARNING, context.line, context.file, msg);
            break;
        case QtCriticalMsg:
            if (level >= Logger::LOG_ERROR)
                LogHelper(LogLevel::LOG_ERROR, context.line, context.file, msg);
            break;
        case QtFatalMsg:
            if (level >= Logger::LOG_ERROR)
                LogHelper(LogLevel::LOG_ERROR, context.line, context.file, msg);
            abort();
        default:
            break;
        }
    }
}

/**
 * @brief Create instance of logger, if there is any other instance it will de deleted
 *
 * @return Logger* - pointer to newly created instance
 */
Logger *Logger::createInstance(QTextStream *stream, LogLevel outputLevel, QObject *parent)
{
    if (instance != nullptr)
    {
        delete instance;
    }
    instance = new Logger(stream, outputLevel, parent);
    return instance;
}

bool Logger::isDebugEnabled()
{
    if (instance != nullptr)
    {
        return instance->outputLevel == LogLevel::LOG_DEBUG;
    }
    return false;
}

QString Logger::getCurrentLogFile()
{
    Q_ASSERT(instance != nullptr);
    if (instance->outputFile.exists())
        return instance->outputFile.fileName();
    else
        return "";
}