#include <cstddef>

#include "o1smugmug.h"

O1SmugMug::O1SmugMug(QObject *parent, QNetworkAccessManager *manager, O0AbstractStore *store)
    : O1(parent, manager, store)
{
   setRequestTokenUrl(QUrl("https://secure.smugmug.com/services/oauth/1.0a/getRequestToken"));
   setAccessTokenUrl(QUrl("https://secure.smugmug.com/services/oauth/1.0a/getAccessToken"));
}

void O1SmugMug::initAuthorizationUrl(Access access, Permissions permissions) {
    static const char * const accessStrings[] = {
        "Public",
        "Full"
    };
    const std::size_t accessStringsSize = sizeof(accessStrings) / sizeof(accessStrings[0]);
    Q_ASSERT(access >= 0 && static_cast<std::size_t>(access) < accessStringsSize
                && "Unsupported SmugMug authorization access!");

    static const char * const permissionsStrings[] = {
        "Read",
        "Add",
        "Modify"
    };
    const std::size_t permissionsStringsSize = sizeof(permissionsStrings) / sizeof(permissionsStrings[0]);
    Q_ASSERT(permissions >= 0 && static_cast<std::size_t>(permissions) < permissionsStringsSize
                && "Unsupported SmugMug authorization permissions!");

    setAuthorizeUrl(QUrl(QString("https://secure.smugmug.com/services/oauth/1.0a/authorize")
                         + "?Access=" + accessStrings[access]
                         + "&Permissions=" + permissionsStrings[permissions]));
}
