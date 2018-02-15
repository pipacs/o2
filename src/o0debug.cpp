
#include "o0debug.h"

#if QT_VERSION >= 0x050000

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
Q_LOGGING_CATEGORY(O0_DEBUG_LOG, "o0.debug", QtInfoMsg)
#else
Q_LOGGING_CATEGORY(O0_DEBUG_LOG, "o0.debug")
#endif

#endif
