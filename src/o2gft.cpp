#include "o2gft.h"
#include "o2google.h"

static const char *GftScope = "https://www.googleapis.com/auth/fusiontables";

O2Gft::O2Gft(QObject *parent, bool inUseExternalInterceptor): O2Google(parent, inUseExternalInterceptor) {
    setScope(GftScope);
}
