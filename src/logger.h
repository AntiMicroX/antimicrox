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
#include <QTimer>

/**
 * @brief Custom class used for logging across application.
 *
 * It manages log-levels, formatting, printing logs and saving them to file.
 * Logs across the program can be written using  qDebug(), qInfo(), qWarning(), qCritical, and qFatal() functions
 *
 */
class Logger : public QObject
{
    Q_OBJECT

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

    typedef struct
    {
        QString message;
        LogLevel level;
        bool newline;
    } LogMessage;

    explicit Logger(QTextStream *stream, LogLevel outputLevel = LOG_INFO, QObject *parent = nullptr);
    explicit Logger(QTextStream *stream, QTextStream *errorStream, LogLevel outputLevel = LOG_INFO,
                    QObject *parent = nullptr);
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
    QList<LogMessage> const &getPendingMessages();

    static void setCurrentStream(QTextStream *stream);
    static void setCurrentLogFile(QString filename);
    static QTextStream *getCurrentStream();

    static void setCurrentErrorStream(QTextStream *stream);
    static void setCurrentErrorLogFile(QString filename);
    static QTextStream *getCurrentErrorStream();

    QTimer *getLogTimer();
    void stopLogTimer();

    bool getWriteTime();
    void setWriteTime(bool status);

    static void appendLog(LogLevel level, const QString &message, bool newline = true);
    static void directLog(LogLevel level, const QString &message, bool newline = true);

    inline static Logger *getInstance()
    {
        Q_ASSERT(instance != nullptr);
        return instance;
    }

    static Logger *instance;

  protected:
    void closeLogger(bool closeStream = true);
    void closeErrorLogger(bool closeStream = true);
    void logMessage(LogMessage msg);

    bool writeTime;

    QFile outputFile;
    QFile errorFile;

    QTextStream outFileStream;
    QTextStream *outputStream;
    QTextStream outErrorFileStream;
    QTextStream *errorStream;

    LogLevel outputLevel;
    QMutex logMutex;
    QTimer pendingTimer;

    QList<LogMessage> pendingMessages;

  signals:
    void stringWritten(QString text);
    void pendingMessage();

  public slots:
    void Log();
    void startPendingTimer();
};

#endif // LOGGER_H
