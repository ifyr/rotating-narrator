#ifndef SubtitleModel_H
#define SubtitleModel_H

#include <QAbstractTableModel>
#include <QFont>
#include <QColor>

#include "Subtitle.h"

enum class SubtitleColumns
{
    Text = 0, // 文本内容

    Font, // 字体
    FontSize, // 文字大小
    FontColor, // 字体颜色
    EnterAnimation, // 进入动画
    MoveAnimation, // 移动动画
    CameraZoom, // 相机变焦

    ColumnCount // 总列数
};

class SubtitleModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    SubtitleModel(QObject * parent = nullptr);

    // 设置数据源
    void setSubtitles(const QList<Subtitle> & subtitles);
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;

    QList<Subtitle> GetSubtitles();

private:
    QList<Subtitle> m_Subtitles;
};

#endif // SUBTITLETABLEWIDGET_H
