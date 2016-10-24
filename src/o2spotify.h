#ifndef O2SPOTIFY_H
#define O2SPOTIFY_H

#include "o2.h"

/// Spotify's dialect of OAuth 2.0
class O2Spotify: public O2 {
    Q_OBJECT

public:
    explicit O2Spotify(QObject *parent = 0);
};

#endif // O2SPOTIFY_H
