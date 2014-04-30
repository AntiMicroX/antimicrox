#ifndef AUTOPROFILEWATCHER_H
#define AUTOPROFILEWATCHER_H

#include <QObject>
#include <QTimer>
#include <QHash>
#include <QList>
#include <QSettings>

#include "autoprofileinfo.h"


class AutoProfileWatcher : public QObject
{
    Q_OBJECT
public:
    explicit AutoProfileWatcher(QSettings *settings, QObject *parent = 0);
    void startTimer();
    void stopTimer();
    QList<AutoProfileInfo*>* getCustomDefaults();
    AutoProfileInfo* getDefaultAllProfile();

    static const int CHECKTIME = 1000; // time in ms

protected:
    QString findAppLocation();

    QTimer appTimer;
    QSettings *settings;
    QHash<QString, QList<AutoProfileInfo*> > appProfileAssignments;
    QHash<QString, AutoProfileInfo*> defaultProfileAssignments;
    //QHash<QString, QList<QHash<QString, QString> > > appProfileAssignments;
    //QHash<QString, QHash<QString, QString> > defaultProfileAssignments;
    //QList<AutoProfileInfo*> *customDefaults;
    AutoProfileInfo *allDefaultInfo;
    QString currentApplication;

signals:
    void foundApplicableProfile(AutoProfileInfo *info);

public slots:
    void syncProfileAssignment();

private slots:
    void runAppCheck();
};

#endif // AUTOPROFILEWATCHER_H
