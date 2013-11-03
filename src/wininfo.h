#ifndef WININFO_H
#define WININFO_H

#include <QObject>
#include <QString>
#include <QHash>

class WinInfo : public QObject
{
    Q_OBJECT
public:
    ~WinInfo();
    static QString getDisplayString(QString codestring);
    static unsigned int getVirtualKey(QString codestring);

protected:
    explicit WinInfo(QObject *parent = 0);
    void populateKnownAliases();

    class KeyInfo
    {
    public:
        KeyInfo(unsigned int vk, QString name)
        {
            this->vk = vk;
            this->name = name;
        }

        unsigned int vk;
        QString name;
    };

    static WinInfo _instance;
    QHash<QString, KeyInfo*> knownAliases;

signals:

public slots:

};

#endif // WININFO_H
