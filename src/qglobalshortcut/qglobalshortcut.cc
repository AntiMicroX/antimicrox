/*
  from https://github.com/mitei/qglobalshortcut
  author: mitei
*/

#include "qglobalshortcut.h"
#include <QCoreApplication>
#include <QtDebug>
#include <QKeySequence>

#ifdef WITH_X11
#include <QX11Info>
#include <xcb/xcb.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#endif

QMultiHash<quint32, QGlobalShortcut*> QGlobalShortcut::shortcuts_;
QGlobalShortcut::QGlobalShortcutEventFilter QGlobalShortcut::global_shortcut_event_;

QGlobalShortcut::QGlobalShortcut(QObject* parent)
    : QObject(parent)
{
    initialize();
}

QGlobalShortcut::QGlobalShortcut(const QKeySequence &keyseq, QObject *parent)
    : QObject(parent)
{
    initialize();
    setKey(keyseq);
}

void QGlobalShortcut::initialize() {
    static bool initialized = false;
    if (!initialized) {
        qApp->installNativeEventFilter(&global_shortcut_event_);
        initialized = true;
    }
}

QGlobalShortcut::~QGlobalShortcut() {
    unsetKey();
}

QKeySequence QGlobalShortcut::key() const
{
    return keyseq_;
}

void QGlobalShortcut::setKey(const QKeySequence& keyseq)
{
    if (!keyseq_.isEmpty()) {
        qDebug() << "Key is not empty";
        unsetKey();
    }

    quint32 keyid = calcId(keyseq);
    qDebug() << "keyid is: " << keyid;

    if (shortcuts_.count(keyid) == 0) {
        quint32 keycode = toNativeKeycode(getKey(keyseq));
        quint32 mods = toNativeModifiers(getMods(keyseq));
        qDebug() << "keycode is: " << keycode;
        qDebug() << "mods is: " << mods;
        registerKey(keycode, mods, keyid);
    }

    this->keyseq_ = keyseq;
    shortcuts_.insert(keyid, this);
}

void QGlobalShortcut::unsetKey() {
    quint32 keyid = calcId(keyseq_);
    if (shortcuts_.remove(keyid, this) > 0) {
        if (shortcuts_.count(keyid) == 0) {
            quint32 keycode = toNativeKeycode(getKey(keyseq_));
            quint32 mods = toNativeModifiers(getMods(keyseq_));
            unregisterKey(keycode, mods, keyid);
        }
    }
}

bool QGlobalShortcut::activate(quint32 id) {
    if (shortcuts_.contains(id)) {
        foreach (QGlobalShortcut* s, shortcuts_.values(id)) {
            emit s->activated();
        }
        return true;
    }
    return false;
}

quint32 QGlobalShortcut::calcId(const QKeySequence& keyseq) {
    quint32 keycode = toNativeKeycode(getKey(keyseq));
    quint32 mods    = toNativeModifiers(getMods(keyseq));
    return calcId(keycode, mods);
}

#ifndef Q_OS_UNIX
quint32 QGlobalShortcut::calcId(quint32 k, quint32 m) {
    return k | m;
}
#endif

Qt::Key QGlobalShortcut::getKey(const QKeySequence& keyseq) {
    if (keyseq.isEmpty()) {
        return Qt::Key(0);
    }
    return Qt::Key(keyseq[0] & ~Qt::KeyboardModifierMask);
}

Qt::KeyboardModifiers QGlobalShortcut::getMods(const QKeySequence& keyseq) {
    if (keyseq.isEmpty()) {
        return Qt::KeyboardModifiers(0);
    }
    return Qt::KeyboardModifiers(keyseq[0] & Qt::KeyboardModifierMask);
}


#ifdef Q_OS_UNIX
#ifdef WITH_X11
bool QGlobalShortcut::QGlobalShortcutEventFilter::nativeEventFilter(
        const QByteArray& eventType, void* message, long* result)
{
    xcb_generic_event_t* e = static_cast<xcb_generic_event_t*>(message);
    if ((e->response_type & ~0x80) == XCB_KEY_PRESS) {
        xcb_key_press_event_t* ke = (xcb_key_press_event_t*)e;
        xcb_get_keyboard_mapping_reply_t rep;
        xcb_keysym_t* k = xcb_get_keyboard_mapping_keysyms(&rep);
        quint32 keycode = ke->detail;
        quint32 mods = ke->state & (ShiftMask|ControlMask|Mod1Mask|Mod3Mask);
        return activate(calcId(keycode, mods));
    }
    return false;
}

quint32 QGlobalShortcut::calcId(quint32 k, quint32 m) {
    return k | (m << 16);
}

quint32 QGlobalShortcut::toNativeKeycode(Qt::Key k) {
    /* keysymdef.h */
    quint32 key = 0;
    if (k >= Qt::Key_F1 && k <= Qt::Key_F35) {
        key = XK_F1 + (k - Qt::Key_F1);
    } else if (k >= Qt::Key_Space && k <= Qt::Key_QuoteLeft) {
        key = k;
    } else if (k >= Qt::Key_BraceLeft && k <= Qt::Key_AsciiTilde) {
        key = k;
    } else if (k >= Qt::Key_nobreakspace && k <= Qt::Key_ydiaeresis) {
        key = k;
    } else {
        switch (k) {
        case Qt::Key_Escape:
            key = XK_Escape;
            break;
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            key = XK_Tab;
            break;
        case Qt::Key_Backspace:
            key = XK_BackSpace;
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            key = XK_Return;
            break;
        case Qt::Key_Insert:
            key = XK_Insert;
            break;
        case Qt::Key_Delete:
            key = XK_Delete;
            break;
        case Qt::Key_Pause:
            key = XK_Pause;
            break;
        case Qt::Key_Print:
            key = XK_Print;
            break;
        case Qt::Key_SysReq:
            key = XK_Sys_Req;
            break;
        case Qt::Key_Clear:
            key = XK_Clear;
            break;
        case Qt::Key_Home:
            key = XK_Home;
            break;
        case Qt::Key_End:
            key = XK_End;
            break;
        case Qt::Key_Left:
            key = XK_Left;
            break;
        case Qt::Key_Up:
            key = XK_Up;
            break;
        case Qt::Key_Right:
            key = XK_Right;
            break;
        case Qt::Key_Down:
            key = XK_Down;
            break;
        case Qt::Key_PageUp:
            key = XK_Page_Up;
            break;
        case Qt::Key_PageDown:
            key = XK_Page_Down;
            break;
        default:
            key = 0;
        }
    }
    return XKeysymToKeycode(QX11Info::display(), key);
}

quint32 QGlobalShortcut::toNativeModifiers(Qt::KeyboardModifiers m) {
    quint32 mods = Qt::NoModifier;
    if (m & Qt::NoModifier)
        mods |= AnyModifier;
    if (m & Qt::ShiftModifier)
        mods |= ShiftMask;
    if (m & Qt::ControlModifier)
        mods |= ControlMask;
    if (m & Qt::AltModifier)
        mods |= Mod1Mask;
    if (m & Qt::MetaModifier)
        mods |= Mod4Mask;

    return mods;
}

void QGlobalShortcut::registerKey(quint32 k, quint32 m, quint32 id)
{
    xcb_grab_key(QX11Info::connection(), 1, QX11Info::appRootWindow(),
                 m, k, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
}

void QGlobalShortcut::unregisterKey(quint32 k, quint32 m, quint32 id)
{
    xcb_ungrab_key(QX11Info::connection(), k, QX11Info::appRootWindow(), m);
}
#endif
#endif
