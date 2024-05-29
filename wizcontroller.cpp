#include "wizcontroller.h"

#include <QColor>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkDatagram>
#include <QSettings>

static const char *SUBSCRIPTIONS_ENDPOINT = "https://api.twitch.tv/helix/eventsub/subscriptions";

WIZController::WIZController(QObject *parent)
    : QObject{parent}, _udpSocket(new QUdpSocket(this)), _nm(this), _o2(this), _o2Requestor(&_nm, &_o2, this), _settings("config.ini", QSettings::IniFormat)
{
    if (!_settings.value("initialized", false).toBool()) {
        _settings.setValue("initialized", true);
        _settings.setValue("client_id", "client_id");
        _settings.setValue("reward_message", "reward_message");
    }

    _udpSocket->bind(QHostAddress::AnyIPv4, 38899);
    connect(_udpSocket, &QUdpSocket::readyRead,
            this, &WIZController::onDatagram);

    _o2.setClientId(_settings.value("client_id").toString());
    _o2.setScope("channel:read:redemptions user:read:chat");
    _o2Requestor.setAddAccessTokenInQuery(false);
    _o2Requestor.setAccessTokenInAuthenticationHTTPHeaderFormat("Bearer %1");

    connect(&_o2, &O2Twitch::linkedChanged, this, &WIZController::linkedChanged);
    connect(&_o2, &O2Twitch::linkingFailed, this, &WIZController::linkingFailed);
    connect(&_o2, &O2Twitch::linkingSucceeded, this, &WIZController::linkingSucceeded);
    connect(&_o2, &O2Twitch::openBrowser, this, &WIZController::onOpenBrowser);
    connect(&_o2, &O2Twitch::closeBrowser, this, &WIZController::onCloseBrowser);
    connect(&_o2Requestor, static_cast<void (O2Requestor::*)(int,QNetworkReply::NetworkError,QByteArray)>(&O2Requestor::finished), this, &WIZController::onRequestFinished);
}

void WIZController::detectLights()
{
    QUdpSocket sendSocket;
    QByteArray dgram = "{\"method\":\"registration\",\"params\":{\"phoneMac\":\"AAAAAAAAAAAA\",\"register\":false,\"phoneIp\":\"1.2.3.4\",\"id\":\"1\"}}";
    _udpSocket->writeDatagram(dgram.data(), dgram.size(), QHostAddress::Broadcast, 38899);
    _bulbs.clear();
    qDebug() << "Sending datagram";
}

void WIZController::changeColor(QString inputColor)
{
    bool hex = inputColor.startsWith("#");
    QString inputColorFiltered = (hex ? "#" : "") + inputColor.remove(QRegularExpression("[^a-zA-Z\\d\\s]"));
    QColor color(inputColorFiltered);

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
    qDebug() << "Register session: " << sessionId;

    _currentSessionId = sessionId;

    getUser();
}

QString WIZController::getRewardMessage()
{
    return _settings.value("reward_message").toString();
}

QStringList WIZController::getBulbs() const
{
    return _bulbs;
}

bool WIZController::isLinked()
{
    return _o2.linked();
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

void WIZController::link()
{
    _o2.link();
}

void WIZController::unlink()
{
    _o2.unlink();
}

void WIZController::onLinkedChanged()
{
    qDebug() << "WIZController::linkedChanged()";
    emit linkedChanged();
}

void WIZController::linkingFailed()
{
    qDebug() << "WIZController::linkingFailed()";
}

void WIZController::linkingSucceeded()
{
    qDebug() << "WIZController::linkingSucceeded()";
}

void WIZController::onOpenBrowser(QUrl url)
{
    qDebug() << "WIZController::onOpenBrowser()";
    emit openBrowser(url);
}

void WIZController::onCloseBrowser()
{
    qDebug() << "WIZController::onCloseBrowser()";
}

void WIZController::onRequestFinished(int requestId, QNetworkReply::NetworkError error, QByteArray replyData)
{
    if (error != QNetworkReply::NoError) {
        qWarning() << "Reply error:" << error;
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(replyData);

    RequestType requestType = _pendingRequests[requestId];
    switch (requestType) {
    case GetSubscriptions:
        handleGetSubscriptions(doc);
        break;
    case AddSubscription:
        handleAddSubscription(doc);
        break;
    case DeleteSubscription:
        handleDeleteSubscription(doc);
        break;
    case GetUser:
        handleGetUser(doc);
        break;
    case None:
    default:
        break;

    }
}

void WIZController::getSubscriptionsRequest()
{
    QUrl url("https://api.twitch.tv/helix/eventsub/subscriptions");
    QNetworkRequest request(url);

    request.setRawHeader("Client-Id", _settings.value("client_id").toString().toLatin1());

    _pendingRequests[_o2Requestor.get(request)] = GetSubscriptions;
}

void WIZController::addSubscriptionsRequest()
{
    QUrl url("https://api.twitch.tv/helix/eventsub/subscriptions");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Client-Id", _settings.value("client_id").toString().toLatin1());

    QJsonDocument doc;
    QJsonObject obj;
    obj["type"] = "channel.channel_points_custom_reward_redemption.add";
    obj["version"] = "1";

    QJsonObject condition;
    condition["broadcaster_user_id"] = _broadcasterId;
    obj["condition"] = condition;

    QJsonObject transport;
    transport["method"] = "websocket";
    transport["session_id"] = _currentSessionId;
    obj["transport"] = transport;

    doc.setObject(obj);

    _pendingRequests[_o2Requestor.post(request, doc.toJson(QJsonDocument::Compact))] = AddSubscription;
}

void WIZController::deleteSubscriptionsRequest(QString id)
{
    QUrl url("https://api.twitch.tv/helix/eventsub/subscriptions?id=" + id);
    QNetworkRequest request(url);

    request.setRawHeader("Client-Id", _settings.value("client_id").toString().toLatin1());

    _pendingRequests[_o2Requestor.deleteResource(request)] = DeleteSubscription;
}

void WIZController::getUser()
{
    QUrl url("https://api.twitch.tv/helix/users");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Client-Id", _settings.value("client_id").toString().toLatin1());

    _pendingRequests[_o2Requestor.get(request)] = GetUser;
}

void WIZController::handleGetSubscriptions(QJsonDocument doc)
{
    QJsonArray data = doc["data"].toArray();
    for (QJsonValue val : data) {
        QJsonObject obj = val.toObject();
        _subscriptionsToDelete.append(obj["id"].toString());
        qDebug() << "Subscription to delete: " << _subscriptionsToDelete;
    }

    if (_subscriptionsToDelete.empty()) {
        addSubscriptionsRequest();
    } else {
        deleteSubscriptionsRequest(_subscriptionsToDelete.takeFirst());
    }
}

void WIZController::handleAddSubscription(QJsonDocument doc)
{

}

void WIZController::handleDeleteSubscription(QJsonDocument doc)
{
    if (_subscriptionsToDelete.empty()) {
        addSubscriptionsRequest();
    } else {
        deleteSubscriptionsRequest(_subscriptionsToDelete.takeFirst());
    }
}

void WIZController::handleGetUser(QJsonDocument doc)
{
    //qDebug() << doc.toJson(QJsonDocument::Compact);

    _broadcasterId = doc["data"][0]["id"].toString();
    qDebug() << "Broadcaster ID: " << _broadcasterId;

    getSubscriptionsRequest();
}
