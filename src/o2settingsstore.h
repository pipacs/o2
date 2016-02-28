#ifndef O2SETTINGSSTORE_H
#define O2SETTINGSSTORE_H

#include <QSettings>
#include <QString>

#include "o2abstractstore.h"
#include "o2simplecrypt.h"

/// Persistent storage for authentication tokens, using QSettings.
class O2SettingsStore: public O2AbstractStore {
    Q_OBJECT

public:
    /// Constructor
    explicit O2SettingsStore(const QString &encryptionKey, QObject *parent = 0);

    /// Construct with an explicit QSettings instance
    explicit O2SettingsStore(QSettings *settings, const QString &encryptionKey, QObject *parent = 0);

    /// Group key prefix
    Q_PROPERTY(QString groupKey READ groupKey WRITE setGroupKey NOTIFY groupKeyChanged)
    QString groupKey() const;
    void setGroupKey(const QString &groupKey);

    /// Get a string value for a key
    QString value(const QString &key, const QString &defaultValue = QString());

    /// Set a string value for a key
    void setValue(const QString &key, const QString &value);

signals:
    // Property change signals
    void groupKeyChanged();

protected:
    QSettings* settings_;
    QString groupKey_;
    O2SimpleCrypt crypt_;
};

#endif // O2SETTINGSSTORE_H
