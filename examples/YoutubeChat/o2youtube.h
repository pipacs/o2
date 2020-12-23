#ifndef O2YOUTUBE_H
#define O2YOUTUBE_H

#include "o0export.h"
#include "o2.h"

namespace youtube
{
class O2Youtube:public O2
{
    Q_OBJECT
public:
    O2Youtube(QObject *parent);
private:
    static quint16 getRandomUnusedPort();

};
}
#endif // O2YOUTUBE_H
