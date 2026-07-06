#include "MonitoringPage.h"
#include "widgets/LcdPanelWidget.h"
#include "widgets/ProtocolStatusBar.h"
#include "widgets/MonitoringChart.h"
#include <QVBoxLayout>

MonitoringPage::MonitoringPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void MonitoringPage::setupUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    m_statusBar = new ProtocolStatusBar(this);
    m_lcdPanel  = new LcdPanelWidget(this);
    m_chart     = new MonitoringChart(this);

    layout->addWidget(m_statusBar);
    layout->addWidget(m_lcdPanel);
    layout->addWidget(m_chart, 1); // chart takes remaining space
}

void MonitoringPage::onMonitoringData(ChargerProtocol::MonitoringData data)
{
    m_lcdPanel->setVoltage(data.voltageV());
    m_lcdPanel->setCurrent(data.currentA());
    m_lcdPanel->setPower(data.powerW());

    m_chart->appendData(data.voltageV(), data.currentA());
}

void MonitoringPage::onConnectionChanged(bool connected)
{
    m_statusBar->setConnectionStatus(connected);
    if (!connected)
        clearData();
}

void MonitoringPage::onModeChanged(const QString &mode)
{
    m_statusBar->setPowerMode(mode);
}

void MonitoringPage::clearData()
{
    m_lcdPanel->clear();
    m_chart->clear();
    m_statusBar->setProtocolInfo(QString());
}
