#ifndef O1FLICKR_H
#define O1FLICKR_H

#include "o1.h"

/// Flickr authenticator.
class O1Flickr: public O1 {
    Q_OBJECT

public:
    explicit O1Flickr(QObject *parent = 0): O1(parent) {
        setRequestTokenUrl(QUrl("http://www.flickr.com/services/oauth/request_token"));
        setAuthorizeUrl(QUrl("http://www.flickr.com/services/oauth/authorize?perms=write"));
        setAccessTokenUrl(QUrl("http://www.flickr.com/services/oauth/access_token"));
    }
};

#endif // O1FLICKR_H
