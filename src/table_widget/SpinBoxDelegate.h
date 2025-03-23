#ifndef SpinBoxDelegate_H
#define SpinBoxDelegate_H

#include <QStyledItemDelegate>
#include <QSpinBox>
#include <QDoubleSpinBox>

class EmptySpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit EmptySpinBox(QWidget * parent = nullptr);

protected:
    void paintEvent(QPaintEvent * event) override;
};

class EmptyDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    explicit EmptyDoubleSpinBox(QWidget * parent = nullptr);

protected:
    void paintEvent(QPaintEvent * event) override;
};

class SpinBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit SpinBoxDelegate(int min, int max, QObject * parent = nullptr);

    QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option,
                           const QModelIndex & index) const override;

    void setEditorData(QWidget * editor, const QModelIndex & index) const override;

    void setModelData(QWidget * editor, QAbstractItemModel * model,
                      const QModelIndex & index) const override;

private:
    int minValue;
    int maxValue;
};

class DoubleSpinBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DoubleSpinBoxDelegate(QObject * parent = nullptr);

    QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option,
                           const QModelIndex & index) const override;

    void setEditorData(QWidget * editor, const QModelIndex & index) const override;

    void setModelData(QWidget * editor, QAbstractItemModel * model,
                      const QModelIndex & index) const override;
};

#endif // SUBTITLETABLEWIDGET_H
