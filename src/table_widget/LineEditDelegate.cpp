#include "LineEditDelegate.h"

#include <QLineEdit>

LineEditDelegate::LineEditDelegate(QObject * parent)
  : QStyledItemDelegate(parent)
{ }

QWidget * LineEditDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QLineEdit * editor = new QLineEdit(parent);
    return editor;
}

void LineEditDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    QLineEdit * lineEdit = qobject_cast<QLineEdit *>(editor);
    if (lineEdit) {
        lineEdit->setText(index.data(Qt::DisplayRole).toString());
    }
}

void LineEditDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
    QLineEdit * lineEdit = qobject_cast<QLineEdit *>(editor);
    if (lineEdit) {
        model->setData(index, lineEdit->text(), Qt::EditRole);
    }
}
