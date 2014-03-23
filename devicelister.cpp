#include <QUrl>
#include <QDebug>
#include <QXmlStreamReader>

#include "devicelister.h"

struct DeviceListerPrivate
{
};

DeviceLister::DeviceLister(QObject *parent)
    : QObject(parent)
{
}

DeviceLister::~DeviceLister()
{
}

void DeviceLister::addDialDevice(const QUrl &dialRestUrl, const QByteArray &description)
{
    DialDevice device;
    device.dialRestUrl = dialRestUrl;

    // Parse description
    QXmlStreamReader xml(description);
    while(!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            QStringRef name(xml.name());

            if (name == QLatin1String("friendlyName")) {
                device.friendlyName = xml.readElementText();
            }
            else if (name == QLatin1String("modelName")) {
                device.modelName = xml.readElementText();
            }
            else if (name == QLatin1String("manufacturer")) {
                device.manufacturer = xml.readElementText();
            }
            else if (name == QLatin1String("deviceType")) {
                device.deviceType = xml.readElementText();
            }
            else if (name == QLatin1String("UDN")) {
                device.udn = xml.readElementText();
            }
        }
    }
    
    if (xml.hasError()) {
        qDebug() << "Unable to parse description" << xml.errorString() << xml.lineNumber();
        return;
    }

    // Print info
    qDebug() << "Friendly Name:" << device.friendlyName;
    qDebug() << "Model Name:" << device.modelName;
    qDebug() << "Manufacturer:" << device.manufacturer;
    qDebug() << "Device Type:" << device.deviceType;
    qDebug() << "UDN:" << device.udn;
    qDebug() << "DIAL Rest URL:" << device.dialRestUrl;

    emit deviceAdded(device);
}


