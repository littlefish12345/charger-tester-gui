#include "SettingsPage.h"
#include "ElaComboBox.h"
#include "ElaPushButton.h"
#include "ElaTheme.h"
#include "utils/ThemeUtils.h"
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QSerialPortInfo>
#include <QSettings>

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadSettings();
    updateTheme();
}

void SettingsPage::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // Serial port group
    m_serialGroup = new QGroupBox(QStringLiteral("串口设置"), this);
    auto *serialLayout = new QFormLayout(m_serialGroup);

    m_portCombo = new ElaComboBox(m_serialGroup);
    m_baudCombo = new ElaComboBox(m_serialGroup);
    m_baudCombo->addItems({"9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"});
    m_baudCombo->setCurrentText("115200");

    m_refreshBtn = new QPushButton(QStringLiteral("刷新"), m_serialGroup);
    m_refreshBtn->setObjectName("refreshBtn");
    m_refreshBtn->setMinimumWidth(56);
    m_refreshBtn->setCursor(Qt::PointingHandCursor);

    auto *portRow = new QHBoxLayout();
    portRow->addWidget(m_portCombo, 1);
    portRow->addWidget(m_refreshBtn);
    serialLayout->addRow(QStringLiteral("端口:"), portRow);
    serialLayout->addRow(QStringLiteral("波特率:"), m_baudCombo);

    m_connectBtn = new QPushButton(QStringLiteral("连接"), m_serialGroup);
    serialLayout->addRow(QString(), m_connectBtn);

    // Display group
    m_displayGroup = new QGroupBox(QStringLiteral("显示设置"), this);
    auto *displayLayout = new QFormLayout(m_displayGroup);

    m_refreshSpin = new QSpinBox(m_displayGroup);
    m_refreshSpin->setRange(100, 5000);
    m_refreshSpin->setValue(500);
    m_refreshSpin->setSuffix(" ms");
    m_refreshSpin->setSingleStep(100);
    displayLayout->addRow(QStringLiteral("刷新间隔:"), m_refreshSpin);

    mainLayout->addWidget(m_serialGroup);
    mainLayout->addWidget(m_displayGroup);
    mainLayout->addStretch();

    // Connections
    connect(m_refreshBtn, &QPushButton::clicked, this, &SettingsPage::refreshPorts);
    connect(m_connectBtn, &QPushButton::clicked, this, &SettingsPage::onConnectClicked);
    connect(m_refreshSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val) {
        emit refreshIntervalChanged(val);
        saveSettings();
    });

    refreshPorts();
}

void SettingsPage::updateTheme()
{
    auto c = ThemeColors::current();

    auto groupStyle = QStringLiteral(
        "QGroupBox { color: %1; font-weight: bold; border: 1px solid %2; "
        "border-radius: 6px; margin-top: 12px; padding: 12px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 4px; }"
    ).arg(c.titleText.name(), c.cardBorder.name());

    if (m_serialGroup)  m_serialGroup->setStyleSheet(groupStyle);
    if (m_displayGroup) m_displayGroup->setStyleSheet(groupStyle);

    if (m_refreshBtn) m_refreshBtn->setStyleSheet(QStringLiteral(
        "#refreshBtn { background-color: #1565C0; color: #FFFFFF; border: none; "
        "border-radius: 4px; padding: 4px 14px; font-weight: bold; }"
        "#refreshBtn:hover { background-color: #1976D2; }"));

    setConnectionState(m_connected);
}

void SettingsPage::refreshPorts()
{
    m_portCombo->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    QStringList added;
    for (const auto &port : ports) {
        // Filter out debug/BT/watch ports
        QString name = port.portName().toLower();
        if (name.contains("debug") || name.contains("bluetooth")
            || name.contains("bt-") || name.contains("watch"))
            continue;
        // Skip duplicates
        if (added.contains(port.portName()))
            continue;
        added.append(port.portName());
        m_portCombo->addItem(port.portName() + " - " + port.description(), port.portName());
    }
    if (added.isEmpty())
        m_portCombo->addItem("(无可用串口)");
    emit portsRefreshed();
}

void SettingsPage::onConnectClicked()
{
    if (!m_connected) {
        QString portName = m_portCombo->currentData().toString();
        if (portName.isEmpty())
            return;

        SerialPortConfig config;
        config.portName = portName;
        config.baudRate = m_baudCombo->currentText().toInt();

        m_connectBtn->setText(QStringLiteral("断开"));
        m_portCombo->setEnabled(false);
        m_baudCombo->setEnabled(false);

        emit connectRequested(config);
    } else {
        emit disconnectRequested();
    }
}

void SettingsPage::setConnectionState(bool connected)
{
    m_connected = connected;
    auto c = ThemeColors::current();
    if (connected) {
        m_connectBtn->setText(QStringLiteral("断开"));
        m_connectBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background-color: %1; color: %2; border: none; "
            "border-radius: 4px; padding: 8px 16px; font-weight: bold; }"
            "QPushButton:hover { background-color: %3; }"
        ).arg(c.btnDisconnectBg.name(), c.btnDisconnectText.name(),
             c.btnDisconnectBg.lighter(110).name()));
    } else {
        m_connectBtn->setText(QStringLiteral("连接"));
        m_connectBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background-color: %1; color: %2; border: none; "
            "border-radius: 4px; padding: 8px 16px; font-weight: bold; }"
            "QPushButton:hover { background-color: %3; }"
        ).arg(c.btnConnectBg.name(), c.btnConnectText.name(),
             c.btnConnectBg.lighter(110).name()));
        m_portCombo->setEnabled(true);
        m_baudCombo->setEnabled(true);
    }
}

int SettingsPage::refreshIntervalMs() const
{
    return m_refreshSpin->value();
}

void SettingsPage::saveSettings()
{
    QSettings settings;
    settings.setValue("serial/baudRate", m_baudCombo->currentText().toInt());
    settings.setValue("display/refreshMs", m_refreshSpin->value());
}

void SettingsPage::loadSettings()
{
    QSettings settings;
    int baud = settings.value("serial/baudRate", 115200).toInt();
    m_baudCombo->setCurrentText(QString::number(baud));

    int refresh = settings.value("display/refreshMs", 500).toInt();
    m_refreshSpin->setValue(refresh);
}
