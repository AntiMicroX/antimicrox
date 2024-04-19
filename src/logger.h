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

#include <sstream>

/**
 * @brief Macro used for printing messages to stdout
 *
 * Example usage
 * PRINT_STDOUT() << "my message";
 *
 */
#define PRINT_STDOUT() StreamPrinter(stdout, __LINE__, __FILE__)
#define PRINT_STDERR() StreamPrinter(stderr, __LINE__, __FILE__)

#define DEBUG() LogHelper(Logger::LogLevel::LOG_DEBUG, __LINE__, __FILE__)
#define VERBOSE() LogHelper(Logger::LogLevel::LOG_VERBOSE, __LINE__, __FILE__)
#define INFO() LogHelper(Logger::LogLevel::LOG_INFO, __LINE__, __FILE__)
#define WARN() LogHelper(Logger::LogLevel::LOG_WARNING, __LINE__, __FILE__)
#define ERROR() LogHelper(Logger::LogLevel::LOG_ERROR, __LINE__, __FILE__)
/**
 * @brief Custom singleton class used for logging across application.
 *
 * It manages log-levels, formatting, printing logs and saving them to file.
 * Logs across the program can be written using
 * Local macros(better support for showing log location in release builds):
 * DEBUG(), INFO(), VERBOSE(), WARN(), ERROR()
 * QT macros:
 * qDebug(), qInfo(), qWarning(), qCritical, and qFatal()
 *
 */
class Logger : public QObject
{
    Q_OBJECT
    Q_ENUMS(LogLevel)

  public:
    enum LogLevel
    {
        LOG_NONE = 0,
        LOG_ERROR = 1,
        LOG_WARNING = 2,
        LOG_INFO = 3,
        LOG_VERBOSE = 4,
        LOG_DEBUG = 5,
        LOG_MAX = LOG_DEBUG
    };

    ~Logger();

    static void loggerMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    static void setLogLevel(LogLevel level);
    LogLevel getCurrentLogLevel();
    static bool isDebugEnabled();

    static void setCurrentStream(QTextStream *stream);
    static void setCurrentLogFile(QString filename);
    static QString getCurrentLogFile();
    bool isWritingToFile();
    static QTextStream *getCurrentStream();

    /**
     * @brief Get the Instance of logger
     *
     * @param raiseExceptionForNull - raise std::runtime_error when instance doesn't exist
     * @return Logger*
     */
    inline static Logger *getInstance(bool raiseExceptionForNull = true)
    {
        if (raiseExceptionForNull && instance == nullptr)
        {
            throw std::runtime_error("There is no logger instance");
        }
        return instance;
    }

    static Logger *createInstance(QTextStream *stream = nullptr, LogLevel outputLevel = LOG_INFO, QObject *parent = nullptr);

  protected:
    explicit Logger(QTextStream *stream, LogLevel output_lvl = LOG_INFO, QObject *parent = nullptr);
    void closeLogger(bool closeStream = true);

    static Logger *instance;

    QFile outputFile;
    QTextStream outFileStream;
    QTextStream *outputStream;

    LogLevel outputLevel;
    QMutex logMutex;
    QThread *loggingThread; // in this thread all of writing operations will be executed

  public slots:
    void logMessage(const QString &message, const Logger::LogLevel level, const uint lineno, const QString &filename);
};

/**
 * @brief simple helper class used for constructing log message and sending it to Logger
 *
 * Message is sent either by using sendMessage(), or during destruction.
 */
class LogHelper : public QObject
{
    Q_OBJECT
  public:
    QString message;
    Logger::LogLevel level;
    uint lineno;
    QString filename;
    bool is_message_sent;
    Logger::LogLevel log_level;

    LogHelper(const Logger::LogLevel level, const uint lineno, const QString &filename, const QString &message = "")
        : message(message)
        , level(level)
        , lineno(lineno)
        , filename(filename)
        , is_message_sent(false)
    {
        Logger *pointer = Logger::getInstance();
        log_level = pointer->getCurrentLogLevel();
        connect(this, &LogHelper::logMessage, pointer, &Logger::logMessage);
    };

    ~LogHelper()
    {
        if (!is_message_sent)
            sendMessage();
    }

    void sendMessage()
    {
        is_message_sent = true;
        emit logMessage(message, level, lineno, filename);
    };

    LogHelper &operator<<(const QString &s)
    {
        if (log_level != Logger::LogLevel::LOG_NONE)
            message = message + s;
        return *this;
    };
    template <typename Message> LogHelper &operator<<(Message ch)
    {
        if (log_level != Logger::LogLevel::LOG_NONE)
        {
            // The simplest way of building string from possible variables
            std::stringstream str;
            str << ch;
            message = message + str.str().c_str();
        }
        return *this;
    }
  signals:
    void logMessage(const QString &message, const Logger::LogLevel level, const uint lineno, const QString &filename);
};

/**
 * @brief Simple adapter for QTextStream additionally logging printed values
 * Logs are printed when StreamPrinter is destroyed or logContent() is called
 *
 * Recommended usage with macros PRINT_STDOUT() and PRINT_STDERR()
 */
class StreamPrinter : public QObject
{
    Q_OBJECT
  private:
    QTextStream m_stream;
    std::stringstream m_message;
    uint m_lineno;
    QString m_filename;

  public:
    StreamPrinter(FILE *file, uint lineno = 0, QString filename = "")
        : m_stream(file)
        , m_message("")
        , m_lineno(lineno)
        , m_filename(filename)
    {
        if (file == stdout)
        {
            m_message << "Printed stdout message📓: ";
        } else if (file == stderr)
        {
            m_message << "Printed stderr message📓: ";
        } else
        {
            m_message << "unknown stream ";
        }
    };

    ~StreamPrinter()
    {
        // When logger prints to stream, then we already have printed messages into console,
        // there is no need to duplicate
        if (Logger::getInstance()->isWritingToFile())
            LogHelper(Logger::LogLevel::LOG_INFO, m_lineno, m_filename, QString(m_message.str().c_str())).sendMessage();
    };

    template <typename Message> StreamPrinter &operator<<(Message ch)
    {
        m_stream << ch;
        m_message << ch;
        return *this;
    }

    StreamPrinter &operator<<(const QString &s)
    {
        m_stream << s;
        m_message << s.toStdString();
        return *this;
    };
};

#endif // LOGGER_H
