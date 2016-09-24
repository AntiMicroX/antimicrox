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

#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>
#include <QTimer>
#include <QFile>

class Logger : public QObject
{
    Q_OBJECT
public:
    enum LogLevel
    {
        LOG_NONE = 0, LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG,
	LOG_MAX = LOG_DEBUG
    };

    typedef struct {
        LogLevel level;
        QString message;
        bool newline;
    } LogMessage;

    explicit Logger(QTextStream *stream, LogLevel outputLevel = LOG_INFO, QObject *parent = 0);
    explicit Logger(QTextStream *stream, QTextStream *errorStream, LogLevel outputLevel = LOG_INFO, QObject *parent = 0);
    ~Logger();

    static void setLogLevel(LogLevel level);
    LogLevel getCurrentLogLevel();

    static void setCurrentStream(QTextStream *stream);
    static void setCurrentLogFile(QString filename);
    static QTextStream* getCurrentStream();

    static void setCurrentErrorStream(QTextStream *stream);
    static void setCurrentErrorLogFile(QString filename);
    static QTextStream* getCurrentErrorStream();

    QTimer* getLogTimer();
    void stopLogTimer();

    bool getWriteTime();
    void setWriteTime(bool status);

    static void appendLog(LogLevel level, const QString &message, bool newline=true);
    static void directLog(LogLevel level, const QString &message, bool newline=true);

    // Some convenience functions that will hopefully speed up
    // logging operations.
    inline static void LogInfo(const QString &message, bool newline=true, bool direct=false)
    {
        if (!direct)
        {
            appendLog(LOG_INFO, message, newline);
        }
        else
        {
            directLog(LOG_INFO, message, newline);
        }
        //Log(LOG_INFO, message, newline);
    }

    inline static void LogDebug(const QString &message, bool newline=true, bool direct=false)
    {
        if (!direct)
        {
            appendLog(LOG_DEBUG, message, newline);
        }
        else
        {
            directLog(LOG_DEBUG, message, newline);
        }

        //Log(LOG_DEBUG, message, newline);
    }

    inline static void LogWarning(const QString &message, bool newline=true, bool direct=false)
    {
        if (!direct)
        {
            appendLog(LOG_WARNING, message, newline);
        }
        else
        {
            directLog(LOG_WARNING, message, newline);
        }
        //Log(LOG_WARNING, message, newline);
    }

    inline static void LogError(const QString &message, bool newline=true, bool direct=false)
    {
        if (!direct)
        {
            appendLog(LOG_ERROR, message, newline);
        }
        else
        {
            directLog(LOG_ERROR, message, newline);
        }
        //Log(LOG_ERROR, message, newline);
    }

    inline static Logger* getInstance()
    {
        Q_ASSERT(instance != NULL);
        return instance;
    }

protected:
    void closeLogger(bool closeStream=true);
    void closeErrorLogger(bool closeStream=true);
    void logMessage(LogMessage msg);

    QFile outputFile;
    QTextStream outFileStream;
    QTextStream *outputStream;
    
    QFile errorFile;
    QTextStream outErrorFileStream;
    QTextStream *errorStream;
    LogLevel outputLevel;
    QMutex logMutex;
    QTimer pendingTimer;
    QList<LogMessage> pendingMessages;
    bool writeTime;

    static Logger *instance;

signals:
    void stringWritten(QString text);
    void pendingMessage();

public slots:
    void Log();
    void startPendingTimer();
};

#endif // LOGGER_H
