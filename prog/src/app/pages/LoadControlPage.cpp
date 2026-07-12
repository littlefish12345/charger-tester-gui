#include "LoadControlPage.h"
#include "ElaDoubleSpinBox.h"
#include "ElaPushButton.h"
#include "utils/ThemeUtils.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFrame>

LoadControlPage::LoadControlPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    updateTheme();
}

void LoadControlPage::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // Target current group
    m_currentGroup = new QGroupBox(QStringLiteral("恒流控制"), this);
    auto *currentLayout = new QHBoxLayout(m_currentGroup);

    auto *label = new QLabel(QStringLiteral("目标电流:"), m_currentGroup);

    m_currentSpin = new ElaDoubleSpinBox(m_currentGroup);
    m_currentSpin->setRange(0, 10.0);
    m_currentSpin->setDecimals(3);
    m_currentSpin->setValue(1.0);
    m_currentSpin->setSuffix(" A");
    m_currentSpin->setSingleStep(0.1);
    m_currentSpin->setMinimumWidth(160);

    m_sendButton = new ElaPushButton(QStringLiteral("设置电流"), m_currentGroup);
    m_sendButton->setMinimumWidth(100);

    currentLayout->addWidget(label);
    currentLayout->addWidget(m_currentSpin, 1);
    currentLayout->addWidget(m_sendButton);

    // Status label
    m_statusLabel = new QLabel(this);

    // Real-time monitoring group
    auto *monitorGroup = new QGroupBox(QStringLiteral("实时监测"), this);
    auto *monitorLayout = new QFormLayout(monitorGroup);

    auto makeValueLabel = [&](const QString &suffix) -> QLabel* {
        auto *lbl = new QLabel("-- " + suffix, monitorGroup);
        lbl->setStyleSheet(QStringLiteral("font-size: 14px; font-weight: bold;"));
        return lbl;
    };

    m_voltageValue = makeValueLabel("V");
    m_currentValue = makeValueLabel("A");
    m_powerValue   = makeValueLabel("W");
    m_tempValue    = makeValueLabel("°C");
    m_fanValue     = makeValueLabel("RPM");

    monitorLayout->addRow(QStringLiteral("电压:"), m_voltageValue);
    monitorLayout->addRow(QStringLiteral("电流:"), m_currentValue);
    monitorLayout->addRow(QStringLiteral("功率:"), m_powerValue);
    monitorLayout->addRow(QStringLiteral("温度:"), m_tempValue);
    monitorLayout->addRow(QStringLiteral("风扇转速:"), m_fanValue);

    mainLayout->addWidget(m_currentGroup);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(monitorGroup);
    mainLayout->addStretch();

    connect(m_sendButton, &ElaPushButton::clicked, this, &LoadControlPage::onSendClicked);
}

void LoadControlPage::updateTheme()
{
    auto c = ThemeColors::current();

    auto groupStyle = QStringLiteral(
        "QGroupBox { color: %1; font-weight: bold; border: 1px solid %2; "
        "border-radius: 6px; margin-top: 12px; padding: 12px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 4px; }"
    ).arg(c.titleText.name(), c.cardBorder.name());

    if (m_currentGroup) m_currentGroup->setStyleSheet(groupStyle);

    auto labelStyle = QStringLiteral("color: %1;").arg(c.primaryText.name());
    if (m_statusLabel) m_statusLabel->setStyleSheet(labelStyle);
}

void LoadControlPage::onSendClicked()
{
    int currentMa = qRound(m_currentSpin->value() * 1000.0);
    emit sendSetMode(QStringLiteral("11")); // XT constant current
    emit sendSetPower(currentMa, 0);
    m_statusLabel->setText(QStringLiteral("已发送: 恒流 %1mA").arg(currentMa));
}

void LoadControlPage::onCommandResponse(ChargerProtocol::CommandResponse response)
{
    if (!response.valid) return;

    auto c = ThemeColors::current();
    if (response.result == 0) {
        m_statusLabel->setStyleSheet(QStringLiteral("color: %1; font-size: 12px;").arg(c.statusOk.name()));
    } else {
        m_statusLabel->setStyleSheet(QStringLiteral("color: %1; font-size: 12px;").arg(c.statusErr.name()));
    }
}

void LoadControlPage::onMonitoringData(ChargerProtocol::MonitoringData data)
{
    if (m_voltageValue)
        m_voltageValue->setText(QStringLiteral("%1 V").arg(data.voltageV(), 0, 'f', 3));
    if (m_currentValue)
        m_currentValue->setText(QStringLiteral("%1 A").arg(data.currentA(), 0, 'f', 3));
    if (m_powerValue)
        m_powerValue->setText(QStringLiteral("%1 W").arg(data.powerW(), 0, 'f', 2));
}
