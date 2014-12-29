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
    enum LogLevel {
        LOG_NONE = 0, LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR,
    };

    explicit Logger(QTextStream *stream, LogLevel outputLevel = LOG_INFO, QObject *parent = 0);
    ~Logger();

    static void setLogLevel(LogLevel level);
    static void Log(LogLevel level, const QString &message);

    // Some convenience functions that will hopefully speed up
    // logging operations.
    inline static void LogInfo(const QString &message) {
        Log(LOG_INFO, message);
    }

    inline static void LogWarning(const QString &message) {
        Log(LOG_WARNING, message);
    }

    inline static void LogError(const QString &message) {
        Log(LOG_ERROR, message);
    }

    inline static void LogDebug(const QString &message) {
        Log(LOG_DEBUG, message);
    }

    inline static Logger* GetInstance() {
        Q_ASSERT(instance != NULL);
        return instance;
    }

protected:
    QTextStream *outputStream;
    LogLevel outputLevel;
    QMutex logMutex;
    static Logger *instance;

signals:

public slots:

};

#endif // LOGGER_H
