#ifndef WINEXTRAS_H
#define WINEXTRAS_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QPoint>

class WinExtras : public QObject
{
    Q_OBJECT

public:
    static QString getDisplayString(int virtualkey); // (unsigned)
    static int getVirtualKey(QString codestring); // unsigned
    static int correctVirtualKey(int scancode,
                                          int virtualkey); // unsigned (unsigned, unsigned)
    static int scancodeFromVirtualKey(int virtualkey, int alias=0); // unsigned (unsigned, unsigned)

    static const int EXTENDED_FLAG; // unsigned
    static QString getForegroundWindowExePath();
    static bool containsFileAssociationinRegistry();
    static void writeFileAssocationToRegistry();
    static void removeFileAssociationFromRegistry();
    static bool IsRunningAsAdmin();
    static bool elevateAntiMicro();
    static void disablePointerPrecision();
    static void enablePointerPrecision();
    static bool isUsingEnhancedPointerPrecision();
    static void grabCurrentPointerPrecision();
    static QString getCurrentWindowText();
    static bool raiseProcessPriority();
    static QPoint getCursorPos();

protected:
    explicit WinExtras(QObject *parent = nullptr);
    void populateKnownAliases();

    static WinExtras _instance;
    QHash<QString, int> knownAliasesX11SymVK; // <.., unsigned>
    QHash<int, QString> knownAliasesVKStrings; // <unsigned, ..>
    static int originalMouseAccel;

};

#endif // WINEXTRAS_H
