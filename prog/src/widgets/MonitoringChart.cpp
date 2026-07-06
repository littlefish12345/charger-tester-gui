#include "MonitoringChart.h"
#include "ElaTheme.h"
#include "utils/ThemeUtils.h"
#include <QPainter>
#include <QPaintEvent>
#include <algorithm>

MonitoringChart::MonitoringChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_voltagePoints.reserve(m_bufferCapacity);
    m_currentPoints.reserve(m_bufferCapacity);
    updateTheme();
}

void MonitoringChart::updateTheme()
{
    auto c = ThemeColors::current();
    m_bgColor      = c.cardBg;
    m_chartBg      = c.chartBg;
    m_gridColor    = c.chartGrid;
    m_textColor    = c.chartText;
    m_voltageColor = c.accentBlue;
    m_currentColor = c.accentGreen;
    update();
}

void MonitoringChart::appendData(double voltage, double current)
{
    DataPoint dp{voltage, current};

    if (m_count < m_bufferCapacity) {
        if (m_buffer.size() < m_bufferCapacity)
            m_buffer.append(dp);
        else
            m_buffer[m_writeIndex] = dp;
        m_count++;
    } else {
        m_buffer[m_writeIndex] = dp;
    }

    m_writeIndex = (m_writeIndex + 1) % m_bufferCapacity;
    recalcRanges();
    update();
}

void MonitoringChart::clear()
{
    m_buffer.clear();
    m_writeIndex = 0;
    m_count = 0;
    m_voltageMin = 0;
    m_voltageMax = 30;
    m_currentMin = 0;
    m_currentMax = 5;
    update();
}

void MonitoringChart::setTimeWindow(int seconds)
{
    m_timeWindow = seconds;
    update();
}

void MonitoringChart::recalcRanges()
{
    if (m_count == 0) return;

    double vMax = 0, cMax = 0;
    int visible = std::min(m_count, m_timeWindow * 2);
    int startIdx = (m_writeIndex - visible + m_buffer.size()) % m_buffer.size();

    for (int i = 0; i < visible; ++i) {
        int idx = (startIdx + i) % m_buffer.size();
        const auto &dp = m_buffer[idx];
        vMax = std::max(vMax, dp.voltage);
        cMax = std::max(cMax, dp.current);
    }

    m_voltageMax = m_voltageMax * 0.6 + std::max(vMax * 1.15, 1.0) * 0.4;
    m_currentMax = m_currentMax * 0.6 + std::max(cMax * 1.15, 0.1) * 0.4;
    m_voltageMin = 0;
    m_currentMin = 0;
}

void MonitoringChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    int w = width();
    int h = height();
    int marginLeft = 60, marginRight = 20, marginTop = 20, marginBottom = 40;
    int chartW = w - marginLeft - marginRight;
    int chartH = h - marginTop - marginBottom;

    if (chartW <= 0 || chartH <= 0) return;

    // Background
    painter.fillRect(rect(), m_bgColor);

    // Chart area
    QRect chartRect(marginLeft, marginTop, chartW, chartH);
    painter.fillRect(chartRect, m_chartBg);

    // Grid
    painter.setPen(QPen(m_gridColor, 1, Qt::DotLine));
    for (int i = 0; i <= 4; ++i) {
        int y = marginTop + (chartH * i / 4);
        painter.drawLine(marginLeft, y, marginLeft + chartW, y);
    }
    for (int i = 0; i <= 4; ++i) {
        int x = marginLeft + (chartW * i / 4);
        painter.drawLine(x, marginTop, x, marginTop + chartH);
    }

    if (m_count < 2) {
        painter.setPen(m_textColor);
        painter.drawText(chartRect, Qt::AlignCenter,
                         QStringLiteral("等待数据..."));
        painter.setPen(m_textColor);
        painter.drawLine(marginLeft, marginTop + chartH, marginLeft + chartW, marginTop + chartH);
        painter.drawLine(marginLeft, marginTop, marginLeft, marginTop + chartH);
        return;
    }

    int visible = std::min(m_count, m_timeWindow * 2);
    m_voltagePoints.clear();
    m_currentPoints.clear();

    int startIdx = (m_writeIndex - visible + m_buffer.size()) % m_buffer.size();

    for (int i = 0; i < visible; ++i) {
        int idx = (startIdx + i) % m_buffer.size();
        const auto &dp = m_buffer[idx];

        double x = marginLeft + (double)i / (visible - 1) * chartW;
        double vFrac = (dp.voltage - m_voltageMin) / (m_voltageMax - m_voltageMin + 0.001);
        double cFrac = (dp.current - m_currentMin) / (m_currentMax - m_currentMin + 0.001);

        double vY = marginTop + chartH - vFrac * chartH;
        double cY = marginTop + chartH - cFrac * chartH;

        m_voltagePoints.append(QPointF(x, std::clamp(vY, (double)marginTop, (double)(marginTop + chartH))));
        m_currentPoints.append(QPointF(x, std::clamp(cY, (double)marginTop, (double)(marginTop + chartH))));
    }

    // Voltage line
    painter.setPen(QPen(m_voltageColor, 2));
    painter.drawPolyline(m_voltagePoints.constData(), m_voltagePoints.size());

    // Current line
    painter.setPen(QPen(m_currentColor, 2));
    painter.drawPolyline(m_currentPoints.constData(), m_currentPoints.size());

    // Y-axis labels
    painter.setPen(m_textColor);
    QFont smallFont = painter.font();
    smallFont.setPixelSize(10);
    painter.setFont(smallFont);

    painter.drawText(QRectF(0, marginTop - 8, marginLeft - 4, 16),
                     Qt::AlignRight | Qt::AlignVCenter,
                     QString::number(m_voltageMax, 'f', 1) + "V");
    painter.drawText(QRectF(0, marginTop + chartH - 8, marginLeft - 4, 16),
                     Qt::AlignRight | Qt::AlignVCenter,
                     "0");

    // Legend
    int legX = marginLeft + 8;
    int legY = marginTop + 4;
    painter.setPen(m_voltageColor);
    painter.drawText(QRectF(legX, legY, 80, 16), Qt::AlignLeft, QStringLiteral("电压 (V)"));
    painter.setPen(m_currentColor);
    painter.drawText(QRectF(legX + 90, legY, 80, 16), Qt::AlignLeft, QStringLiteral("电流 (A)"));
}

void MonitoringChart::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}
