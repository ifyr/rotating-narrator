#ifndef MainWindow_H
#define MainWindow_H

#include <QWidget>
#include <QWidget>

#include "SubtitleAnimatorWidget.h"
#include "table_widget/SubtitleModel.h"
#include "Subtitle.h"

namespace Ui {
class MainWindow;
}

class ScreenRecorder;
class ScreenRecorderManager;
class QDialog;
class QColor;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget * parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent * event) override;
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    bool eventFilter(QObject * obj, QEvent * event) override;

private:
    void SlotBtnClick();
    void SlotComBoxClick();
    void SlotCheckBoxClick();

    int ParseTimeToMs(const QString & timeStr);
    void ParseSrtFile(const QString & filePath);
    void AddRandomAttributes();
    void ExportVideo();
    void AnimationFinished(const bool & generateVideo);
    void SynchronizeData();

    void ApplayData();
    void GenerateRandomData();
    void ExportCSV();
    void OpenCSV();
    void OpenSRT();
    void OpenExportDir();
    void ChangeBgColor();
    void SelectCustomFont();

    void UpdataProgress(int value);

    void SplitPart();

    QString GetCurrentFont();

    SubtitleAnimatorWidget::ScreenRatio GetCurrentScreenRatio();

    void UpdateScreenStyle();

private:
    Ui::MainWindow * ui;

    QList<Subtitle> m_Subtitles;
    SubtitleModel * m_SubtitleModel;

    SubtitleAnimatorWidget * m_SubtitleAnimatorWidget;
    SubtitleAnimatorWidget * m_RecordingAnimatorWidget;
    ScreenRecorderManager * m_ScreenRecorderMgr;
    QDialog * m_ProgressDialog;

    QColor m_BackgroundColor = QColor(5, 5, 5);
};

#endif // MainWindow
