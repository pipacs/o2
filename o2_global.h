#ifndef O2_GLOBAL_H
#define O2_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(O2_LIBRARY)
#  define O2_EXPORT Q_DECL_EXPORT
#else
#  define O2_EXPORT Q_DECL_IMPORT
#endif

#endif // O2_GLOBAL_H
