#ifndef MONITORING_CHART_H
#define MONITORING_CHART_H

#include <QWidget>
#include <QColor>
#include <QVector>
#include <QPointF>

class MonitoringChart : public QWidget
{
    Q_OBJECT

public:
    explicit MonitoringChart(QWidget *parent = nullptr);

    void appendData(double voltage, double current);
    void clear();
    void setTimeWindow(int seconds);
    int  timeWindow() const { return m_timeWindow; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateTheme();

private:
    struct DataPoint {
        double voltage;
        double current;
    };

    void recalcRanges();

    QVector<DataPoint> m_buffer;
    int    m_bufferCapacity = 240;
    int    m_writeIndex     = 0;
    int    m_count          = 0;
    int    m_timeWindow     = 60;

    double m_voltageMin = 0, m_voltageMax = 30;
    double m_currentMin = 0, m_currentMax = 5;

    mutable QVector<QPointF> m_voltagePoints;
    mutable QVector<QPointF> m_currentPoints;

    // Theme-aware colors
    QColor m_bgColor;
    QColor m_chartBg;
    QColor m_gridColor;
    QColor m_textColor;
    QColor m_voltageColor;
    QColor m_currentColor;
};

#endif // MONITORING_CHART_H
