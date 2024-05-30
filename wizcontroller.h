#ifndef WIZCONTROLLER_H
#define WIZCONTROLLER_H

#include "o2twitch.h"

#include <QObject>
#include <QSettings>
#include <QUdpSocket>
#include <o2.h>
#include <o2requestor.h>

class WIZController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList bulbs READ getBulbs NOTIFY bulbsChanged FINAL)
    Q_PROPERTY(bool linked READ isLinked NOTIFY linkedChanged)
public:
    enum RequestType {
        None,
        GetSubscriptions,
        AddSubscription,
        DeleteSubscription,
        GetUser,
    };

    explicit WIZController(QObject *parent = nullptr);

    Q_INVOKABLE void detectLights();
    Q_INVOKABLE void changeColor(QString inputColor, bool force = false);
    Q_INVOKABLE void registerSession(QString sessionId);
    Q_INVOKABLE QString getRewardMessage();
    Q_INVOKABLE void startRainbow();
    QStringList getBulbs() const;
    bool isLinked();

signals:
    void bulbsChanged();
    void openBrowser(QUrl url);
    void linkedChanged();
    void connectSocket();

public slots:
    void onDatagram();
    Q_INVOKABLE void link();
    Q_INVOKABLE void unlink();
    void onLinkedChanged();
    void linkingFailed();
    void linkingSucceeded();
    void onOpenBrowser(QUrl url);
    void onCloseBrowser();
    void onRequestFinished(int requestId, QNetworkReply::NetworkError error, QByteArray replyData);
    void onRainbowTimerTimeout();

private:
    void getSubscriptionsRequest();
    void addSubscriptionsRequest();
    void deleteSubscriptionsRequest(QString id);
    void getUser();
    void handleGetSubscriptions(QJsonDocument doc);
    void handleAddSubscription(QJsonDocument doc);
    void handleDeleteSubscription(QJsonDocument doc);
    void handleGetUser(QJsonDocument doc);

private:
    QUdpSocket *_udpSocket;
    QStringList _bulbs;
    QNetworkAccessManager _nm;
    O2Twitch _o2;
    O2Requestor _o2Requestor;
    QHash<int, RequestType> _pendingRequests;
    QString _broadcasterId = "";
    QStringList _subscriptionsToDelete;
    QString _currentSessionId = "";
    QSettings _settings;
    int _subscriptionsCount = 0;
    QTimer _rainbowTimer;
    int _rainbowCounter = 0;
};

#endif // WIZCONTROLLER_H
