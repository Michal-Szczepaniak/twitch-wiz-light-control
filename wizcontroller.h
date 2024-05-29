#ifndef WIZCONTROLLER_H
#define WIZCONTROLLER_H

#include <QObject>
#include <QUdpSocket>
#include <TwitchQt>

class WIZController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList bulbs READ getBulbs NOTIFY bulbsChanged FINAL)
public:
    explicit WIZController(QObject *parent = nullptr);

    Q_INVOKABLE void detectLights();
    Q_INVOKABLE void changeColor(QString inputColor);
    Q_INVOKABLE void registerSession(QString sessionId);
    QStringList getBulbs() const;

signals:
    void bulbsChanged();

public slots:
    void onDatagram();

private:
    QUdpSocket *_udpSocket;
    QStringList _bulbs;
    Twitch::Api _api;
};

#endif // WIZCONTROLLER_H
