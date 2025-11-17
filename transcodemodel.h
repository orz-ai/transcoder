#ifndef TRANSCODEMODEL_H
#define TRANSCODEMODEL_H

#include <QAbstractTableModel>
#include <QIcon>

enum class TranscodeStatus
{
    Pending,    // 等待中
    Processing, // 转码中
    Success,    // 成功
    Failed      // 失败
};

struct TranscodeRecord
{
    QString fileName;
    QString sourcePath;
    QString targetPath;
    TranscodeStatus status;
    QString errorMessage;
    int progress;

    TranscodeRecord(const QString &file, const QString &source, const QString &target)
        : fileName(file), sourcePath(source), targetPath(target),
          status(TranscodeStatus::Pending), progress(0) {}
};

class TranscodeModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column
    {
        Index = 0,
        Status,
        SourcePath,
        TargetPath,
        Progress,
        ColumnCount
    };

    explicit TranscodeModel(QObject *parent = nullptr);

    // QAbstractTableModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // 添加和更新记录
    void addRecord(const QString &fileName, const QString &sourcePath, const QString &targetPath);
    void updateRecordStatus(const QString &fileName, TranscodeStatus status, const QString &errorMessage = QString());
    void updateRecordProgress(const QString &fileName, int progress);
    void clearRecords();

private:
    QList<TranscodeRecord> m_records;
    QIcon m_successIcon;
    QIcon m_failedIcon;
    QIcon m_pendingIcon;
    QIcon m_processingIcon;

    int findRecordIndex(const QString &fileName) const;
};

#endif // TRANSCODEMODEL_H