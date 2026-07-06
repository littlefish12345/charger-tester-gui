#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <ElaWindow.h>
#include <QEvent>
#include <QPoint>
#include <QTimer>
#include "protocol/ChargerTypes.h"
#include "io/SerialPortConfig.h"

class ElaStatusBar;
class ElaText;
class MonitoringPage;
class ProtocolAnalysisPage;
class LoadControlPage;
class SettingsPage;
class SerialPortManager;

class MainWindow : public ElaWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onSettingsConnect(const SerialPortConfig &config);
    void onSettingsDisconnect();
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(const QString &error);
    void onMonitoringData(ChargerProtocol::MonitoringData data);
    void onCommandResponse(ChargerProtocol::CommandResponse response);
    void onProtocolInfoReceived(ChargerProtocol::MonitoringData data);
    void onRawFrameReceived(const QString &rawData);
    void onRefreshIntervalChanged(int ms);
    void onLoadControlSendMode(const QString &mode);
    void onLoadControlSendPower(int currentMa, int voltageMv);
    void onLoadControlSendDecoyPdo(int index);
    void onLoadControlSendDecoyPps(int voltageMv);
    void onAutoConnectScan();
    void onAutoConnectEnabledChanged(bool enabled);

private:
    void setupPages();
    void setupStatusBar();
    void setupConnections();
    void setupAutoConnect();
    bool tryAutoConnect();

    // Window drag/resize for frameless window
    bool eventFilter(QObject *obj, QEvent *event) override;
    static Qt::Edges hitTestEdges(const QPoint &pos, int w, int h, int margin);
    QPoint m_dragStartPos;

    SerialPortManager *m_portManager = nullptr;

    // Status bar
    ElaStatusBar *m_statusBar  = nullptr;
    ElaText      *m_statusText = nullptr;

    // Pages
    MonitoringPage       *m_monitoringPage       = nullptr;
    ProtocolAnalysisPage *m_protocolAnalysisPage = nullptr;
    LoadControlPage      *m_loadControlPage      = nullptr;
    SettingsPage         *m_settingsPage         = nullptr;

    bool     m_connected    = false;
    bool     m_connecting   = false;
    bool     m_autoConnectEnabled = true;
    bool     m_autoConnected = false;
    QString  m_portName;
    int      m_baudRate     = 115200;
    QTimer  *m_autoConnectTimer = nullptr;
};

#endif // MAIN_WINDOW_H
