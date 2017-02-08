#ifndef O2GOOGLE_H
#define O2GOOGLE_H

#include "o0export.h"
#include "o2.h"

/// Google Account dialect of OAuth 2.0
class O0_EXPORT O2Google : public O2 {
    Q_OBJECT

public:
    explicit O2Google(QObject *parent = 0);
};

#endif // O2GOOGLE_H
