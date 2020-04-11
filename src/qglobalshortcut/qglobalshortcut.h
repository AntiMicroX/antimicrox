/*
  from https://github.com/mitei/qglobalshortcut
  author: mitei
*/

#pragma once

#include <QObject>
#include <QMultiHash>
#include <QAbstractNativeEventFilter>
#include <QKeySequence>

class QGlobalShortcut : public QObject {
    Q_OBJECT
    Q_PROPERTY(QKeySequence key READ key WRITE setKey)
    //Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)
private:

    class QGlobalShortcutEventFilter : public QAbstractNativeEventFilter {
    public:
        bool nativeEventFilter(const QByteArray& eventType, void* message, long* result);
    };

public:
    explicit QGlobalShortcut(QObject* parent = nullptr);
    explicit QGlobalShortcut(const QKeySequence& keyseq, QObject* parent = nullptr);
    ~QGlobalShortcut();
    bool nativeEventFilter(const QByteArray& event_type, void* message, long* result);

    QKeySequence key() const;
    void setKey(const QKeySequence& keyseq);
    //bool isEnabled() const;    void setEnabled(bool on);

signals:
    void activated();

private:
    QKeySequence keyseq_;
    void unsetKey();
    void initialize();

private:
    static QGlobalShortcutEventFilter global_shortcut_event_;
    /* quint32           keyid
       QGlobalShortcut*  shortcut */
    static QMultiHash<quint32, QGlobalShortcut*> shortcuts_;
    static bool activate(quint32 id);
    static inline quint32 calcId(const QKeySequence& keyseq);
    static inline quint32 calcId(quint32 k, quint32 m);
    static inline Qt::Key getKey(const QKeySequence& keyseq);
    static inline Qt::KeyboardModifiers getMods(const QKeySequence& keyseq);
    static quint32 toNativeKeycode(Qt::Key k);
    static quint32 toNativeModifiers(Qt::KeyboardModifiers m);
    static void registerKey(quint32 k, quint32 m, quint32 id);
    static void unregisterKey(quint32 k, quint32 m, quint32 id);
};
