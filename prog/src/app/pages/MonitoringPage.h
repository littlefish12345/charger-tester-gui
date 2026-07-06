#ifndef MONITORING_PAGE_H
#define MONITORING_PAGE_H

#include <QWidget>
#include "protocol/ChargerTypes.h"

class LcdPanelWidget;
class ProtocolStatusBar;
class MonitoringChart;
class QLabel;

class MonitoringPage : public QWidget
{
    Q_OBJECT

public:
    explicit MonitoringPage(QWidget *parent = nullptr);

public slots:
    void onMonitoringData(ChargerProtocol::MonitoringData data);
    void onConnectionChanged(bool connected);
    void onModeChanged(const QString &mode);
    void clearData();

private:
    void setupUi();

    LcdPanelWidget    *m_lcdPanel    = nullptr;
    ProtocolStatusBar *m_statusBar   = nullptr;
    MonitoringChart   *m_chart       = nullptr;
};

#endif // MONITORING_PAGE_H
