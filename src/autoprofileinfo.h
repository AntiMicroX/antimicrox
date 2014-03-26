#ifndef AUTOPROFILEINFO_H
#define AUTOPROFILEINFO_H

#include <QObject>
#include <QMetaType>

class AutoProfileInfo : public QObject
{
    Q_OBJECT
public:
    explicit AutoProfileInfo(QString guid, QString profileLocation,
                             bool active, QObject *parent = 0);
    explicit AutoProfileInfo(QString guid, QString profileLocation,
                             QString exe, bool active, QObject *parent = 0);
    explicit AutoProfileInfo(QObject *parent=0);
    ~AutoProfileInfo();

    void setGUID(QString guid);
    QString getGUID();

    void setProfileLocation(QString profileLocation);
    QString getProfileLocation();

    void setExe(QString exe);
    QString getExe();

    void setActive(bool active);
    bool isActive();

    void setDeviceName(QString name);
    QString getDeviceName();

    void setDefaultState(bool value);
    bool isCurrentDefault();

protected:
    QString guid;
    QString profileLocation;
    QString exe;
    QString deviceName;
    bool active;
    bool defaultState;

signals:

public slots:

};

Q_DECLARE_METATYPE(AutoProfileInfo*)

#endif // AUTOPROFILEINFO_H
