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
    static QString getDisplayString(unsigned int virtualkey);
    static unsigned int getVirtualKey(QString codestring);
    static unsigned int correctVirtualKey(unsigned int scancode,
                                          unsigned int virtualkey);
    static unsigned int scancodeFromVirtualKey(unsigned int virtualkey, unsigned int alias=0);

    static const unsigned int EXTENDED_FLAG;
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
    explicit WinExtras(QObject *parent = 0);
    void populateKnownAliases();

    static WinExtras _instance;
    QHash<QString, unsigned int> knownAliasesX11SymVK;
    QHash<unsigned int, QString> knownAliasesVKStrings;
    static int originalMouseAccel;

signals:

public slots:

};

#endif // WINEXTRAS_H
