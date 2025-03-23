#include "SubtitleTableWidget.h"

#include <QHeaderView>

#include "ComboBoxDelegate.h"
#include "LineEditDelegate.h"
#include "SpinBoxDelegate.h"
#include "ColorPickerDelegate.h"

SubtitleTableWidget::SubtitleTableWidget(QWidget * parent)
  : QTableView(parent)
  , comboBoxDelegate(new ComboBoxDelegate(this))
  , spinBoxDelegate(new SpinBoxDelegate(8, 99, this))
  , colorPickerDelegate(new ColorPickerDelegate(this))
  , doubleSpinBoxDelegate(new DoubleSpinBoxDelegate(this))
{
    setupDelegates();
    setCornerButtonEnabled(false);
    verticalHeader()->setVisible(true);
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    verticalHeader()->setFixedWidth(30);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::AllEditTriggers);
    setShowGrid(true);
    resize(800, 600);

    setStyleSheet("QTableView::item { height: 25px; }");
}

void SubtitleTableWidget::setupDelegates()
{
    setItemDelegateForColumn((int)SubtitleColumns::Font, comboBoxDelegate);
    setItemDelegateForColumn((int)SubtitleColumns::FontSize, spinBoxDelegate);
    setItemDelegateForColumn((int)SubtitleColumns::FontColor, colorPickerDelegate);
    setItemDelegateForColumn((int)SubtitleColumns::EnterAnimation, comboBoxDelegate);
    setItemDelegateForColumn((int)SubtitleColumns::MoveAnimation, comboBoxDelegate);
    setItemDelegateForColumn((int)SubtitleColumns::CameraZoom, doubleSpinBoxDelegate);
}

void SubtitleTableWidget::configureColumnWidths()
{
    horizontalHeader()->setSectionResizeMode((int)SubtitleColumns::Text, QHeaderView::Stretch);
    for (int i = (int)SubtitleColumns::Font; i < (int)SubtitleColumns::ColumnCount; ++i) {
        horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
    }

    setColumnWidth((int)SubtitleColumns::Font, 110);
    setColumnWidth((int)SubtitleColumns::FontSize, 50);
    setColumnWidth((int)SubtitleColumns::FontColor, 45);
    setColumnWidth((int)SubtitleColumns::EnterAnimation, 75);
    setColumnWidth((int)SubtitleColumns::MoveAnimation, 75);
    setColumnWidth((int)SubtitleColumns::CameraZoom, 55);
}

void SubtitleTableWidget::setModel(SubtitleModel * model)
{
    QTableView::setModel(model);
    configureColumnWidths();
}
