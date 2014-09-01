#ifndef UINPUTHELPER_H
#define UINPUTHELPER_H

#include <QObject>
#include <QString>
#include <QHash>

class UInputHelper : public QObject
{
    Q_OBJECT
public:
    static UInputHelper* getInstance();
    void deleteInstance();

    QString getDisplayString(unsigned int virtualkey);
    unsigned int getVirtualKey(QString codestring);

protected:
    explicit UInputHelper(QObject *parent = 0);
    ~UInputHelper();

    void populateKnownAliases();

    static UInputHelper *_instance;
    QHash<QString, unsigned int> knownAliasesX11SymVK;
    QHash<unsigned int, QString> knownAliasesVKStrings;

signals:

public slots:

};

#endif // UINPUTHELPER_H
