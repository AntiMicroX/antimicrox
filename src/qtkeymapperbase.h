#ifndef QTKEYMAPPERBASE_H
#define QTKEYMAPPERBASE_H

#include <QObject>
#include <QHash>

class QtKeyMapperBase : public QObject
{
    Q_OBJECT
public:
    virtual unsigned int returnInstanceVirtualKey(unsigned int qkey) = 0;
    virtual unsigned int returnInstanceQtKey(unsigned int key) = 0;

    enum {
        AntKey_Shift_R = 0x400000E5, // SDLK_RSHIFT
        AntKey_Control_R = 0x400000E4, // SDLK_RCTRL
        AntKey_Shift_Lock = 0x1100ffe6, // XK_Shift_Lock | 0x11000000
        AntKey_Meta_R = 0x400000E7, // SDLK_RGUI
        AntKey_Alt_R = 0x400000E6, // SDLK_RALT
        AntKey_KP_Divide = 0x40000054, // SDLK_KP_DIVIDE
        AntKey_KP_Multiply = 0x40000055, // SDLK_KP_MULTIPLY
        AntKey_KP_Subtract = 0x40000056, // SDLK_KP_MINUS
        AntKey_KP_Add = 0x40000057, // SDLK_KP_PLUS
        AntKey_KP_Decimal = 0x40000063, // SDLK_KP_PERIOD
        AntKey_KP_Insert = 0x40000049, // SDLK_INSERT
        AntKey_KP_Delete = 0x110000b1, // Final part taken from SDLK_DELETE
        AntKey_KP_End = 0x4000004D, // SDLK_END
        AntKey_KP_Down = 0x40000051, // SDLK_DOWN
        AntKey_KP_Prior = 0x4000009D, // SDLK_PRIOR
        AntKey_KP_Left = 0x40000050, // SDLK_LEFT
        AntKey_KP_Begin = 0x400000D8, // SDLK_KP_CLEAR
        AntKey_KP_Right = 0x4000004F, // SDLK_RIGHT
        AntKey_KP_Home = 0x4000004A, // SDLK_HOME
        AntKey_KP_Up = 0x40000052, // SDLK_UP
        AntKey_KP_Next = 0x4000004E, // SDLK_PAGEDOWN
        AntKey_KP_0 = 0x40000059, // SDLK_KP_0
        AntKey_KP_1 = 0x4000005a, // SDLK_KP_1
        AntKey_KP_2 = 0x4000005b, // SDLK_KP_2
        AntKey_KP_3 = 0x4000005c, // SDLK_KP_3
        AntKey_KP_4 = 0x4000005d, // SDLK_KP_4
        AntKey_KP_5 = 0x4000005e, // SDLK_KP_5
        AntKey_KP_6 = 0x4000005f, // SDLK_KP_6
        AntKey_KP_7 = 0x40000060, // SDLK_KP_7
        AntKey_KP_8 = 0x40000061, // SDLK_KP_8
        AntKey_KP_9 = 0x40000062 // SDLK_KP_9
    };

protected:
    explicit QtKeyMapperBase(QObject *parent = 0);
    virtual void populateMappingHashes() = 0;

signals:

public slots:

};

#endif // QTKEYMAPPERBASE_H
