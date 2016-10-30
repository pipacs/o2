#include "o2spotify.h"

static const char *SpotifyEndpoint = "https://accounts.spotify.com/authorize";
static const char *SpotifyTokenUrl = "https://accounts.spotify.com/api/token";

O2Spotify::O2Spotify(QObject *parent): O2(parent) {
    setRequestUrl(SpotifyEndpoint);
    setTokenUrl(SpotifyTokenUrl);
    setRefreshTokenUrl(SpotifyTokenUrl);
}
