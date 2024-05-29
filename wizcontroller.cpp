#include "wizcontroller.h"

#include <QColor>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkDatagram>

WIZController::WIZController(QObject *parent)
    : QObject{parent}, _udpSocket(new QUdpSocket(this)), _api("client-id")
{
    _api.setBearerToken("Bearer token");

    _udpSocket->bind(QHostAddress::AnyIPv4, 38899);
    connect(_udpSocket, &QUdpSocket::readyRead,
            this, &WIZController::onDatagram);
}

void WIZController::detectLights()
{
    QUdpSocket sendSocket;
    QByteArray dgram = "{\"method\":\"registration\",\"params\":{\"phoneMac\":\"AAAAAAAAAAAA\",\"register\":false,\"phoneIp\":\"1.2.3.4\",\"id\":\"1\"}}";
    _udpSocket->writeDatagram(dgram.data(), dgram.size(), QHostAddress("255.255.255.255"), 38899);
    _bulbs.clear();
    qDebug() << "Sending datagram";

}

void WIZController::changeColor(QString inputColor)
{
    QColor color(inputColor);

    for (QString bulb : _bulbs) {
        QJsonDocument doc;
        QJsonObject obj;
        QJsonObject params;
        obj["method"] = "setPilot";
        params["r"] = color.red();
        params["g"] = color.green();
        params["b"] = color.blue();
        params["dimming"] = 100;
        params["state"] = true;
        if (0 == color.red() == color.blue() == color.green()) {
            params["state"] = false;
        }
        obj["params"] = params;
        doc.setObject(obj);
        QString data = doc.toJson(QJsonDocument::Compact);
        QByteArray dgram = data.toLatin1();
        qDebug() << data;
        _udpSocket->writeDatagram(dgram.data(), dgram.size(), QHostAddress(bulb), 38899);
    }
}

void WIZController::registerSession(QString sessionId)
{
}

QStringList WIZController::getBulbs() const
{
    return _bulbs;
}

void WIZController::onDatagram()
{
    while (_udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = _udpSocket->receiveDatagram();
        QJsonDocument response = QJsonDocument::fromJson(datagram.data());

        if (response.object().contains("result") && response["method"].toString() == "registration") {
            qDebug() << datagram.data();
            qDebug() << "Discoverd lightbulb: " << datagram.senderAddress().toString();

            _bulbs.append(datagram.senderAddress().toString());

            emit bulbsChanged();
        } else {
            qDebug() << datagram.data();
        }
    }
}
