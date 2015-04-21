#include <QHashIterator>

#include "qtvmultikeymapper.h"

QtVMultiKeyMapper::QtVMultiKeyMapper(QObject *parent) :
    QtKeyMapperBase(parent)
{
    populateMappingHashes();
}

void QtVMultiKeyMapper::populateMappingHashes()
{
    if (qtKeyToVirtualKey.isEmpty())
    {
        // Map A - Z keys
        for (int i=0; i <= (Qt::Key_Z - Qt::Key_A); i++)
        {
            qtKeyToVirtualKey[Qt::Key_A + i] = 0x04 + i;
        }

        // Map 1 - 9 numeric keys
        for (int i=0; i <= (Qt::Key_9 - Qt::Key_1); i++)
        {
            qtKeyToVirtualKey[Qt::Key_1 + i] = 0x1E + i;
        }

        // Map 0 numeric key
        qtKeyToVirtualKey[Qt::Key_0] = 0x27;

        qtKeyToVirtualKey[Qt::Key_Return] = 0x28;
        qtKeyToVirtualKey[Qt::Key_Escape] = 0x29;
        qtKeyToVirtualKey[Qt::Key_Backspace] = 0x2A;
        qtKeyToVirtualKey[Qt::Key_Tab] = 0x2B;
        qtKeyToVirtualKey[Qt::Key_Space] = 0x2C;
        qtKeyToVirtualKey[Qt::Key_Minus] = 0x2D;
        qtKeyToVirtualKey[Qt::Key_Equal] = 0x2E;
        qtKeyToVirtualKey[Qt::Key_BracketLeft] = 0x2F;
        qtKeyToVirtualKey[Qt::Key_BracketRight] = 0x30;
        qtKeyToVirtualKey[Qt::Key_Backslash] = 0x31;
        qtKeyToVirtualKey[Qt::Key_NumberSign] = 0x32;
        qtKeyToVirtualKey[Qt::Key_Semicolon] = 0x33;
        qtKeyToVirtualKey[Qt::Key_Apostrophe] = 0x34;
        qtKeyToVirtualKey[Qt::Key_Dead_Grave] = 0x35;
        qtKeyToVirtualKey[Qt::Key_Comma] = 0x36;
        qtKeyToVirtualKey[Qt::Key_Period] = 0x37;
        qtKeyToVirtualKey[Qt::Key_Slash] = 0x38;
        qtKeyToVirtualKey[Qt::Key_CapsLock] = 0x39;

        // Map F1 - F12 keys
        for (int i=0; i <= (Qt::Key_F12 - Qt::Key_F1); i++)
        {
            qtKeyToVirtualKey[Qt::Key_F1 + i] = 0x3A + i;
        }

        qtKeyToVirtualKey[Qt::Key_Print] = 0x46;
        qtKeyToVirtualKey[Qt::Key_ScrollLock] = 0x47;
        qtKeyToVirtualKey[Qt::Key_Pause] = 0x48;
        qtKeyToVirtualKey[Qt::Key_Insert] = 0x49;
        qtKeyToVirtualKey[Qt::Key_Home] = 0x4A;
        qtKeyToVirtualKey[Qt::Key_PageUp] = 0x4B;
        qtKeyToVirtualKey[Qt::Key_Delete] = 0x4C;
        qtKeyToVirtualKey[Qt::Key_End] = 0x4D;
        qtKeyToVirtualKey[Qt::Key_PageDown] = 0x4E;
        qtKeyToVirtualKey[Qt::Key_Right] = 0x4F;
        qtKeyToVirtualKey[Qt::Key_Left] = 0x50;
        qtKeyToVirtualKey[Qt::Key_Down] = 0x51;
        qtKeyToVirtualKey[Qt::Key_Up] = 0x52;
        qtKeyToVirtualKey[Qt::Key_NumLock] = 0x53;

        qtKeyToVirtualKey[AntKey_KP_Divide] = 0x54;
        qtKeyToVirtualKey[AntKey_KP_Multiply] = 0x55;
        qtKeyToVirtualKey[AntKey_KP_Subtract] = 0x56;
        qtKeyToVirtualKey[AntKey_KP_Add] = 0x57;
        qtKeyToVirtualKey[Qt::Key_Enter] = 0x58;

        // Map Numpad 1 - 9 keys
        for (int i=0; i <= (AntKey_KP_9 - AntKey_KP_1); i++)
        {
            qtKeyToVirtualKey[AntKey_KP_1 + i] = 0x59 + i;
        }

        qtKeyToVirtualKey[AntKey_KP_0] = 0x62;
        qtKeyToVirtualKey[AntKey_KP_Decimal] = 0x63;

        //qtKeyToVirtualKey[Qt::Key_Backslash] = 0x64;
        qtKeyToVirtualKey[Qt::Key_ApplicationLeft] = 0x65;
        qtKeyToVirtualKey[Qt::Key_PowerOff] = 0x66;
        //qtKeyToVirtualKey[] = 0x67;

        for (int i=0; i <= (Qt::Key_F24 - Qt::Key_F13); i++)
        {
            qtKeyToVirtualKey[Qt::Key_F13 + i] = 0x68 + i;
        }

        qtKeyToVirtualKey[Qt::Key_Execute] = 0x74;
        qtKeyToVirtualKey[Qt::Key_Help] = 0x75;
        qtKeyToVirtualKey[Qt::Key_Menu] = 0x76;
        qtKeyToVirtualKey[Qt::Key_Select] = 0x77;
        qtKeyToVirtualKey[Qt::Key_Stop] = 0x78;
        //qtKeyToVirtualKey[] = 0x79;
        qtKeyToVirtualKey[Qt::Key_Undo] = 0x7A;
        qtKeyToVirtualKey[Qt::Key_Cut] = 0x7B;
        qtKeyToVirtualKey[Qt::Key_Copy] = 0x7C;
        qtKeyToVirtualKey[Qt::Key_Paste] = 0x7D;
        qtKeyToVirtualKey[Qt::Key_Find] = 0x7E;
        qtKeyToVirtualKey[Qt::Key_VolumeMute] = 0x7F;
        qtKeyToVirtualKey[Qt::Key_VolumeUp] = 0x80;
        qtKeyToVirtualKey[Qt::Key_VolumeDown] = 0x81;
        //qtKeyToVirtualKey[] = 0x82;
        //qtKeyToVirtualKey[] = 0x83;
        //qtKeyToVirtualKey[] = 0x84;
        //qtKeyToVirtualKey[] = 0x85;

        // International Keys?
        //qtKeyToVirtualKey[] = 0x87;
        //qtKeyToVirtualKey[] = 0x88;
        //qtKeyToVirtualKey[] = 0x89;
        //qtKeyToVirtualKey[] = 0x8A;
        //qtKeyToVirtualKey[] = 0x8B;
        //qtKeyToVirtualKey[] = 0x8C;
        //qtKeyToVirtualKey[] = 0x8D;
        //qtKeyToVirtualKey[] = 0x8E;
        //qtKeyToVirtualKey[] = 0x8F;

        qtKeyToVirtualKey[Qt::Key_Control] = 0xE0;
        qtKeyToVirtualKey[Qt::Key_Shift] = 0xE1;
        qtKeyToVirtualKey[Qt::Key_Alt] = 0xE2;
        qtKeyToVirtualKey[Qt::Key_Meta] = 0xE3;
        qtKeyToVirtualKey[AntKey_Control_R] = 0xE4;
        qtKeyToVirtualKey[AntKey_Shift_R] = 0xE5;
        qtKeyToVirtualKey[AntKey_Meta_R] = 0xE7;

        qtKeyToVirtualKey[Qt::Key_LaunchMedia] = 0x87 | consumerUsagePagePrefix;

        // Populate other hash. Flip key and value so mapping
        // goes VK -> Qt Key.
        QHashIterator<unsigned int, unsigned int> iter(qtKeyToVirtualKey);
        while (iter.hasNext())
        {
            iter.next();
            virtualKeyToQtKey[iter.value()] = iter.key();
        }
    }
}
