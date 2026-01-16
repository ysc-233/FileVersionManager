#ifndef VERSIONTREEMODEL_H
#define VERSIONTREEMODEL_H
#pragma once
#include <QAbstractTableModel>
#include <QDateTime>
#include "core/versioninfo.h"

struct VersionNode {
    QString m_versionId;
    QDateTime m_time;
    qint64 m_fileSize;
    bool operator==(const VersionNode& other) const
    {
        return m_versionId == other.m_versionId;
    }
};

struct FileNode {
    QString m_filePath;
    QVector<VersionNode> m_versions;
};

class VersionTreeModel : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit VersionTreeModel(QObject* parent = nullptr);

    void setAllVersions(const QMap<QString, QList<VersionInfo>>& data);
    void setCurrentVersions(const QMap<QString, QString>& current);

    VersionInfo versionAt(const QModelIndex& index) const;

    QModelIndex index(int row, int column,const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex&) const override;
    QVariant data(const QModelIndex& index, int role) const override;

private:
    QVector<FileNode> m_files;
    QMap<QString, QString> m_currentVersions; // filePath → versionId
};
#endif // VERSIONTREEMODEL_H
