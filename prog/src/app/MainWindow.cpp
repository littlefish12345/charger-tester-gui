#include "MainWindow.h"
#include "pages/MonitoringPage.h"
#include "pages/ProtocolAnalysisPage.h"
#include "pages/LoadControlPage.h"
#include "pages/SettingsPage.h"
#include "io/SerialPortManager.h"

#include <ElaStatusBar.h>
#include <ElaText.h>
#include <QApplication>
#include <QMouseEvent>
#include <QSerialPortInfo>
#include <QWindow>

namespace {
constexpr int kAutoConnectIntervalMs = 3000;
constexpr int kAutoConnectBaudRate = 115200;
constexpr const char *kAutoConnectSerial = "ElectricLoader";
}

MainWindow::MainWindow(QWidget *parent)
    : ElaWindow(parent)
{
    setWindowTitle(QStringLiteral("充电头测试仪"));
    setMinimumSize(1200, 800);
    setWindowButtonFlag(ElaAppBarType::ThemeChangeButtonHint, false);

    m_portManager = new SerialPortManager(this);

    setupPages();
    setupStatusBar();
    setupConnections();
    setupAutoConnect();

    qApp->installEventFilter(this);
}

MainWindow::~MainWindow() = default;

// ── Navigation pages ──

void MainWindow::setupPages()
{
    m_monitoringPage       = new MonitoringPage(this);
    m_protocolAnalysisPage = new ProtocolAnalysisPage(this);
    m_loadControlPage      = new LoadControlPage(this);
    m_settingsPage         = new SettingsPage(this);

    addPageNode(QStringLiteral("实时监控"), m_monitoringPage,
                ElaIconType::MonitorWaveform);
    addPageNode(QStringLiteral("协议分析"), m_protocolAnalysisPage,
                ElaIconType::Microchip);
    addPageNode(QStringLiteral("负载控制"), m_loadControlPage,
                ElaIconType::Bolt);
    addPageNode(QStringLiteral("设置"), m_settingsPage,
                ElaIconType::GearComplex);
}

// ── Status bar ──

void MainWindow::setupStatusBar()
{
    m_statusBar  = new ElaStatusBar(this);
    m_statusBar->setSizeGripEnabled(false);
    m_statusText = new ElaText(QStringLiteral("已断开"), this);
    m_statusText->setTextPixelSize(12);
    m_statusText->setMinimumWidth(200);
    m_statusBar->addWidget(m_statusText);
    setStatusBar(m_statusBar);
}

// ── Signal/slot wiring ──

void MainWindow::setupConnections()
{
    // Settings page -> serial management
    connect(m_settingsPage, &SettingsPage::connectRequested,
            this, &MainWindow::onSettingsConnect);
    connect(m_settingsPage, &SettingsPage::disconnectRequested,
            this, &MainWindow::onSettingsDisconnect);

    connect(m_portManager, &SerialPortManager::connected, this, &MainWindow::onConnected);
    connect(m_portManager, &SerialPortManager::disconnected, this, &MainWindow::onDisconnected);
    connect(m_portManager, &SerialPortManager::errorOccurred, this, &MainWindow::onErrorOccurred);

    // Data flow: manager -> pages
    connect(m_portManager, &SerialPortManager::monitoringDataReceived,
            this, &MainWindow::onMonitoringData);
    connect(m_portManager, &SerialPortManager::commandResponseReceived,
            this, &MainWindow::onCommandResponse);
    connect(m_portManager, &SerialPortManager::protocolInfoReceived,
            this, &MainWindow::onProtocolInfoReceived);
    connect(m_portManager, &SerialPortManager::rawFrameReceived,
            this, &MainWindow::onRawFrameReceived);

    // Protocol analysis page -> manager (decoy commands)
    connect(m_protocolAnalysisPage, &ProtocolAnalysisPage::sendSetDecoyPdo,
            this, &MainWindow::onLoadControlSendDecoyPdo);
    connect(m_protocolAnalysisPage, &ProtocolAnalysisPage::sendSetDecoyPps,
            this, &MainWindow::onLoadControlSendDecoyPps);

    // Load control page -> manager (commands)
    connect(m_loadControlPage, &LoadControlPage::sendSetMode,
            this, &MainWindow::onLoadControlSendMode);
    connect(m_loadControlPage, &LoadControlPage::sendSetPower,
            this, &MainWindow::onLoadControlSendPower);
    connect(m_loadControlPage, &LoadControlPage::sendSetDecoyPdo,
            this, &MainWindow::onLoadControlSendDecoyPdo);
    connect(m_loadControlPage, &LoadControlPage::sendSetDecoyPps,
            this, &MainWindow::onLoadControlSendDecoyPps);

    // Settings page signals
    connect(m_settingsPage, &SettingsPage::refreshIntervalChanged,
            this, &MainWindow::onRefreshIntervalChanged);
    connect(m_settingsPage, &SettingsPage::autoConnectEnabledChanged,
            this, &MainWindow::onAutoConnectEnabledChanged);
}

void MainWindow::setupAutoConnect()
{
    m_autoConnectEnabled = m_settingsPage->autoConnectEnabled();
    m_autoConnectTimer = new QTimer(this);
    m_autoConnectTimer->setInterval(kAutoConnectIntervalMs);
    connect(m_autoConnectTimer, &QTimer::timeout,
            this, &MainWindow::onAutoConnectScan);
    m_autoConnectTimer->start();

    QTimer::singleShot(0, this, &MainWindow::onAutoConnectScan);
}

bool MainWindow::tryAutoConnect()
{
    if (!m_autoConnectEnabled || m_connected || m_connecting)
        return false;

    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto &port : ports) {
        if (port.serialNumber() != QString::fromLatin1(kAutoConnectSerial))
            continue;

        SerialPortConfig config;
        config.portName = port.portName();
        config.baudRate = kAutoConnectBaudRate;

        m_portName = config.portName;
        m_baudRate = config.baudRate;
        m_connecting = true;
        m_statusText->setText(QStringLiteral("自动连接中... %1").arg(m_portName));
        m_portManager->connectToPort(config);
        return true;
    }

    return false;
}

// ── Slot implementations ──

void MainWindow::onSettingsConnect(const SerialPortConfig &config)
{
    m_portName = config.portName;
    m_baudRate = config.baudRate;
    m_connecting = true;
    m_statusText->setText(QStringLiteral("连接中... %1").arg(m_portName));
    m_portManager->connectToPort(config);
}

void MainWindow::onSettingsDisconnect()
{
    m_connecting = false;
    m_portManager->disconnectFromPort();
}

void MainWindow::onConnected()
{
    m_connected = true;
    m_connecting = false;
    m_statusText->setText(QStringLiteral("已连接 — %1").arg(m_portName));
    m_settingsPage->setConnectionState(true);
    m_monitoringPage->onConnectionChanged(true);
}

void MainWindow::onDisconnected()
{
    m_connected = false;
    m_connecting = false;
    m_statusText->setText(QStringLiteral("已断开"));

    m_settingsPage->setConnectionState(false);
    m_monitoringPage->onConnectionChanged(false);
}

void MainWindow::onErrorOccurred(const QString &error)
{
    m_connecting = false;
    m_statusText->setText(QStringLiteral("错误: %1").arg(error));
}

void MainWindow::onMonitoringData(ChargerProtocol::MonitoringData data)
{
    m_monitoringPage->onMonitoringData(data);
}

void MainWindow::onCommandResponse(ChargerProtocol::CommandResponse response)
{
    m_loadControlPage->onCommandResponse(response);
}

void MainWindow::onProtocolInfoReceived(ChargerProtocol::MonitoringData data)
{
    m_protocolAnalysisPage->onProtocolInfoReceived(data);
    m_loadControlPage->onMonitoringData(data);
}

void MainWindow::onRawFrameReceived(const QString &rawData)
{
    m_protocolAnalysisPage->appendPdPacket(rawData);
}

void MainWindow::onRefreshIntervalChanged(int ms)
{
    Q_UNUSED(ms)
}

void MainWindow::onLoadControlSendMode(const QString &mode)
{
    m_portManager->sendSetMode(mode);
    m_monitoringPage->onModeChanged(mode);
}

void MainWindow::onLoadControlSendPower(int currentMa, int voltageMv)
{
    m_portManager->sendSetPower(currentMa, voltageMv);
}

void MainWindow::onLoadControlSendDecoyPdo(int index)
{
    m_portManager->sendSetDecoyPdo(index);
}

void MainWindow::onLoadControlSendDecoyPps(int voltageMv)
{
    m_portManager->sendSetDecoyPps(voltageMv);
}

void MainWindow::onAutoConnectScan()
{
    tryAutoConnect();
}

void MainWindow::onAutoConnectEnabledChanged(bool enabled)
{
    m_autoConnectEnabled = enabled;
    if (enabled)
        tryAutoConnect();
}

// ── Frameless window drag/resize ──

static constexpr int kResizeMargin = 8;

Qt::Edges MainWindow::hitTestEdges(const QPoint &pos, int w, int h, int m)
{
    Qt::Edges edges;
    if (pos.x() <= m)       edges |= Qt::LeftEdge;
    if (pos.x() >= w - m)   edges |= Qt::RightEdge;
    if (pos.y() <= m)       edges |= Qt::TopEdge;
    if (pos.y() >= h - m)   edges |= Qt::BottomEdge;
    return edges;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        if (isMaximized())
            setWindowState(windowState() & ~Qt::WindowMaximized);
        else
            setWindowState(windowState() | Qt::WindowMaximized);
        return true;
    }

    if (event->type() == QEvent::MouseButtonPress) {
        auto *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton) {
            QPoint localPos = mapFromGlobal(me->globalPosition().toPoint());
            auto edges = hitTestEdges(localPos, width(), height(), kResizeMargin);
            if (edges && windowHandle() && windowHandle()->startSystemResize(edges))
                return true;
            m_dragStartPos = me->globalPosition().toPoint();
        }
    }

    if (event->type() == QEvent::MouseMove) {
        auto *me = static_cast<QMouseEvent *>(event);
        if ((me->buttons() & Qt::LeftButton) && windowHandle()) {
            if ((me->globalPosition().toPoint() - m_dragStartPos).manhattanLength() > 10) {
                windowHandle()->startSystemMove();
                return true;
            }
        }
    }

    return ElaWindow::eventFilter(obj, event);
}
