#ifndef O2HUBIC_H
#define O2HUBIC_H

#include "o2.h"

class O2Hubic: public O2 {
    Q_OBJECT

public:


public:
    /// Constructor.
    /// @param  parent  Parent object.
    explicit O2Hubic(QObject *parent = 0);

public slots:
    /// Authenticate.
    Q_INVOKABLE virtual void link();

};

#endif // O2_HUBIC
