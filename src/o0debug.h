
#ifndef O2DEBUG_H
#define O2DEBUG_H

#include <QDebug>

#if QT_VERSION >= 0x050000
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(O0_DEBUG_LOG)

#define o0debug() qCDebug(O0_DEBUG_LOG)
#define o0warning() qCWarning(O0_DEBUG_LOG)

#else

#define o0debug() qDebug()
#define o0warning() qWarning()

#endif



#endif // O2DEBUG_H
