#ifndef DISCOVERER_H
#define DISCOVERER_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;
class QUrl;

struct DiscovererPrivate;

class Discoverer : public QObject
{
    Q_OBJECT

public:
    enum State {
        StateIdle,
        StateSearching,
        StateError
    };

    Discoverer(QObject *parent=0);
    ~Discoverer();

signals:
    void serverFound(const QUrl &location);
    void foundDialDevice(const QUrl &dialRestUrl, const QByteArray &description);

public slots:
    void startDiscovery();
    void readDeviceDescription(const QUrl &location);

private slots:
    void processReplies();
    void processDescription(QNetworkReply *reply);

private:
    DiscovererPrivate *d;
};

#endif // DISCOVERER_H
