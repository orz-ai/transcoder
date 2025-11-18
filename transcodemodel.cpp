#include "transcodemodel.h"
#include <QApplication>
#include <QStyle>

TranscodeModel::TranscodeModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    // 创建状态图标
    QStyle *style = QApplication::style();
    m_successIcon = style->standardIcon(QStyle::SP_DialogApplyButton);
    m_failedIcon = style->standardIcon(QStyle::SP_DialogCancelButton);
    m_pendingIcon = style->standardIcon(QStyle::SP_MediaPlay);
    m_processingIcon = style->standardIcon(QStyle::SP_ArrowRight);
}

int TranscodeModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_records.size();
}

int TranscodeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return ColumnCount;
}

QVariant TranscodeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_records.size())
        return QVariant();

    const TranscodeRecord &record = m_records.at(index.row());

    switch (role)
    {
    case Qt::DisplayRole:
        switch (index.column())
        {
        case Index:
            return index.row() + 1; // 序号从1开始
        case Status:
            switch (record.status)
            {
            case TranscodeStatus::Pending:
                return QString::fromLocal8Bit("等待中");
            case TranscodeStatus::Processing:
                return QString::fromLocal8Bit("转码中");
            case TranscodeStatus::Success:
                return QString::fromLocal8Bit("成功");
            case TranscodeStatus::Failed:
                return QString::fromLocal8Bit("失败");
            }
            break;
        case SourcePath:
            return record.sourcePath;
        case TargetPath:
            return record.targetPath;
        case Progress:
            if (record.status == TranscodeStatus::Processing)
            {
                return QString("%1%").arg(record.progress);
            }
            else if (record.status == TranscodeStatus::Success)
            {
                return QString("100%");
            }
            else if (record.status == TranscodeStatus::Failed)
            {
                return QString::fromLocal8Bit("失败");
            }
            return QString("-");
        }
        break;

    case Qt::DecorationRole:
        if (index.column() == Status)
        {
            switch (record.status)
            {
            case TranscodeStatus::Pending:
                return m_pendingIcon;
            case TranscodeStatus::Processing:
                return m_processingIcon;
            case TranscodeStatus::Success:
                return m_successIcon;
            case TranscodeStatus::Failed:
                return m_failedIcon;
            }
        }
        break;

    case Qt::ToolTipRole:
        if (index.column() == Status && record.status == TranscodeStatus::Failed)
        {
            return record.errorMessage;
        }
        else if (index.column() == SourcePath)
        {
            return record.sourcePath;
        }
        else if (index.column() == TargetPath)
        {
            return record.targetPath;
        }
        break;

    case Qt::TextAlignmentRole:
        if (index.column() == Index || index.column() == Progress)
        {
            return Qt::AlignCenter;
        }
        break;

    case Qt::BackgroundRole:
        switch (record.status)
        {
        case TranscodeStatus::Success:
            return QColor(240, 255, 240); // 浅绿色
        case TranscodeStatus::Failed:
            return QColor(255, 240, 240); // 浅红色
        case TranscodeStatus::Processing:
            return QColor(240, 248, 255); // 浅蓝色
        default:
            break;
        }
        break;
    }

    return QVariant();
}

bool TranscodeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_records.size() || role != Qt::EditRole)
        return false;

    TranscodeRecord &record = m_records[index.row()];

    switch (index.column())
    {
    case TargetPath:
        record.targetPath = value.toString();
        emit dataChanged(index, index);
        return true;
    default:
        break;
    }

    return false;
}

QVariant TranscodeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    switch (section)
    {
    case Index:
        return QString::fromLocal8Bit("序号");
    case Status:
        return QString::fromLocal8Bit("状态");
    case SourcePath:
        return QString::fromLocal8Bit("源路径");
    case TargetPath:
        return QString::fromLocal8Bit("目标路径");
    case Progress:
        return QString::fromLocal8Bit("进度");
    }

    return QVariant();
}

void TranscodeModel::addRecord(const QString &fileName, const QString &sourcePath, const QString &targetPath)
{
    beginInsertRows(QModelIndex(), m_records.size(), m_records.size());
    m_records.append(TranscodeRecord(fileName, sourcePath, targetPath));
    endInsertRows();
}

void TranscodeModel::updateRecordStatus(const QString &fileName, TranscodeStatus status, const QString &errorMessage)
{
    int index = findRecordIndex(fileName);
    if (index != -1)
    {
        m_records[index].status = status;
        if (!errorMessage.isEmpty())
        {
            m_records[index].errorMessage = errorMessage;
        }

        QModelIndex topLeft = createIndex(index, 0);
        QModelIndex bottomRight = createIndex(index, ColumnCount - 1);
        emit dataChanged(topLeft, bottomRight);
    }
}

void TranscodeModel::updateRecordProgress(const QString &fileName, int progress)
{
    int index = findRecordIndex(fileName);
    if (index != -1)
    {
        m_records[index].progress = progress;
        m_records[index].status = TranscodeStatus::Processing;

        QModelIndex progressIndex = createIndex(index, Progress);
        QModelIndex statusIndex = createIndex(index, Status);
        emit dataChanged(statusIndex, progressIndex);
    }
}

void TranscodeModel::clearRecords()
{
    beginResetModel();
    m_records.clear();
    endResetModel();
}

int TranscodeModel::findRecordIndex(const QString &fileName) const
{
    for (int i = 0; i < m_records.size(); ++i)
    {
        if (m_records.at(i).fileName == fileName)
        {
            return i;
        }
    }
    return -1;
}