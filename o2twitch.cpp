#include "o2twitch.h"

static const char *GftEndpoint = "https://id.twitch.tv/oauth2/authorize";

O2Twitch::O2Twitch(QObject *parent)
    : O2{parent}
{
    setRequestUrl(GftEndpoint);
    setGrantFlow(O2::GrantFlowImplicit);
    setLocalhostPolicy("http://localhost:%1/");
}
