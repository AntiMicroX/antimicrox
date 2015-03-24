#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>

class Logger : public QObject
{
    Q_OBJECT
public:
    enum LogLevel
    {
        LOG_NONE = 0, LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG,
    };

    explicit Logger(QTextStream *stream, LogLevel outputLevel = LOG_INFO, QObject *parent = 0);
    explicit Logger(QTextStream *stream, QTextStream *errorStream, LogLevel outputLevel = LOG_INFO, QObject *parent = 0);
    ~Logger();

    static void setLogLevel(LogLevel level);
    LogLevel getCurrentLogLevel();

    static void setCurrentStream(QTextStream *stream);
    static QTextStream* getCurrentStream();

    static void setCurrentErrorStream(QTextStream *stream);
    static QTextStream* getCurrentErrorStream();

    static void Log(LogLevel level, const QString &message, bool newline=true);

    // Some convenience functions that will hopefully speed up
    // logging operations.
    inline static void LogInfo(const QString &message, bool newline=true)
    {
        Log(LOG_INFO, message, newline);
    }

    inline static void LogDebug(const QString &message, bool newline=true)
    {
        Log(LOG_DEBUG, message, newline);
    }

    inline static void LogWarning(const QString &message, bool newline=true)
    {
        Log(LOG_WARNING, message, newline);
    }

    inline static void LogError(const QString &message, bool newline=true)
    {
        Log(LOG_ERROR, message, newline);
    }

    inline static Logger* GetInstance()
    {
        Q_ASSERT(instance != NULL);
        return instance;
    }

protected:
    void closeLogger(bool closeStream=true);

    QTextStream *outputStream;
    QTextStream *errorStream;
    LogLevel outputLevel;
    QMutex logMutex;
    static Logger *instance;

signals:

public slots:

};

#endif // LOGGER_H
