#include "LcdPanelWidget.h"
#include "ElaLCDNumber.h"
#include "ElaTheme.h"
#include "utils/ThemeUtils.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>

LcdPanelWidget::LcdPanelWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    updateTheme();
}

void LcdPanelWidget::setupUi()
{
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(16);

    auto createCard = [this](const QString &title, const QString &unit,
                             QWidget *&card, QWidget *&titleWidget,
                             ElaLCDNumber *&lcd, QWidget *&unitWidget)
    {
        card = new QFrame(this);
        card->setObjectName("lcdCard");

        auto *layout = new QVBoxLayout(card);
        layout->setSpacing(4);

        auto *tl = new QLabel(title, card);
        tl->setAlignment(Qt::AlignCenter);
        titleWidget = tl;

        lcd = new ElaLCDNumber(card);
        lcd->setDigitCount(7);
        lcd->setSegmentStyle(QLCDNumber::Flat);

        auto *ul = new QLabel(unit, card);
        ul->setAlignment(Qt::AlignCenter);
        unitWidget = ul;

        layout->addWidget(tl);
        layout->addWidget(lcd, 1, Qt::AlignCenter);
        layout->addWidget(ul);

        return card;
    };

    auto *vCard = createCard(QStringLiteral("电压"), "V",
                              m_voltageCard, m_voltageTitle,
                              m_voltageLcd, m_voltageUnit);
    auto *cCard = createCard(QStringLiteral("电流"), "A",
                              m_currentCard, m_currentTitle,
                              m_currentLcd, m_currentUnit);
    auto *pCard = createCard(QStringLiteral("功率"), "W",
                              m_powerCard, m_powerTitle,
                              m_powerLcd, m_powerUnit);

    mainLayout->addWidget(vCard);
    mainLayout->addWidget(cCard);
    mainLayout->addWidget(pCard);
}

void LcdPanelWidget::updateTheme()
{
    auto c = ThemeColors::current();

    auto styleCard = [&](QWidget *card) {
        if (card) card->setStyleSheet(QStringLiteral(
            "#lcdCard { background-color: %1; border-radius: 8px; "
            "border: 1px solid %2; padding: 12px; }"
        ).arg(c.cardBg.name(), c.cardBorder.name()));
    };
    styleCard(m_voltageCard);
    styleCard(m_currentCard);
    styleCard(m_powerCard);

    auto styleLabel = [&](QWidget *w, int fontSize) {
        auto *label = qobject_cast<QLabel*>(w);
        if (label) label->setStyleSheet(
            QStringLiteral("color: %1; font-size: %2px;").arg(c.subtitleText.name()).arg(fontSize));
    };
    styleLabel(m_voltageTitle, 12);
    styleLabel(m_currentTitle, 12);
    styleLabel(m_powerTitle, 12);
    styleLabel(m_voltageUnit, 11);
    styleLabel(m_currentUnit, 11);
    styleLabel(m_powerUnit, 11);

    auto styleLcd = [&](ElaLCDNumber *lcd, const QColor &color) {
        if (lcd) lcd->setStyleSheet(QStringLiteral(
            "background-color: transparent; border: none; color: %1; font-size: 28px;"
        ).arg(color.name()));
    };
    styleLcd(m_voltageLcd, c.accentBlue);
    styleLcd(m_currentLcd, c.accentGreen);
    styleLcd(m_powerLcd,  c.accentOrange);
}

void LcdPanelWidget::setVoltage(double v)
{
    if (m_voltageLcd)
        m_voltageLcd->display(QString::number(v, 'f', 3));
}

void LcdPanelWidget::setCurrent(double a)
{
    if (m_currentLcd)
        m_currentLcd->display(QString::number(a, 'f', 3));
}

void LcdPanelWidget::setPower(double w)
{
    if (m_powerLcd)
        m_powerLcd->display(QString::number(w, 'f', 2));
}

void LcdPanelWidget::clear()
{
    setVoltage(0);
    setCurrent(0);
    setPower(0);
}
