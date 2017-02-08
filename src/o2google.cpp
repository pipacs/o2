#include "o2google.h"

// All scopes can be found here: https://developers.google.com/identity/protocols/googlescopes
// Using google plus scope for example
static const char *PlusScope = "https://www.googleapis.com/auth/plus.me";
static const char *GoogleEndpoint = "https://accounts.google.com/o/oauth2/auth";
static const char *GoogleTokenUrl = "https://accounts.google.com/o/oauth2/token";
static const char *GoogleRefreshUrl = "https://accounts.google.com/o/oauth2/token";

O2Google::O2Google(QObject *parent): O2(parent) {
    setRequestUrl(GoogleEndpoint);
    setTokenUrl(GoogleTokenUrl);
    setRefreshTokenUrl(GoogleRefreshUrl);
    setScope(PlusScope);
}
