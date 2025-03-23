#ifndef ComboBoxDelegate_H
#define ComboBoxDelegate_H

#include <QStyledItemDelegate>
#include <QComboBox>

class EmptyTextComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit EmptyTextComboBox(QWidget * parent = nullptr);

protected:
    void paintEvent(QPaintEvent * event) override;
};

class ComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ComboBoxDelegate(QObject * parent = nullptr);

    QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option,
                           const QModelIndex & index) const override;

    void setEditorData(QWidget * editor, const QModelIndex & index) const override;

    void setModelData(QWidget * editor, QAbstractItemModel * model,
                      const QModelIndex & index) const override;

protected:
    bool eventFilter(QObject * editor, QEvent * event) override;

};

#endif // SUBTITLETABLEWIDGET_H
