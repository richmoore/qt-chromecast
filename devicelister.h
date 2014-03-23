#ifndef DEVICELISTER_H
#define DEVICELISTER_H

#include <QObject>
#include <QUrl>

struct DialDevice
{
    QUrl dialRestUrl;
    QString friendlyName;
    QString modelName;
    QString manufacturer;
    QString deviceType;
    QString udn;
};

struct DeviceListerPrivate;

class DeviceLister : public QObject
{
    Q_OBJECT

public:
    DeviceLister(QObject *parent=0);
    ~DeviceLister();

public slots:
    void addDialDevice(const QUrl &dialRestUrl, const QByteArray &description);

signals:
    void deviceAdded(const DialDevice &dev);

private:
    DeviceListerPrivate *d;
};

#endif // DEVICELISTER_H

