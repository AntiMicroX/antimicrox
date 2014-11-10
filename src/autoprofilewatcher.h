#ifndef AUTOPROFILEWATCHER_H
#define AUTOPROFILEWATCHER_H

#include <QObject>
#include <QTimer>
#include <QHash>
#include <QList>

#include "autoprofileinfo.h"
#include "antimicrosettings.h"


class AutoProfileWatcher : public QObject
{
    Q_OBJECT
public:
    explicit AutoProfileWatcher(AntiMicroSettings *settings, QObject *parent = 0);
    void startTimer();
    void stopTimer();
    QList<AutoProfileInfo*>* getCustomDefaults();
    AutoProfileInfo* getDefaultAllProfile();

    static const int CHECKTIME = 1000; // time in ms

protected:
    QString findAppLocation();
    void clearProfileAssignments();

    QTimer appTimer;
    AntiMicroSettings *settings;
    // Path, QList<AutoProfileInfo*>
    QHash<QString, QList<AutoProfileInfo*> > appProfileAssignments;
    // WM_CLASS, QList<AutoProfileInfo*>
    QHash<QString, QList<AutoProfileInfo*> > windowClassProfileAssignments;
    // WM_NAME, QList<AutoProfileInfo*>
    QHash<QString, QList<AutoProfileInfo*> > windowNameProfileAssignments;
    // GUID, AutoProfileInfo*
    QHash<QString, AutoProfileInfo*> defaultProfileAssignments;
    //QList<AutoProfileInfo*> *customDefaults;
    AutoProfileInfo *allDefaultInfo;
    QString currentApplication;
    QString currentAppWindowTitle;

signals:
    void foundApplicableProfile(AutoProfileInfo *info);

public slots:
    void syncProfileAssignment();

private slots:
    void runAppCheck();
};

#endif // AUTOPROFILEWATCHER_H
