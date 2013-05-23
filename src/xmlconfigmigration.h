#ifndef XMLCONFIGMIGRATION_H
#define XMLCONFIGMIGRATION_H

#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "common.h"

class XMLConfigMigration : public QObject
{
    Q_OBJECT
public:
    explicit XMLConfigMigration(QXmlStreamReader *reader, QObject *parent = 0);
    ~XMLConfigMigration();

    QString migrate();
    bool requiresMigration();

protected:
    QString initialMigration();
    void initialMigrationReadButton(QXmlStreamWriter &writer);

    QFile *configFile;
    QXmlStreamReader *reader;
    QString fileName;
    int fileVersion;


signals:
    
public slots:
    
};

#endif // XMLCONFIGMIGRATION_H
