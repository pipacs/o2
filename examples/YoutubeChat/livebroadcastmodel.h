#ifndef LIVEBOADCASTMODEL_H
#define LIVEBOADCASTMODEL_H

#include <QObject>
#include <QVariantMap>
#include <QString>
#include <QAbstractListModel>

namespace youtube
{
class LiveBroadcastModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        RoleLiveChatId = Qt::UserRole + 1,
    };

    LiveBroadcastModel(QObject *parent = 0);

    /// Clear all broadcasts
    void clearBroadcasts();

    /// Add a broadcast
    void addBroadcast(QVariantMap tweet);

    /// Get number of broadcasts
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    /// Access a broadcasts
    QVariant data(const QModelIndex & index, int role = Roles::RoleLiveChatId) const;

    /// Get role names
    QHash<int, QByteArray>roleNames() const;

protected:
    QList<QVariantMap> broadcasts;

};
}
#endif // LIVEBOADCASTMODEL_H
