#include "ComboBoxDelegate.h"

#include <QEvent>
#include <QComboBox>
#include <QFontDatabase>
#include <QPainter>

#include "SubtitleModel.h"

ComboBoxDelegate::ComboBoxDelegate(QObject * parent)
  : QStyledItemDelegate(parent)
{ }

QWidget * ComboBoxDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option,
                                         const QModelIndex & index) const
{
    EmptyTextComboBox * comboBox = new EmptyTextComboBox(parent);
    switch ((SubtitleColumns)index.column()) {
    case SubtitleColumns::Font:
        comboBox->addItems(QFontDatabase().families());
        break;

    case SubtitleColumns::EnterAnimation:
        for (int i = 0; i < 10; i++) {
            comboBox->addItem(AnimationToString((EnterAnimationType)i));
        }
        break;

    case SubtitleColumns::MoveAnimation:
        comboBox->addItems({ AnimationToString(MoveAnimationType::MoveUp),
                             AnimationToString(MoveAnimationType::RotateClockwise),
                             AnimationToString(MoveAnimationType::RotateCounterclockwise) });
        break;

    default:
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    comboBox->installEventFilter(const_cast<ComboBoxDelegate *>(this));
    connect(comboBox, &QComboBox::currentIndexChanged, this,
            [this, comboBox, index]() {
                if (comboBox->currentIndex() != -1) {
                    QAbstractItemModel * model = const_cast<QAbstractItemModel *>(index.model());
                    this->setModelData(comboBox, model, index);
                }
            });
    return comboBox;
}

void ComboBoxDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    QComboBox * comboBox = qobject_cast<QComboBox *>(editor);
    if (comboBox) {
        comboBox->blockSignals(true);

        QString currentText = comboBox->currentText();
        QString newText = index.data(Qt::DisplayRole).toString();

        comboBox->setCurrentText(newText);
        comboBox->blockSignals(false);

        if (!comboBox->isVisible()) {
            comboBox->showPopup();
        }
    }
}

void ComboBoxDelegate::setModelData(QWidget * editor, QAbstractItemModel * model,
                                    const QModelIndex & index) const
{
    QComboBox * comboBox = qobject_cast<QComboBox *>(editor);
    if (comboBox) {
        switch ((SubtitleColumns)index.column()) {
        case SubtitleColumns::Font: {
            QString selectedText = comboBox->currentText();
            model->setData(index, selectedText, Qt::EditRole);
            break;
        }

        case SubtitleColumns::EnterAnimation: {
            model->setData(index, comboBox->currentIndex(), Qt::EditRole);
            break;
        }

        case SubtitleColumns::MoveAnimation: {
            model->setData(index, comboBox->currentIndex(), Qt::EditRole);
            break;
        }
        }
    }
}

bool ComboBoxDelegate::eventFilter(QObject * editor, QEvent * event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QComboBox * comboBox = qobject_cast<QComboBox *>(editor);
        if (comboBox) {
            comboBox->showPopup();
        }
    }
    return QStyledItemDelegate::eventFilter(editor, event);
}

EmptyTextComboBox::EmptyTextComboBox(QWidget * parent)
  : QComboBox(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setFixedWidth(100);
}

void EmptyTextComboBox::paintEvent(QPaintEvent * event)
{
    // QComboBox::paintEvent(event);
}
