#ifndef UNIXCAPTUREWINDOWUTILITY_H
#define UNIXCAPTUREWINDOWUTILITY_H

#include <QObject>

class UnixCaptureWindowUtility : public QObject
{
    Q_OBJECT
public:
    explicit UnixCaptureWindowUtility(QObject *parent = 0);
    QString getTargetPath();

protected:
    QString targetPath;

signals:
    void captureFinished();

public slots:
    void attemptWindowCapture();
};

#endif // UNIXCAPTUREWINDOWUTILITY_H
