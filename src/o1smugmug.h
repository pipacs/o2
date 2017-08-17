#ifndef O1SMUGMUG_H
#define O1SMUGMUG_H

#include "o0export.h"
#include "o1.h"

/// SmugMug OAuth 1.0 client
class O0_EXPORT O1SmugMug: public O1 {
    Q_OBJECT
    Q_ENUMS(Access)
    Q_ENUMS(Permissions)

public:
    enum Access {
        AccessPublic,
        AccessFull
    };

    enum Permissions {
        PermissionsRead,
        PermissionsAdd,
        PermissionsModify
    };

    Q_INVOKABLE void initAuthorizationUrl(Access access, Permissions permissions);

    explicit O1SmugMug(QObject *parent = 0, QNetworkAccessManager *manager = 0, O0AbstractStore *store = 0);
};

#endif // O1SMUGMUG_H
