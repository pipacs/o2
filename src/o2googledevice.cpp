#include "o2googledevice.h"

static const char *GoogleDeviceEndpoint = "https://oauth2.googleapis.com/device/code";
// Google uses a different grant type value than specified in RFC 8628
static const char *GoogleDeviceGrantType = "http://oauth.net/grant_type/device/1.0";

O2GoogleDevice::O2GoogleDevice(QObject *parent) : O2Google(parent) {
    setGrantFlow(GrantFlowDevice);
    setGrantType(GoogleDeviceGrantType);
    // O2Google() already set the correct token and refresh token URLs.
    // However, the request endpoint is different.
    setRequestUrl(GoogleDeviceEndpoint);
}
