#include "ProtocolStatusBar.h"
#include "ElaTheme.h"
#include "utils/ThemeUtils.h"
#include <QLabel>
#include <QHBoxLayout>

ProtocolStatusBar::ProtocolStatusBar(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    updateTheme();
}

void ProtocolStatusBar::setupUi()
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(16);

    m_protocolLabel = new QLabel(QStringLiteral("协议: --"), this);
    m_modeLabel     = new QLabel(QStringLiteral("模式: --"), this);
    m_statusLabel   = new QLabel(QStringLiteral("未连接"), this);

    layout->addWidget(m_protocolLabel);
    layout->addWidget(m_modeLabel);
    layout->addStretch();
    layout->addWidget(m_statusLabel);
}

void ProtocolStatusBar::updateTheme()
{
    auto c = ThemeColors::current();

    if (m_protocolLabel) m_protocolLabel->setStyleSheet(QStringLiteral(
        "color: %1; font-size: 13px; font-weight: bold;"
    ).arg(c.primaryText.name()));

    if (m_modeLabel) m_modeLabel->setStyleSheet(QStringLiteral(
        "color: %1; font-size: 13px;"
    ).arg(c.secondaryText.name()));
}

void ProtocolStatusBar::setProtocolInfo(const QString &info)
{
    if (m_protocolLabel)
        m_protocolLabel->setText(QStringLiteral("协议: %1").arg(info.isEmpty() ? "--" : info));
}

void ProtocolStatusBar::setPowerMode(const QString &mode)
{
    if (m_modeLabel)
        m_modeLabel->setText(QStringLiteral("模式: %1").arg(modeToString(mode)));
}

void ProtocolStatusBar::setConnectionStatus(bool connected)
{
    if (!m_statusLabel)
        return;

    auto c = ThemeColors::current();

    if (connected) {
        m_statusLabel->setText(QStringLiteral("已连接"));
        m_statusLabel->setStyleSheet(QStringLiteral(
            "color: %1; font-size: 13px; font-weight: bold;"
        ).arg(c.statusOk.name()));
    } else {
        m_statusLabel->setText(QStringLiteral("未连接"));
        m_statusLabel->setStyleSheet(QStringLiteral(
            "color: %1; font-size: 13px; font-weight: bold;"
        ).arg(c.statusErr.name()));
    }
}

QString ProtocolStatusBar::modeToString(const QString &mode) const
{
    if (mode == "11") return QStringLiteral("XT 恒流");
    if (mode == "12") return QStringLiteral("XT 恒压");
    if (mode == "20") return QStringLiteral("Type-C 仅诱骗");
    if (mode == "21") return QStringLiteral("Type-C 诱骗恒流");
    return mode.isEmpty() ? "--" : mode;
}
