/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
 * Copyright (C) 2020 Paweł Kotiuk
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

#ifndef LOGGER_H
#define LOGGER_H

#include <QFile>
#include <QMutex>
#include <QObject>
#include <QTextStream>
#include <QThread>

/**
 * @brief Custom singleton class used for logging across application.
 *
 * It manages log-levels, formatting, printing logs and saving them to file.
 * Logs across the program can be written using  qDebug(), qInfo(), qWarning(), qCritical, and qFatal() functions
 *
 */
class Logger : public QObject
{
    Q_OBJECT
    Q_ENUMS(LogLevel);

  public:
    enum LogLevel
    {
        LOG_NONE = 0,
        LOG_ERROR,
        LOG_WARNING,
        LOG_INFO,
        LOG_DEBUG,
        LOG_MAX = LOG_DEBUG
    };

    ~Logger();

    /**
     * @brief log message handling function
     *
     * It is meant to be registered via qInstallMessageHandler() at the beginning of application
     *
     * @param type
     * @param context
     * @param msg
     */
    static void loggerMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    static void setLogLevel(LogLevel level);
    LogLevel getCurrentLogLevel();

    static void setCurrentStream(QTextStream *stream);
    static void setCurrentLogFile(QString filename);
    static QTextStream *getCurrentStream();

    /**
     * @brief Get the Instance of logger if instance doesn't exist, then create a new one using passed arguments
     *
     * @return Logger*
     */
    inline static Logger *getInstance(QTextStream *stream = nullptr, LogLevel outputLevel = LOG_INFO,
                                      QObject *parent = nullptr)
    {
        if (instance == nullptr)
        {
            instance = new Logger(stream, outputLevel, parent);
        }

        return instance;
    }

    static Logger *instance;

  protected:
    explicit Logger(QTextStream *stream, LogLevel outputLevel = LOG_INFO, QObject *parent = nullptr);
    void closeLogger(bool closeStream = true);

    QFile outputFile;
    QTextStream outFileStream;
    QTextStream *outputStream;

    LogLevel outputLevel;
    QMutex logMutex;
    QThread *loggingThread; // in this thread all of writing operations will be executed

  public slots:
    /**
     * @brief Write an individual message to the text stream.
     *
     * This socket method is executed in separate logging thread
     */
    void logMessage(const QString &message, const Logger::LogLevel level, const uint lineno, const QString &filename);
};

/**
 * @brief simple helper class used for constructing log message and sending it to Logger
 *
 */
class LogHelper : public QObject
{
    Q_OBJECT
  public:
    QString message;
    Logger::LogLevel level;
    uint lineno;
    QString filename;

    LogHelper(const Logger::LogLevel level, const uint lineno, const QString &filename, const QString &message = "")
        : message(message)
        , level(level)
        , lineno(lineno)
        , filename(filename)
    {
        Logger *pointer = Logger::getInstance();
        connect(this, &LogHelper::logMessage, pointer, &Logger::logMessage);
    };

    void sendMessage() { emit logMessage(message, level, lineno, filename); };

  signals:
    void logMessage(const QString &message, const Logger::LogLevel level, const uint lineno, const QString &filename);
};

#endif // LOGGER_H
