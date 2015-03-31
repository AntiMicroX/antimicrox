#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>
#include <QTimer>

class Logger : public QObject
{
    Q_OBJECT
public:
    enum LogLevel
    {
        LOG_NONE = 0, LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG,
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
    static QTextStream* getCurrentStream();

    static void setCurrentErrorStream(QTextStream *stream);
    static QTextStream* getCurrentErrorStream();

    QTimer* getLogTimer();
    void stopLogTimer();

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
    void logMessage(LogMessage msg);

    QTextStream *outputStream;
    QTextStream *errorStream;
    LogLevel outputLevel;
    QMutex logMutex;
    QTimer pendingTimer;
    QList<LogMessage> pendingMessages;

    static Logger *instance;

signals:

public slots:
    void Log();
};

#endif // LOGGER_H
