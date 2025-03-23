#ifndef ColorPickerWidget_H
#define ColorPickerWidget_H

#include <QWidget>
#include <QColor>
#include <QVector>

class ColorPickerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ColorPickerWidget(QWidget * parent = nullptr);
    ~ColorPickerWidget();

    void UpdateWidth();
    QVector<QColor> GetColors() const;

protected:
    void paintEvent(QPaintEvent * event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void enterEvent(QEnterEvent * event) override;
    void leaveEvent(QEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;

private:
    void DrawCircle(QPainter & painter, const QRect & rect, const QColor & color, bool isHovered);

private:
    QVector<QColor> m_Colors;
    int m_HoveredIndex;
};

#endif // ColorPickerWidget_H
