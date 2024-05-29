#ifndef O2TWITCH_H
#define O2TWITCH_H

#include <o2.h>
#include <QObject>

class O2Twitch : public O2
{
    Q_OBJECT
public:
    explicit O2Twitch(QObject *parent = nullptr);

    void setRefreshToken(const QString &v);
};

#endif // O2TWITCH_H
