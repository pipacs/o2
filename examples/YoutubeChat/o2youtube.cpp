#include "o2youtube.h"

namespace youtube
{
static const char *YoutubeScope = "https://www.googleapis.com/auth/youtube.readonly";
static const char *YoutubeEndpoint = "https://accounts.google.com/o/oauth2/v2/auth";
static const char *YoutubeTokenUrl = "https://www.googleapis.com/oauth2/v4/token";
static const char *YoutubeRefreshUrl = "https://www.googleapis.com/oauth2/v4/token";


O2Youtube::O2Youtube(QObject *parent): O2(parent)
{
    setRequestUrl(YoutubeEndpoint);
    setTokenUrl(YoutubeTokenUrl);
    setRefreshTokenUrl(YoutubeRefreshUrl);
    setScope(YoutubeScope);
    //setLocalPort(getRandomUnusedPort());
}


quint16 O2Youtube::getRandomUnusedPort()
{
    //listen()
    return 0;
}
}
