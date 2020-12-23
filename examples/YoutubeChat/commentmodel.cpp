#include "commentmodel.h"

namespace youtube
{
CommentModel::CommentModel(QObject *parent): QAbstractListModel(parent)
{

}

QHash<int, QByteArray> CommentModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[RoleCommentMessage] = "Message properties";
    roles[RoleCommentAuthor] = "Message author properties";
    return roles;
}

void CommentModel::clearComments()
{
    beginRemoveRows(QModelIndex(), 0, std::max(0,rowCount() - 1));
    comments.clear();
    endRemoveRows();
}

void CommentModel::addComment(QVariantMap comment)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    comments.append(comment);
    endInsertRows();
}

int CommentModel::rowCount(const QModelIndex &parent) const
{
    return comments.count();
}

QVariant CommentModel::data(const QModelIndex &index, int role) const
{
    QVariant result;
    QVariantMap comment = comments[index.row()];
    switch (role) {
    case Roles::RoleCommentMessage:
        result = comment["snippet"];

        /*foreach (const QVariant &v, broadcast.keys()) {
                qWarning() << v.toString() << " : " << broadcast[v.toString()];
        }*/
        break;
    case Roles::RoleCommentAuthor:
        result = comment["authorDetails"];
        break;
    default:
        result = comment;
    }
    return result;
}

}
