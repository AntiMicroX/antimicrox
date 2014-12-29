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
}

/**
 * @brief Close output stream and set instance to 0.
 */
Logger::~Logger()
{
    outputStream->flush();

    if (outputStream->device() != 0)
    {
        QIODevice *device = outputStream->device();
        if (device->isOpen())
        {
            device->close();
        }
    }

    instance = 0;
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

/**
 * @brief Check if message should be logged according to the set log level.
 *     Log the message to the output stream.
 * @param Log level
 * @param String to write to output stream if appropriate to the current
 *     log level.
 */
void Logger::Log(LogLevel level, const QString &message)
{
    Q_ASSERT(instance != 0);

    if (instance->outputLevel != LOG_NONE && level >= instance->outputLevel)
    {
        QMutexLocker locker(&instance->logMutex);
        Q_UNUSED(locker);

        QString displayTime = QTime::currentTime().toString("hh:mm:ss.zzz");
        *instance->outputStream << displayTime << " - " << message << endl;
        instance->outputStream->flush();
    }
}
