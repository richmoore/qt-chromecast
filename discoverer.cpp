#include <QHostAddress>
#include <QUdpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDebug>

#include "discoverer.h"

const int UPNP_PORT = 1900;

struct DiscovererPrivate
{
    Discoverer::State state;
    QUdpSocket *sock;
    QNetworkAccessManager *nam;
};

Discoverer::Discoverer(QObject *parent)
    : QObject(parent)
{
    d = new DiscovererPrivate;
    d->state = StateIdle;
    d->nam = new QNetworkAccessManager(this);

    connect(this, SIGNAL(serverFound(const QUrl &)),
            SLOT(readDeviceDescription(const QUrl &)));
}

Discoverer::~Discoverer()
{
    delete d;
}

void Discoverer::startDiscovery()
{
    if (d->state != StateIdle) {
        qDebug() << "Not Idle";
        return;
    }

    d->state = StateSearching;
    d->sock = new QUdpSocket(this);
    //bool ok = d->sock->bind(QHostAddress(QHostAddress::AnyIPv4), UPNP_PORT);
#if QT_VERSION >= 0x050000
    bool ok = d->sock->bind(QHostAddress(QHostAddress::AnyIPv4));
#else
    bool ok = d->sock->bind(QHostAddress(QHostAddress::Any), 0);
#endif
    if (!ok) {
        qDebug() << "Bind failed" << d->sock->errorString();
        d->state = StateError;
        return;
    }

    connect(d->sock, SIGNAL(readyRead()), SLOT(processReplies()));

    // Send UPNP Search
    QHostAddress groupAddress("239.255.255.250");
    ok = d->sock->joinMulticastGroup(groupAddress);
    if (!ok) {
        qDebug() << "Joining group failed" << d->sock->errorString();
        d->state = StateError;
        return;
    }

    QByteArray message("M-SEARCH * HTTP/1.1\r\n"        \
                       "HOST: 239.255.255.250:1900\r\n" \
                       "MAN: \"ssdp:discover\"\r\n" \
                       "MX: 1\r\n" \
                       "ST: urn:dial-multiscreen-org:service:dial:1\r\n" \
                       "\r\n");
                    
    ok = d->sock->writeDatagram(message.constData(), message.size(),
                                groupAddress, UPNP_PORT);
    if (!ok) {
        qDebug() << "Sending failed" << d->sock->errorString();
        d->state = StateError;
        return;
    }
}

void Discoverer::processReplies()
{
    while (d->sock->hasPendingDatagrams()) {
        QByteArray reply;
        reply.resize(d->sock->pendingDatagramSize());
        d->sock->readDatagram(reply.data(), reply.size());
        //qDebug() << "Got reply:\n" << reply;

        QList<QByteArray> lines = reply.split('\n');

        if (qstrnicmp(lines[0].constData(), "http/1.1 200", 12)) {
            qDebug() << "Unexpected response";
            continue;
        }

        foreach (const QByteArray &line, lines) {
            if (!qstrnicmp(line.constData(), "Location: ", 10)) {
                QByteArray urldata(line.mid(10).trimmed());
                qDebug() << "URL:" << urldata;

                QUrl url(QUrl::fromEncoded(urldata));
                if (!url.isValid()) {
                    qDebug() << "Failed to parse url";
                    continue;
                }

                emit serverFound(url);
            }
        }
    }
}

void Discoverer::readDeviceDescription(const QUrl &location)
{
    qDebug() << "read device description" << location;

    if (!d->nam) {
        qDebug() << "No network access manager";
        return;
    }

    d->nam->get(QNetworkRequest(location));
    connect(d->nam, SIGNAL(finished(QNetworkReply*)),
            SLOT(processDescription(QNetworkReply *)),
            Qt::UniqueConnection);
}

void Discoverer::processDescription(QNetworkReply *reply)
{
    //    qDebug() << "process description for" << reply->request().url() << reply;

    QByteArray description = reply->readAll();
#if QT_VERSION >= 0x050000
    QByteArray urlData = reply->rawHeader(QByteArrayLiteral("Application-URL"));
#else
    QByteArray urlData = reply->rawHeader(QByteArray("Application-URL"));
#endif
    QUrl dialRestUrl(QUrl::fromEncoded(urlData));

    //    qDebug() << "dialRestUrl" << dialRestUrl;

    emit foundDialDevice(dialRestUrl, description);

    reply->deleteLater();
}
