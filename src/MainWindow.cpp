#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QtWidgets>
#include <QtConcurrent>
#include <QStringConverter>

#include "ScreenRecorder.h"

MainWindow::MainWindow(QWidget * parent)
  : QWidget(parent)
  , ui(new Ui::MainWindow)
  , m_ScreenRecorderMgr(nullptr)
{
    ui->setupUi(this);
    installEventFilter(this);

    m_SubtitleModel = new SubtitleModel();
    ui->subtitleTableWidg->setModel(m_SubtitleModel);

    m_SubtitleAnimatorWidget = new SubtitleAnimatorWidget();
    ui->animatorLayout->addWidget(m_SubtitleAnimatorWidget);

    ui->cboxFont->addItems(QFontDatabase().families());
    ui->cboxFont->setCurrentText(u8"黑体");

    ui->sBoxMinFontSize->setValue(40);
    ui->sBoxMaxFontSize->setValue(51);
    ui->sBoxMinFontSize->setRange(10, 99);
    ui->sBoxMaxFontSize->setRange(10, 99);

    ui->colorPicker->UpdateWidth();

    QList<QPushButton *> btns = findChildren<QPushButton *>();
    foreach (auto btn, btns) {
        connect(btn, &QPushButton::clicked, this, &MainWindow::SlotBtnClick);
    }

    QObject::connect(ui->cboxScreenRatio, &QComboBox::currentIndexChanged, this, &MainWindow::SlotComBoxClick);
    QObject::connect(ui->cboxLimitScreenPara, &QCheckBox::checkStateChanged, this, &MainWindow::SlotCheckBoxClick);
    QObject::connect(ui->cboxSelectFont, &QCheckBox::checkStateChanged, this, &MainWindow::SlotCheckBoxClick);

    QObject::connect(ui->slider, &QSlider::valueChanged, this, &MainWindow::UpdataProgress);
    QObject::connect(m_SubtitleAnimatorWidget, &SubtitleAnimatorWidget::SgnIndexChange, this, &MainWindow::UpdataProgress);
    QObject::connect(m_SubtitleAnimatorWidget, &SubtitleAnimatorWidget::SgnAnimationFinished,
                     this, &MainWindow::AnimationFinished);

    // 生成提示对话框
    m_ProgressDialog = new QDialog();
    m_ProgressDialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    m_ProgressDialog->installEventFilter(this);
    QLabel * label = new QLabel(u8"正在生成录制视频，请稍候... \n ESC 取消视频生成", m_ProgressDialog);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("QLabel { font-size: 24px; color: white; }");
    QHBoxLayout * dialogLayout = new QHBoxLayout(m_ProgressDialog);
    dialogLayout->setContentsMargins(0, 0, 0, 0);
    dialogLayout->addWidget(label);
    m_ProgressDialog->setGeometry(this->geometry());
    m_ProgressDialog->setAttribute(Qt::WA_TranslucentBackground);
    m_ProgressDialog->setStyleSheet("background-color: rgba(0, 0, 0, 150);");

    m_RecordingAnimatorWidget = new SubtitleAnimatorWidget();
    m_RecordingAnimatorWidget->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    m_RecordingAnimatorWidget->show();
    m_RecordingAnimatorWidget->move(9999, 9999);

    setWindowTitle(u8"文字动效助手-v1.0.1");
    setMinimumSize(960 + 200, 540);
    resize(960 + 200, 540);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    m_RecordingAnimatorWidget->close();
    QWidget::closeEvent(event);
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QStyleOption option;
    option.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);
}

void MainWindow::resizeEvent(QResizeEvent * e)
{
    QWidget::resizeEvent(e);
    UpdateScreenStyle();
    if (width() > 1500) {
        if (ui->cboxScreenRatio->currentIndex() == 1) {
            setMinimumSize(960 + 200, 1000);
        } else {
            setMinimumSize(960 + 200, 840);
        }
    } else {
        setMinimumSize(960 + 200, 540);
    }
}

bool MainWindow::eventFilter(QObject * obj, QEvent * event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            AnimationFinished(false);
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void MainWindow::SlotBtnClick()
{
    QPushButton * btn = qobject_cast<QPushButton *>(sender());
    if (btn == ui->btnOpen) {
        OpenSRT();
    } else if (btn == ui->btnApply) {
        ApplayData();
    } else if (btn == ui->btnOpenCSV) {
        OpenCSV();
    } else if (btn == ui->btnExportCSV) {
        ExportCSV();
    } else if (btn == ui->btnSelectFont) {
        SelectCustomFont();
    } else if (btn == ui->btnBgColor) {
        ChangeBgColor();
    } else if (btn == ui->btnExport) {
        ExportVideo();
    } else if (btn == ui->btnExportDir) {
        OpenExportDir();
    } else if (btn == ui->btnRandom) {
        GenerateRandomData();
    } else if (btn == ui->btnPlay) {
        ui->btnPlay->setText(ui->btnPlay->isChecked() ? u8"暂停" : u8"播放");
        ui->slider->setEnabled(!ui->btnPlay->isChecked());
        m_SubtitleAnimatorWidget->SetAutoPlay(ui->btnPlay->isChecked());
    }
}

void MainWindow::SlotComBoxClick()
{
    QComboBox * cbox = qobject_cast<QComboBox *>(sender());
    if (cbox == ui->cboxScreenRatio) {
        if (ui->cboxScreenRatio->currentIndex() == 1) {
            ui->centerWid->addWidget(ui->subtitleTableWidg);
            ui->leftLayout->addWidget(ui->animatorLayWId);
            ui->dSBoxTextRation->setValue(0.1);
        } else {
            ui->centerWid->addWidget(ui->animatorLayWId);
            ui->leftLayout->addWidget(ui->subtitleTableWidg);
            ui->dSBoxTextRation->setValue(0.3);
        }
        SynchronizeData();
    }
}

void MainWindow::SlotCheckBoxClick()
{
    QCheckBox * cbox = qobject_cast<QCheckBox *>(sender());
    if (cbox == ui->cboxLimitScreenPara) {
        ui->sboxLimitScreenPara->setEnabled(ui->cboxLimitScreenPara->isChecked());
    } else if (cbox == ui->cboxSelectFont) {
        ui->btnSelectFont->setEnabled(ui->cboxSelectFont->isChecked());
    }
}

int MainWindow::ParseTimeToMs(const QString & timeStr)
{
    QTime time = QTime::fromString(timeStr, "hh:mm:ss,zzz");
    return QTime(0, 0, 0).msecsTo(time);
}

void MainWindow::ParseSrtFile(const QString & filePath)
{
    m_Subtitles.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream in(&file);
    QRegularExpression regex(R"((\d{2}:\d{2}:\d{2},\d{3}) --> (\d{2}:\d{2}:\d{2},\d{3}))");
    Subtitle currentSubtitle;
    currentSubtitle.font = GetCurrentFont();

    bool readingText = false;
    qint64 previousEndTimeMs = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (regex.match(line).hasMatch()) {
            QRegularExpressionMatch match = regex.match(line);
            qint64 startTimeMs = ParseTimeToMs(match.captured(1));
            qint64 endTimeMs = ParseTimeToMs(match.captured(2));
            if (startTimeMs < previousEndTimeMs) {
                QMessageBox::warning(this, tr("字幕解析错误"),
                                     tr("字幕时间戳不连续。检测到开始时间 %1 小于前一个字幕的结束时间。")
                                       .arg(match.captured(1)));
                m_Subtitles.clear();
                file.close();
                return;
            }
            currentSubtitle.startTimeMs = startTimeMs;
            currentSubtitle.durationMs = endTimeMs - startTimeMs;
            previousEndTimeMs = endTimeMs;
            readingText = true;
        } else if (readingText && line.isEmpty()) {
            m_Subtitles << currentSubtitle;
            currentSubtitle = Subtitle();
            currentSubtitle.font = GetCurrentFont();
            readingText = false;
        } else if (readingText) {
            currentSubtitle.text += (currentSubtitle.text.isEmpty() ? "" : "\n") + line;
        }
    }

    if (!currentSubtitle.text.isEmpty()) {
        m_Subtitles << currentSubtitle;
    }

    file.close();
}

void MainWindow::AddRandomAttributes()
{
    bool alternateRotation = true;
    int nextRotationIndex = QRandomGenerator::global()->bounded(4, 8);
    QList<QColor> colorLists = ui->colorPicker->GetColors();
    for (int i = 0; i < m_Subtitles.size(); ++i) {
        Subtitle & subtitle = m_Subtitles[i];
        QColor randomColor = colorLists[QRandomGenerator::global()->bounded(colorLists.size())];
        subtitle.fontColor = randomColor;
        int fontSize = QRandomGenerator::global()->bounded(ui->sBoxMinFontSize->value(), ui->sBoxMaxFontSize->value());
        subtitle.font = GetCurrentFont();
        subtitle.font.setPointSize(fontSize);

        int randomValue = QRandomGenerator::global()->bounded(9);
        subtitle.enterAnimation = (EnterAnimationType)randomValue;

        subtitle.moveAnimation = MoveAnimationType::MoveUp;

        if (i == nextRotationIndex) {
            if (alternateRotation) {
                subtitle.moveAnimation = MoveAnimationType::RotateClockwise;
            } else {
                subtitle.moveAnimation = MoveAnimationType::RotateCounterclockwise;
            }
            alternateRotation = !alternateRotation;
            nextRotationIndex += QRandomGenerator::global()->bounded(4, 8);
        }
    }
}

void MainWindow::ExportVideo()
{
    AnimationFinished(false);
    ApplayData();

    if (m_Subtitles.isEmpty()) {
        return;
    }
    setEnabled(false);

    QTimer::singleShot(100, nullptr, [&]() {
        m_ScreenRecorderMgr = new ScreenRecorderManager(m_RecordingAnimatorWidget, this);
        QTimer::singleShot(10, nullptr, [&]() {
            m_SubtitleAnimatorWidget->SetAutoPlay(true);
            m_RecordingAnimatorWidget->SetAutoPlay(true);
        });
        m_ScreenRecorderMgr->StartRecording(ui->cboxBgTrans->isChecked());
    });

    m_ProgressDialog->setGeometry(geometry());
    m_ProgressDialog->exec();
}

void MainWindow::AnimationFinished(const bool & generateVideo)
{
    if (m_ScreenRecorderMgr) {
        QEventLoop loop;
        connect(m_ScreenRecorderMgr, &ScreenRecorderManager::SgnCleanupFinished, &loop, &QEventLoop::quit);
        m_ScreenRecorderMgr->StopRecording();
        loop.exec();

        m_ScreenRecorderMgr->setParent(nullptr);
        m_ScreenRecorderMgr->deleteLater();
        m_ScreenRecorderMgr = nullptr;

        QThread::msleep(500);
        m_ProgressDialog->hide();
    }

    ui->btnPlay->setChecked(false);
    ui->btnPlay->setText(u8"播放");
    ui->slider->setEnabled(true);

    m_SubtitleAnimatorWidget->SetCurrentIndex(0);
    m_SubtitleAnimatorWidget->SetAutoPlay(false);

    m_RecordingAnimatorWidget->SetCurrentIndex(0);
    m_RecordingAnimatorWidget->SetAutoPlay(false);

    UpdataProgress(0);

    QTimer::singleShot(1000, this, [&]() {
        setEnabled(true);
    });
}

void MainWindow::SynchronizeData()
{
    SplitPart();
    int latestIndex = ui->slider->value();
    ui->slider->setRange(0, m_Subtitles.size());

    // 屏幕比例、尺寸
    UpdateScreenStyle();
    // 背景色
    m_SubtitleAnimatorWidget->SetBackgroundColor(m_BackgroundColor);
    if (ui->cboxBgTrans->isChecked()) {
        m_RecordingAnimatorWidget->SetBackgroundColor(QColor(0, 0, 0, 0));
    } else {
        m_RecordingAnimatorWidget->SetBackgroundColor(m_BackgroundColor);
    }
    m_RecordingAnimatorWidget->SetBackgroundTransparent(ui->cboxBgTrans->isChecked());
    // 字幕
    m_SubtitleModel->setSubtitles(m_Subtitles);
    m_SubtitleAnimatorWidget->SetSubtitles(m_Subtitles);
    m_RecordingAnimatorWidget->SetSubtitles(m_Subtitles);
    // 避免重叠
    m_SubtitleAnimatorWidget->SetAvoidOverlap(ui->cboxAvoidOverlap->isChecked());
    m_RecordingAnimatorWidget->SetAvoidOverlap(ui->cboxAvoidOverlap->isChecked());
    // 最大同屏数量
    if (ui->cboxLimitScreenPara->isChecked()) {
        m_SubtitleAnimatorWidget->SetSameScreenPartCount(ui->sboxLimitScreenPara->value());
        m_RecordingAnimatorWidget->SetSameScreenPartCount(ui->sboxLimitScreenPara->value());
    } else {
        m_SubtitleAnimatorWidget->SetSameScreenPartCount(-1);
        m_RecordingAnimatorWidget->SetSameScreenPartCount(-1);
    }

    if (latestIndex < m_Subtitles.size()) {
        UpdataProgress(latestIndex);
    } else {
        UpdataProgress(0);
    }
}

void MainWindow::ApplayData()
{
    m_Subtitles = m_SubtitleModel->GetSubtitles();
    SynchronizeData();
}

void MainWindow::GenerateRandomData()
{
    AddRandomAttributes();
    SynchronizeData();
}

void MainWindow::ExportCSV()
{
    SynchronizeData();
    QString fileName = QFileDialog::getSaveFileName(nullptr, "Save CSV File", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "Error", "Could not save file");
        return;
    }

    QTextStream out(&file);
    out << QChar(0xFEFF);
    for (const Subtitle & subtitle : m_Subtitles) {
        QString text = subtitle.text;
        text.replace("\n", "|&&|");
        out << text;
        out << ","
            << subtitle.startTimeMs << ","
            << subtitle.durationMs << ","
            << subtitle.font.family() << ","
            << subtitle.font.pointSize() << ","
            << subtitle.fontColor.name() << ","
            << static_cast<int>(subtitle.enterAnimation) << ","
            << static_cast<int>(subtitle.moveAnimation) << ","
            << subtitle.cameraZoom << "\n";
    }

    file.close();
}

void MainWindow::OpenCSV()
{
    QString fileName = QFileDialog::getOpenFileName(nullptr, "Open CSV File", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()) {
        return;
    }
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "Error", "Could not open file");
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::System);

    m_Subtitles.clear();
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');
        if (fields.size() != 9) {
            continue;
        }
        Subtitle subtitle;
        QString text = fields[0];
        text.replace("|&&|", "\n");
        subtitle.text = text;
        subtitle.startTimeMs = fields[1].toLongLong();
        subtitle.durationMs = fields[2].toLongLong();
        subtitle.font = QFont(fields[3], fields[4].toInt());
        subtitle.fontColor = QColor(fields[5]);
        subtitle.enterAnimation = static_cast<EnterAnimationType>(fields[6].toInt());
        subtitle.moveAnimation = static_cast<MoveAnimationType>(fields[7].toInt());
        subtitle.cameraZoom = fields[8].toDouble();
        m_Subtitles.append(subtitle);
    }

    file.close();
    SynchronizeData();
}

void MainWindow::OpenSRT()
{
    QString fileName =
      QFileDialog::getOpenFileName(
        this, u8"选择字幕", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
        u8"Image Files (*.srt)");

    if (!fileName.isEmpty()) {
        ParseSrtFile(fileName);
        AddRandomAttributes();
        SynchronizeData();
    }
}

void MainWindow::OpenExportDir()
{
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QRegularExpression regex("[\u4e00-\u9fa5]");
    if (desktopPath.contains(regex)) {
        qInfo() << "Chinese detected, changing to default path.";
        desktopPath = "./out";
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(desktopPath));
}

void MainWindow::ChangeBgColor()
{
    QColor curColor = m_SubtitleAnimatorWidget->GetBackgroundColor();
    QColorDialog colorDialog(curColor);
    colorDialog.setOption(QColorDialog::DontUseNativeDialog);
    if (colorDialog.exec() == QDialog::Accepted) {
        QColor selectedColor = colorDialog.selectedColor();
        if (selectedColor.isValid()) {
            m_BackgroundColor = selectedColor;
            SynchronizeData();
        }
    }
}

void MainWindow::SelectCustomFont()
{
    QString fileName = QFileDialog::getOpenFileName(nullptr, "Select custom font", "", "*.ttf");
    if (fileName.isEmpty()) {
        return;
    }
    int fontId = QFontDatabase::addApplicationFont(fileName);
    if (fontId != -1) {
        QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
        ui->btnSelectFont->setText(fontFamily);
    }
}

void MainWindow::UpdataProgress(int value)
{
    if (ui->slider->value() != value) {
        ui->slider->setValue(value);
    }
    m_SubtitleAnimatorWidget->SetCurrentIndex(value);
    ui->labProgressTag->setText(QString("%1/%2").arg(value).arg(ui->slider->maximum()));
}

void MainWindow::SplitPart()
{
    int partCount = 0;
    for (int i = 0; i < m_Subtitles.size(); ++i) {
        m_Subtitles[i].part = partCount;
        MoveAnimationType type = m_Subtitles[i].moveAnimation;
        if (IsPartSeparator(type)) {
            partCount++;
        }
    }
}

QString MainWindow::GetCurrentFont()
{
    if (ui->cboxSelectFont->isChecked()) {
        return ui->btnSelectFont->text();
    } else {
        return ui->cboxFont->currentText();
    }
}

SubtitleAnimatorWidget::ScreenRatio MainWindow::GetCurrentScreenRatio()
{
    if (ui->cboxScreenRatio->currentIndex() == 0) {
        return SubtitleAnimatorWidget::R_16_9;
    } else if (ui->cboxScreenRatio->currentIndex() == 1) {
        return SubtitleAnimatorWidget::R_9_16;
    }

    return SubtitleAnimatorWidget::R_16_9;
}

void MainWindow::UpdateScreenStyle()
{
    SubtitleAnimatorWidget::Style style = SubtitleAnimatorWidget::Normal;
    SubtitleAnimatorWidget::Style rStyle = SubtitleAnimatorWidget::Recording;
    if (width() < 1500) {
        style = SubtitleAnimatorWidget::Small;
    }

    m_SubtitleAnimatorWidget->UpdateScreenStyle(
      style, GetCurrentScreenRatio(), ui->cboxResolution->currentIndex(), ui->dSBoxTextRation->value());
    m_RecordingAnimatorWidget->UpdateScreenStyle(
      rStyle, GetCurrentScreenRatio(), ui->cboxResolution->currentIndex(), ui->dSBoxTextRation->value());
}
