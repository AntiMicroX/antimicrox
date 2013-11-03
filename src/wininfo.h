#ifndef WININFO_H
#define WININFO_H

#include <QObject>
#include <QString>
#include <QHash>

class WinInfo : public QObject
{
    Q_OBJECT
public:
    static QString getDisplayString(unsigned int virtualkey);
    static unsigned int getVirtualKey(QString codestring);
    static unsigned int correctVirtualKey(unsigned int scancode,
                                          unsigned int virtualkey);
    static unsigned int scancodeFromVirtualKey(unsigned int virtualkey);

protected:
    explicit WinInfo(QObject *parent = 0);
    void populateKnownAliases();

    static WinInfo _instance;
    QHash<QString, unsigned int> knownAliasesX11SymVK;
    QHash<unsigned int, QString> knownAliasesVKStrings;

signals:

public slots:

};

#endif // WININFO_H
