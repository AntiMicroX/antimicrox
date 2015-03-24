#include <QTime>

#include "logger.h"

Logger* Logger::instance = 0;

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
    instance = this;
    instance->outputStream = stream;
    instance->outputLevel = outputLevel;
    instance->errorStream = 0;
}

Logger::Logger(QTextStream *stream, QTextStream *errorStream, LogLevel outputLevel, QObject *parent) :
    QObject(parent)
{
    instance = this;
    instance->outputStream = stream;
    instance->outputLevel = outputLevel;
    instance->errorStream = errorStream;
}

/**
 * @brief Close output stream and set instance to 0.
 */
Logger::~Logger()
{
    closeLogger();
}

/**
 * @brief Set the highest logging level. Determines which messages
 *     are output to the output stream.
 * @param Highest log level utilized.
 */
void Logger::setLogLevel(LogLevel level)
{
    Q_ASSERT(instance != 0);

    QMutexLocker locker(&instance->logMutex);
    Q_UNUSED(locker);

    instance->outputLevel = level;
}

Logger::LogLevel Logger::getCurrentLogLevel()
{
    Q_ASSERT(instance != 0);

    return instance->outputLevel;
}

void Logger::setCurrentStream(QTextStream *stream)
{
    Q_ASSERT(instance != 0);

    QMutexLocker locker(&instance->logMutex);
    Q_UNUSED(locker);

    instance->outputStream->flush();
    instance->outputStream = stream;
}

QTextStream* Logger::getCurrentStream()
{
    Q_ASSERT(instance != 0);

    return instance->outputStream;
}

void Logger::setCurrentErrorStream(QTextStream *stream)
{
    Q_ASSERT(instance != 0);

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
    Q_ASSERT(instance != 0);

    return instance->errorStream;
}

/**
 * @brief Check if message should be logged according to the set log level.
 *     Log the message to the output stream.
 * @param Log level
 * @param String to write to output stream if appropriate to the current
 *     log level.
 */
void Logger::Log(LogLevel level, const QString &message, bool newline)
{
    Q_ASSERT(instance != 0);

    if (instance->outputLevel != LOG_NONE && level <= instance->outputLevel)
    {
        QMutexLocker locker(&instance->logMutex);
        Q_UNUSED(locker);

        QString displayTime = "";
        QString initialPrefix = "";
        if (instance->outputLevel > LOG_INFO)
        {
            displayTime = QString("[%1] - ").arg(QTime::currentTime().toString("hh:mm:ss.zzz"));
            initialPrefix = displayTime;
        }

        QTextStream *writeStream = instance->outputStream;
        if (level > LOG_INFO && instance->errorStream)
        {
            writeStream = instance->errorStream;
        }

        *writeStream << initialPrefix << message;
        if (newline)
        {
            *writeStream << endl;
        }

        writeStream->flush();
    }
}

/**
 * @brief Flushes output stream and closes stream if requested.
 * @param Whether to close the current stream. Defaults to true.
 */
void Logger::closeLogger(bool closeStream)
{
    if (outputStream)
    {
        outputStream->flush();

        if (closeStream && outputStream->device() != 0)
        {
            QIODevice *device = outputStream->device();
            if (device->isOpen())
            {
                device->close();
            }
        }
    }

    if (errorStream)
    {
        errorStream->flush();

        if (closeStream && errorStream->device() != 0)
        {
            QIODevice *device = errorStream->device();
            if (device->isOpen())
            {
                device->close();
            }
        }
    }

    instance = 0;
}
