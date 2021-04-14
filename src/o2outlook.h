#ifndef O2OUTLOOK_H
#define O2OUTLOOK_H

#include <o2/o2.h>
#include <o2/o2skydrive.h>

/// Outlook's dialect of OAuth 2.0
class O2Outlook: public O2Skydrive
{
    Q_OBJECT
public:
    explicit O2Outlook(QObject *parent = nullptr);

public:
    QUrl redirectUrl();
};

#endif // O2OUTLOOK_H
