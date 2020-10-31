#ifndef O2GOOGLEDEVICE_H
#define O2GOOGLEDEVICE_H

#include "o0export.h"
#include "o2google.h"

/// "Google Sign-In for TVs and Devices",
/// A dialect of RFC 8628: OAuth 2.0 Device Authorization Grant
class O0_EXPORT O2GoogleDevice : public O2Google {
    Q_OBJECT

public:
    explicit O2GoogleDevice(QObject *parent = 0);
};

#endif // O2GOOGLEDEVICE_H
