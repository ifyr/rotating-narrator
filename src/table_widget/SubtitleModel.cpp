#include "SubtitleModel.h"

SubtitleModel::SubtitleModel(QObject * parent)
  : QAbstractTableModel(parent)
{ }

void SubtitleModel::setSubtitles(const QList<Subtitle> & subtitles)
{
    beginResetModel();
    m_Subtitles = subtitles;
    endResetModel();
}

QList<Subtitle> SubtitleModel::GetSubtitles()
{
    return m_Subtitles;
}

int SubtitleModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return m_Subtitles.size();
}

int SubtitleModel::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(SubtitleColumns::ColumnCount);
}

QVariant SubtitleModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid() || (role != Qt::DisplayRole)) {
        return QVariant();
    }

    const Subtitle & subtitle = m_Subtitles.at(index.row());

    if ((role == Qt::DisplayRole) || (role == Qt::EditRole)) {
        switch ((SubtitleColumns)index.column()) {
        case SubtitleColumns::Text: // 文本内容
            return subtitle.text;

        case SubtitleColumns::Font: // 字体名称
            return subtitle.font.family();

        case SubtitleColumns::FontSize: // 字体大小
            return subtitle.font.pointSize(); // 返回数字

        case SubtitleColumns::FontColor: // 字体颜色
            return subtitle.fontColor.name(); // 返回颜色名称

        case SubtitleColumns::EnterAnimation: // 进入动画
            return AnimationToString(subtitle.enterAnimation);

        case SubtitleColumns::MoveAnimation: // 移动动画
            return AnimationToString(subtitle.moveAnimation);

        case SubtitleColumns::CameraZoom: // 相机变焦
            return subtitle.cameraZoom;
        }
    }
    return QVariant();
}

bool SubtitleModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (!index.isValid() || (role != Qt::EditRole)) {
        return false;
    }
    Subtitle & subtitle = m_Subtitles[index.row()];

    switch ((SubtitleColumns)index.column()) {
    case SubtitleColumns::Text:
        subtitle.text = value.toString();
        break;

    case SubtitleColumns::Font:
        subtitle.font = QFont(value.toString(), subtitle.font.pointSize());
        break;

    case SubtitleColumns::FontSize:
        subtitle.font.setPointSize(value.toInt());
        break;

    case SubtitleColumns::FontColor:
        subtitle.fontColor = value.value<QColor>();
        break;

    case SubtitleColumns::EnterAnimation:
        subtitle.enterAnimation = static_cast<EnterAnimationType>(value.toInt());
        break;

    case SubtitleColumns::MoveAnimation:
        subtitle.moveAnimation = static_cast<MoveAnimationType>(value.toInt());
        break;

    case SubtitleColumns::CameraZoom:
        subtitle.cameraZoom = value.toDouble();
        break;

    default:
        return false;
    }

    emit dataChanged(index, index);
    return true;
}

QVariant SubtitleModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role != Qt::DisplayRole)) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        switch (static_cast<SubtitleColumns>(section)) {
        case SubtitleColumns::Text:
            return u8"文本";

        case SubtitleColumns::Font:
            return u8"字体";

        case SubtitleColumns::FontSize:
            return u8"尺寸";

        case SubtitleColumns::FontColor:
            return u8"颜色";

        case SubtitleColumns::EnterAnimation:
            return u8"进入动画";

        case SubtitleColumns::MoveAnimation:
            return u8"移动动画";

        case SubtitleColumns::CameraZoom:
            return u8"相机变焦";

        default:
            return QVariant();
        }
    } else if (orientation == Qt::Vertical) {
        // 返回行号（从1开始）
        return QString::number(section + 1);
    }
}

Qt::ItemFlags SubtitleModel::flags(const QModelIndex & index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    if ((SubtitleColumns)index.column() == SubtitleColumns::Text) {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}
