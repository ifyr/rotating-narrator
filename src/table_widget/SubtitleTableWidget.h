#ifndef SubtitleTableWidget_H
#define SubtitleTableWidget_H

#include <QTableView>

#include "SubtitleModel.h"

class ComboBoxDelegate;
class LineEditDelegate;
class SpinBoxDelegate;
class ColorPickerDelegate;
class DoubleSpinBoxDelegate;

class SubtitleTableWidget : public QTableView
{
    Q_OBJECT

public:
    explicit SubtitleTableWidget(QWidget * parent = nullptr);
    void setModel(SubtitleModel * model);

private:
    void setupDelegates();
    void configureColumnWidths();

private:
    ComboBoxDelegate * comboBoxDelegate;
    SpinBoxDelegate * spinBoxDelegate;
    ColorPickerDelegate * colorPickerDelegate;
    DoubleSpinBoxDelegate * doubleSpinBoxDelegate;
};

#endif // SUBTITLETABLEWIDGET_H
