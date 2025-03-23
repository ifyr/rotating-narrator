#include "ColorPickerDelegate.h"

#include <QColorDialog>
#include <QPainter>
#include <QApplication>

ColorPickerDelegate::ColorPickerDelegate(QObject * parent)
  : QStyledItemDelegate(parent)
{ }

QWidget * ColorPickerDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option,
                                            const QModelIndex & index) const
{
    QColor initialColor = QColor(index.data(Qt::DisplayRole).toString());
    QColorDialog * colorDialog = new QColorDialog(initialColor);
    colorDialog->setAttribute(Qt::WA_DeleteOnClose);
    return colorDialog;
}

void ColorPickerDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    QColorDialog * colorDialog = qobject_cast<QColorDialog *>(editor);
    if (colorDialog) {
        QColor color = index.data(Qt::DisplayRole).value<QColor>();
        colorDialog->setCurrentColor(color);
        QWidget * activeWindow = QApplication::activeWindow();
        if (activeWindow) {
            QPoint globalPos = activeWindow->mapToGlobal(activeWindow->rect().center());
            globalPos.setX(globalPos.x() - colorDialog->width() / 2);
            globalPos.setY(globalPos.y() - colorDialog->height() / 2);
            colorDialog->move(globalPos);
        }
    }
}

void ColorPickerDelegate::setModelData(QWidget * editor, QAbstractItemModel * model,
                                       const QModelIndex & index) const
{
    QColorDialog * colorDialog = qobject_cast<QColorDialog *>(editor);
    if (colorDialog) {
        colorDialog->close();
        QColor selectedColor = colorDialog->currentColor();
        model->setData(index, selectedColor, Qt::EditRole);
    }
}

void ColorPickerDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option,
                                const QModelIndex & index) const
{
    QColor color = QColor(index.data(Qt::DisplayRole).toString());
    int margin = 5;
    QRect rect = option.rect.adjusted(margin, margin, -margin, -margin);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setBrush(QBrush(color));
    painter->setPen(Qt::NoPen);
    int size = qMin(rect.width(), rect.height());
    QRect square(rect.x(), rect.y(), size, size);
    painter->drawRect(square);
    painter->restore();
}
