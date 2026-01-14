#include "versiontreemodel.h"
#include <QFileInfo>

VersionTreeModel::VersionTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

void VersionTreeModel::setVersionsData(const QVector<FileNode>& files)
{
    beginResetModel();
    m_files = files;
    endResetModel();
}

void VersionTreeModel::setAllVersions(
    const QMap<QString, QList<VersionInfo>>& data)
{
    beginResetModel();
    m_files.clear();

    for (auto it = data.begin(); it != data.end(); ++it) {
        FileNode file;
        file.m_filePath = it.key();

        for (const VersionInfo& info : it.value()) {
            VersionNode node;
            node.m_versionId = info.versionId;
            node.m_time = info.timestamp;
            file.m_versions.push_back(node);
        }
        m_files.push_back(file);
    }

    endResetModel();
}

QModelIndex VersionTreeModel::index(
    int row, int column,
    const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid()) {
        // 文件节点，internalId = -1
        return createIndex(row, column, -1);
    }

    // 版本节点，internalId = 文件行号
    return createIndex(row, column, parent.row());
}

QModelIndex VersionTreeModel::parent(
    const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    const qint64 id = index.internalId();
    if (id == -1)
        return QModelIndex(); // 文件节点没有父

    // 版本节点 → 文件节点
    return createIndex(static_cast<int>(id), 0, -1);
}

int VersionTreeModel::rowCount(
    const QModelIndex& parent) const
{
    if (!parent.isValid())
        return m_files.size();

    // 文件节点 → 版本数量
    if (parent.internalId() == -1) {
        const int row = parent.row();
        if (row < 0 || row >= m_files.size())
            return 0;
        return m_files[row].m_versions.size();
    }

    return 0;
}

int VersionTreeModel::columnCount(
    const QModelIndex&) const
{
    return 1;
}

QVariant VersionTreeModel::data(
    const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    // 文件节点
    if (index.internalId() == -1) {
        return QFileInfo(
            m_files[index.row()].m_filePath).fileName();
    }

    // 版本节点
    const int fileRow = static_cast<int>(index.internalId());
    const int versionRow = index.row();

    const VersionNode& v =
        m_files[fileRow].m_versions[versionRow];

    return QString("%1  %2")
        .arg(v.m_versionId.left(8))
        .arg(v.m_time.toString("yyyy-MM-dd HH:mm:ss"));
}

bool VersionTreeModel::isFileNode(
    const QModelIndex& index) const
{
    return index.isValid() && index.internalId() == -1;
}

VersionInfo VersionTreeModel::versionAt(
    const QModelIndex& index) const
{
    if (!index.isValid() || isFileNode(index))
        return VersionInfo{};

    const int fileRow = static_cast<int>(index.internalId());
    const int versionRow = index.row();

    const FileNode& file = m_files[fileRow];
    const VersionNode& v = file.m_versions[versionRow];

    VersionInfo info;
    info.filePath = file.m_filePath;
    info.versionId = v.m_versionId;
    info.timestamp = v.m_time;
    return info;
}

VersionInfo VersionTreeModel::versionInfo(
    const QModelIndex& index) const
{
    return versionAt(index);
}
