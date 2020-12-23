#include <QDebug>
#include "livebroadcastmodel.h"

namespace youtube
{
LiveBroadcastModel::LiveBroadcastModel(QObject *parent) : QAbstractListModel(parent)
{   
}

QHash<int, QByteArray> LiveBroadcastModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[RoleLiveChatId] = "RoleLiveChatId";
    return roles;
}

void LiveBroadcastModel::addBroadcast(QVariantMap broadcast) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    broadcasts.append(broadcast);
    endInsertRows();
}

void LiveBroadcastModel::clearBroadcasts() {
    beginRemoveRows(QModelIndex(), 0, std::max(0,rowCount() - 1));
    broadcasts.clear();
    endRemoveRows();
}

int LiveBroadcastModel::rowCount(const QModelIndex &) const {
    return broadcasts.count();
}

QVariant LiveBroadcastModel::data(const QModelIndex &index, int role) const {
    QVariant result;
    QVariantMap broadcast = broadcasts[index.row()];
    switch (role) {
    case LiveBroadcastModel::RoleLiveChatId:    
        result = broadcast["snippet"].toMap()["liveChatId"];

        /*foreach (const QVariant &v, broadcast.keys()) {
                qWarning() << v.toString() << " : " << broadcast[v.toString()];
        }*/
        break;
    default:
        result = broadcast;
    }
    return result;
}
}

