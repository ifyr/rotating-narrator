#include "ColorPickerWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QColorDialog>

static const int m_CircleRadius = 12;
static const int m_HorizontalSpacing = 6;
static const int m_VerticalSpacing = 3;
static const int m_WindowHeight = 30;

ColorPickerWidget::ColorPickerWidget(QWidget * parent)
  : QWidget(parent)
  , m_HoveredIndex(-1)
{
    setMouseTracking(true);
    m_Colors = {
        QColor(255, 99, 71), // 番茄红
        QColor(60, 179, 113), // 中海洋绿
        QColor(72, 209, 204), // 水鸭蓝
        QColor(100, 149, 237), // 矢车菊蓝
        QColor(255, 165, 0), // 橙色
        QColor(138, 43, 226), // 蓝紫色
    };
    setFixedHeight(m_WindowHeight);
    UpdateWidth();

    setToolTip(u8"左键修改颜色，右键删除颜色，中键增加颜色");
}

ColorPickerWidget::~ColorPickerWidget()
{
}

void ColorPickerWidget::UpdateWidth()
{
    setFixedWidth((m_CircleRadius * 2 + m_HorizontalSpacing) * m_Colors.size() + m_HorizontalSpacing);
}

QVector<QColor> ColorPickerWidget::GetColors() const
{
    return m_Colors;
}

void ColorPickerWidget::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int x = m_HorizontalSpacing;

    for (int i = 0; i < m_Colors.size(); ++i) {
        QRect circleRect(x, m_VerticalSpacing, m_CircleRadius * 2, m_CircleRadius * 2);
        DrawCircle(painter, circleRect, m_Colors[i], i == m_HoveredIndex);
        x += m_CircleRadius * 2 + m_HorizontalSpacing;
    }
}

void ColorPickerWidget::mousePressEvent(QMouseEvent * event)
{
    int x = m_HorizontalSpacing;

    for (int i = 0; i < m_Colors.size(); ++i) {
        QRect circleRect(x, m_VerticalSpacing, m_CircleRadius * 2, m_CircleRadius * 2);

        if (circleRect.contains(event->pos())) {
            if (event->button() == Qt::LeftButton) {
                // 左键点击：修改颜色
                QColor newColor = QColorDialog::getColor(m_Colors[i], this);
                if (newColor.isValid()) {
                    m_Colors[i] = newColor;
                }
            } else if (event->button() == Qt::RightButton) {
                // 右键点击：删除颜色
                if (m_Colors.size() > 1) {
                    m_Colors.removeAt(i);
                }
            } else if (event->button() == Qt::MiddleButton) {
                // 中键点击：增加颜色
                QColor newColor = QColorDialog::getColor(Qt::white, this);
                if (newColor.isValid()) {
                    m_Colors.insert(i + 1, newColor);
                }
            }
            UpdateWidth();
            update();
            return;
        }
        x += m_CircleRadius * 2 + m_HorizontalSpacing;
    }
}

void ColorPickerWidget::mouseMoveEvent(QMouseEvent * event)
{
    int x = m_HorizontalSpacing;
    m_HoveredIndex = -1;

    for (int i = 0; i < m_Colors.size(); ++i) {
        QRect circleRect(x, m_VerticalSpacing, m_CircleRadius * 2, m_CircleRadius * 2);

        if (circleRect.contains(event->pos())) {
            m_HoveredIndex = i;
            break;
        }

        x += m_CircleRadius * 2 + m_HorizontalSpacing;
    }
    update();
}

void ColorPickerWidget::enterEvent(QEnterEvent * event)
{
    Q_UNUSED(event);
    update();
}

void ColorPickerWidget::leaveEvent(QEvent * event)
{
    Q_UNUSED(event);
    m_HoveredIndex = -1;
    update();
}

void ColorPickerWidget::DrawCircle(QPainter & painter, const QRect & rect, const QColor & color, bool isHovered)
{
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(rect);

    if (isHovered) {
        QPen pen(Qt::gray);
        pen.setWidth(2);
        painter.setPen(pen);
        painter.drawEllipse(rect.adjusted(-2, -2, 2, 2));
    }
}
