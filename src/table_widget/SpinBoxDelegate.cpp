#include "SpinBoxDelegate.h"

#include <QPainter>

EmptySpinBox::EmptySpinBox(QWidget * parent)
  : QSpinBox(parent)
{
    setFocusPolicy(Qt::NoFocus);
}

void EmptySpinBox::paintEvent(QPaintEvent * event)
{
    QSpinBox::paintEvent(event);
}

EmptyDoubleSpinBox::EmptyDoubleSpinBox(QWidget * parent)
  : QDoubleSpinBox(parent)
{
    setFocusPolicy(Qt::NoFocus);
}

void EmptyDoubleSpinBox::paintEvent(QPaintEvent * event)
{
    QDoubleSpinBox::paintEvent(event);
}

// ---------------------------

SpinBoxDelegate::SpinBoxDelegate(int min, int max, QObject * parent)
  : QStyledItemDelegate(parent)
  , minValue(min)
  , maxValue(max)
{ }

QWidget * SpinBoxDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    EmptySpinBox * spinBox = new EmptySpinBox(parent);
    spinBox->setGeometry(parent->geometry());
    spinBox->setRange(minValue, maxValue);
    return spinBox;
}

void SpinBoxDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    EmptySpinBox * spinBox = qobject_cast<EmptySpinBox *>(editor);
    if (spinBox) {
        int value = index.data(Qt::DisplayRole).toInt();
        spinBox->setValue(value);
    }
}

void SpinBoxDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
    EmptySpinBox * spinBox = qobject_cast<EmptySpinBox *>(editor);
    if (spinBox) {
        model->setData(index, spinBox->value(), Qt::EditRole);
    }
}

// ---------------------------

DoubleSpinBoxDelegate::DoubleSpinBoxDelegate(QObject * parent)
  : QStyledItemDelegate(parent)
{ }

QWidget * DoubleSpinBoxDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    EmptyDoubleSpinBox * spinBox = new EmptyDoubleSpinBox(parent);
    spinBox->setGeometry(parent->geometry());
    spinBox->setSingleStep(.05);
    spinBox->setRange(0.05, 2.0);
    return spinBox;
}

void DoubleSpinBoxDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    EmptyDoubleSpinBox * spinBox = qobject_cast<EmptyDoubleSpinBox *>(editor);
    if (spinBox) {
        int value = index.data(Qt::DisplayRole).toInt();
        spinBox->setValue(value);
    }
}

void DoubleSpinBoxDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
    EmptyDoubleSpinBox * spinBox = qobject_cast<EmptyDoubleSpinBox *>(editor);
    if (spinBox) {
        model->setData(index, spinBox->value(), Qt::EditRole);
    }
}
