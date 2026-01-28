#include "versiontreemodel.h"
#include <QFileInfo>
#include <QFont>
#include <QBrush>
VersionTreeModel::VersionTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

void VersionTreeModel::setAllVersions(const QMap<QString, QList<VersionInfo>>& data)
{
    beginResetModel();
    m_files.clear();

    for (auto it = data.begin(); it != data.end(); ++it) {
        FileNode file;
        file.m_filePath = it.key();
        file.m_versions = buildDisplayVersions(it.value());
        if (!file.m_versions.isEmpty())
            m_files.push_back(file);
    }
    endResetModel();
}

QModelIndex VersionTreeModel::index(int row, int column,const QModelIndex& parent) const
{
    if (column != 0 || row < 0)
        return QModelIndex();

    // 根节点 → 文件
    if (!parent.isValid())
    {
        if (row >= m_files.size())
            return QModelIndex();

        return createIndex(
            row, column,
            const_cast<FileNode*>(&m_files[row])
        );
    }

    // 文件 → 版本
    FileNode* file =
        static_cast<FileNode*>(parent.internalPointer());

    if (!file || row >= file->m_versions.size())
        return QModelIndex();

    return createIndex(
        row, column,
        const_cast<VersionNode*>(&file->m_versions[row])
    );
}

QModelIndex VersionTreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    void* ptr = index.internalPointer();

    // ---------- 如果是 FileNode，parent 是无效 ----------
    for (int i = 0; i < m_files.size(); ++i)
    {
        if (ptr == &m_files[i])
            return QModelIndex();
    }

    // ---------- 否则一定是 VersionNode，找所属 FileNode ----------
    for (int i = 0; i < m_files.size(); ++i)
    {
        auto& file = m_files[i];
        for (auto& v : file.m_versions)
        {
            if (ptr == &v)
            {
                return createIndex(
                    i, 0,
                    const_cast<FileNode*>(&m_files[i])
                );
            }
        }
    }

    return QModelIndex();
}



int VersionTreeModel::rowCount(const QModelIndex& parent) const
{
    // 根 → 文件数量
    if (!parent.isValid())
        return m_files.size();

    // 版本节点 → 不能再展开
    if (parent.parent().isValid())
        return 0;

    // 文件节点 → 版本数量
    FileNode* file =
        static_cast<FileNode*>(parent.internalPointer());

    if (!file)
        return 0;

    return file->m_versions.size();
}


int VersionTreeModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant VersionTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    // ---------- 文件节点 ----------
    if (!index.parent().isValid())
    {
        if (role == Qt::DisplayRole)
        {
            const FileNode* file = static_cast<const FileNode*>(index.internalPointer());
            return QFileInfo(file->m_filePath).fileName();
        }
        return {};
    }

    // ---------- 版本节点 ----------
    const VersionNode* v = static_cast<const VersionNode*>(index.internalPointer());
    const FileNode* f = static_cast<const FileNode*>(index.parent().internalPointer());

    if (!v || !f)
        return {};

    if (role == Qt::DisplayRole)
    {
        return QString("%1  %2")
            .arg(v->m_versionId.left(8))
            .arg(v->m_time.toString("yyyy-MM-dd HH:mm:ss"));
    }
    // Deleted → 红色
   if (role == Qt::ForegroundRole) {
       if (v->m_state == FileState::Deleted) {
           return QBrush(QColor(200, 60, 60)); // 暗红色
       }
   }

    const QString current = m_currentVersions.value(f->m_filePath);

    if (!current.isEmpty() && current == v->m_versionId)
    {
        if (role == Qt::FontRole)
        {
            QFont font;
            font.setBold(true);
            return font;
        }
        if (role == Qt::ForegroundRole)
        {
            return QColor(Qt::darkGreen);
        }
    }

    return {};
}

QVector<VersionNode> VersionTreeModel::buildDisplayVersions(const QList<VersionInfo>& versions) const
{
    QVector<VersionNode> result;
    if (versions.isEmpty())
        return result;

    // ---------- 判断文件最终状态 ----------
    const VersionInfo& last = versions.last();
    const bool fileDeleted = (last.state == FileState::Deleted);

    // ---------- 按 versionId 取最后一次事件 ----------
    QMap<QString, const VersionInfo*> latestByVersion;

    for (const auto& v : versions) {
        auto it = latestByVersion.find(v.versionId);
        if (it == latestByVersion.end() || v.timestamp > it.value()->timestamp) {
            latestByVersion[v.versionId] = &v;
        }
    }

    // ---------- 构建显示列表 ----------
    for (auto it = latestByVersion.begin(); it != latestByVersion.end(); ++it) {
        const VersionInfo* v = it.value();

        // 关键过滤逻辑
        if (!fileDeleted && v->state == FileState::Deleted) {
            // 文件已恢复 → 不显示任何 Deleted
            continue;
        }

        if (fileDeleted && v->state == FileState::Deleted) {
            // 文件已删除 → 只保留最后一个 Deleted
            // （下面会通过时间排序 + 去重自然保证）
        }

        VersionNode node;
        node.m_versionId = v->versionId;
        node.m_time      = v->timestamp;
        node.m_fileSize  = v->fileSize;
        node.m_state     = v->state;

        result.push_back(node);
    }

    // ---------- 时间排序 ----------
    std::sort(result.begin(), result.end(),
              [](const VersionNode& a, const VersionNode& b) {
                  return a.m_time < b.m_time;
              });

    // ---------- 如果文件已删除，只保留最后一个 Deleted ----------
    if (fileDeleted) {
        for (int i = result.size() - 1; i >= 0; --i) {
            if (result[i].m_state == FileState::Deleted) {
                result = { result[i] };
                break;
            }
        }
    }

    return result;
}

VersionInfo VersionTreeModel::versionAt(const QModelIndex& index) const
{
    if (!index.isValid() || !index.parent().isValid())
        return VersionInfo{};

    const VersionNode* v =
        static_cast<const VersionNode*>(index.internalPointer());
    const FileNode* f =
        static_cast<const FileNode*>(index.parent().internalPointer());

    if (!v || !f)
        return VersionInfo{};

    VersionInfo info;
    info.filePath  = f->m_filePath;
    info.versionId = v->m_versionId;
    info.timestamp = v->m_time;
    info.fileSize = v->m_fileSize;
    return info;
}

void VersionTreeModel::setCurrentVersions(const QMap<QString, QString> &current)
{
    m_currentVersions = current;

    if (m_files.isEmpty())
        return;

    QModelIndex topLeft = index(0, 0, QModelIndex());
    if (!topLeft.isValid())
        return;

    QModelIndex bottomRight = index(m_files.size() - 1, 0, QModelIndex());

    emit dataChanged(topLeft,bottomRight,
        { Qt::DisplayRole, Qt::FontRole, Qt::ForegroundRole }
    );
}
