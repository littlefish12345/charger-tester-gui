#include "ProtocolAnalysisPage.h"
#include "ElaDoubleSpinBox.h"
#include "ElaPushButton.h"
#include "ElaTheme.h"
#include "utils/ThemeUtils.h"
#include <QTableWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QHeaderView>
#include <QTime>
#include <QPushButton>

ProtocolAnalysisPage::ProtocolAnalysisPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    updateTheme();
}

void ProtocolAnalysisPage::setupUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(8);

    auto *splitter = new QSplitter(Qt::Vertical, this);

    // ---- Upper: PD packet monitor table ----
    auto *pdContainer = new QWidget(splitter);
    auto *pdLayout = new QVBoxLayout(pdContainer);
    pdLayout->setContentsMargins(0, 0, 0, 0);

    m_pdLabel = new QLabel(QStringLiteral("PD 数据包监听"), pdContainer);
    m_pdLabel->setStyleSheet(QStringLiteral("font-size: 12px; font-weight: bold;"));

    m_pdTable = new QTableWidget(0, 5, pdContainer);
    m_pdTable->setHorizontalHeaderLabels({
        QStringLiteral("时间"), QStringLiteral("方向"), QStringLiteral("类型"),
        QStringLiteral("数据"), QStringLiteral("原始数据")});
    m_pdTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_pdTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_pdTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_pdTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_pdTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_pdTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_pdTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pdTable->verticalHeader()->setVisible(false);

    pdLayout->addWidget(m_pdLabel);
    pdLayout->addWidget(m_pdTable);

    // ---- Lower: PDO/PPS table + decoy controls ----
    auto *protoContainer = new QWidget(splitter);
    auto *protoLayout = new QVBoxLayout(protoContainer);
    protoLayout->setContentsMargins(0, 0, 0, 0);

    m_tableLabel = new QLabel(QStringLiteral("可用协议 (PDO/PPS)"), protoContainer);
    m_tableLabel->setStyleSheet(QStringLiteral("font-size: 12px; font-weight: bold;"));

    m_protoTable = new QTableWidget(0, 3, protoContainer);
    m_protoTable->setHorizontalHeaderLabels({
        QStringLiteral("类型"), QStringLiteral("参数"), QStringLiteral("操作")});
    m_protoTable->horizontalHeader()->setStretchLastSection(true);
    m_protoTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_protoTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_protoTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_protoTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_protoTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_protoTable->verticalHeader()->setVisible(false);

    // PPS voltage selector row
    auto *ppsRow = new QHBoxLayout();
    auto *ppsLabel = new QLabel(QStringLiteral("PPS 诱骗电压:"), protoContainer);

    m_ppsVoltageSpin = new ElaDoubleSpinBox(protoContainer);
    m_ppsVoltageSpin->setRange(0, 21.0);
    m_ppsVoltageSpin->setDecimals(2);
    m_ppsVoltageSpin->setSuffix(" V");
    m_ppsVoltageSpin->setSingleStep(0.1);
    m_ppsVoltageSpin->setMinimumWidth(200);
    m_ppsVoltageSpin->setEnabled(false);

    m_ppsBtn = new ElaPushButton(QStringLiteral("确认诱骗"), protoContainer);
    m_ppsBtn->setEnabled(false);
    m_ppsBtn->setMinimumWidth(100);

    ppsRow->addWidget(ppsLabel);
    ppsRow->addWidget(m_ppsVoltageSpin, 1);
    ppsRow->addWidget(m_ppsBtn);

    protoLayout->addWidget(m_tableLabel);
    protoLayout->addWidget(m_protoTable);
    protoLayout->addLayout(ppsRow);

    splitter->addWidget(pdContainer);
    splitter->addWidget(protoContainer);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    layout->addWidget(splitter);

    connect(m_ppsBtn, &ElaPushButton::clicked, this, &ProtocolAnalysisPage::onPpsClicked);
}

void ProtocolAnalysisPage::updateTheme()
{
    auto c = ThemeColors::current();

    auto labelStyle = [&](QWidget *w) {
        if (auto *l = qobject_cast<QLabel*>(w))
            l->setStyleSheet(QStringLiteral("color: %1; font-size: 12px; font-weight: bold;")
                                 .arg(c.subtitleText.name()));
    };
    labelStyle(m_pdLabel);
    labelStyle(m_tableLabel);

    auto tableStyle = QStringLiteral(
        "QTableWidget { background-color: %1; color: %2; border: 1px solid %3; gridline-color: %3; }"
        "QHeaderView::section { background-color: %4; color: %5; padding: 4px; border: none; }"
        "QPushButton { background-color: %6; color: %2; border: none; border-radius: 3px; padding: 4px 12px; }"
        "QPushButton:hover { background-color: %4; }"
    ).arg(c.tableBg.name(), c.tableText.name(), c.tableBorder.name(),
          c.tableHeaderBg.name(), c.subtitleText.name(), c.btnDefaultBg.name());

    if (m_pdTable)    m_pdTable->setStyleSheet(tableStyle);
    if (m_protoTable) m_protoTable->setStyleSheet(tableStyle);
}

void ProtocolAnalysisPage::appendPdPacket(const QString &rawData)
{
    if (!m_pdTable)
        return;

    int row = m_pdTable->rowCount();
    m_pdTable->insertRow(row);

    auto timeStr = QTime::currentTime().toString("hh:mm:ss.zzz");
    m_pdTable->setItem(row, 0, new QTableWidgetItem(timeStr));

    // Parse direction and type from raw data
    // Format expected: <chip_id,cmd>{json} or "SRC->SNK:Type Data"
    // For now just put raw data in the last column
    m_pdTable->setItem(row, 4, new QTableWidgetItem(rawData));

    // Auto-scroll
    m_pdTable->scrollToBottom();

    // Limit rows to 500
    while (m_pdTable->rowCount() > 500)
        m_pdTable->removeRow(0);
}

void ProtocolAnalysisPage::onProtocolInfoReceived(ChargerProtocol::MonitoringData data)
{
    if (!m_protoTable)
        return;

    m_protoTable->setRowCount(0);
    m_pdoIndices.clear();

    for (const auto &pdo : data.pdoList) {
        int row = m_protoTable->rowCount();
        m_protoTable->insertRow(row);
        m_protoTable->setItem(row, 0, new QTableWidgetItem(
            QStringLiteral("PDO %1").arg(pdo.index)));
        m_protoTable->setItem(row, 1, new QTableWidgetItem(
            QStringLiteral("%1V / %2A")
                .arg(pdo.voltageV(), 0, 'f', 1)
                .arg(pdo.currentA(), 0, 'f', 2)));

        auto *btn = new QPushButton(QStringLiteral("诱骗"), m_protoTable);
        btn->setProperty("pdoIndex", pdo.index);
        connect(btn, &QPushButton::clicked, this, &ProtocolAnalysisPage::onPdoClicked);
        m_protoTable->setCellWidget(row, 2, btn);
        m_pdoIndices.append(pdo.index);
    }

    if (data.pps.valid) {
        m_ppsVoltageSpin->setRange(data.pps.minVoltageMv / 1000.0,
                                   data.pps.maxVoltageMv / 1000.0);
        m_ppsVoltageSpin->setValue(data.pps.maxVoltageMv / 1000.0);
        m_ppsVoltageSpin->setEnabled(true);
        m_ppsBtn->setEnabled(true);
    } else {
        m_ppsVoltageSpin->setEnabled(false);
        m_ppsBtn->setEnabled(false);
    }

    if (data.pdoList.isEmpty() && !data.pps.valid) {
        int row = m_protoTable->rowCount();
        m_protoTable->insertRow(row);
        m_protoTable->setItem(row, 0, new QTableWidgetItem(QStringLiteral("--")));
        m_protoTable->setItem(row, 1, new QTableWidgetItem(
            QStringLiteral("(未检测到充电头/PD 协议)")));
    }
}

void ProtocolAnalysisPage::onPdoClicked()
{
    auto *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    int index = btn->property("pdoIndex").toInt();
    emit sendSetDecoyPdo(index);
}

void ProtocolAnalysisPage::onPpsClicked()
{
    int mv = static_cast<int>(m_ppsVoltageSpin->value() * 1000);
    if (mv > 0)
        emit sendSetDecoyPps(mv);
}

void ProtocolAnalysisPage::clearLog()
{
    if (m_pdTable)
        m_pdTable->setRowCount(0);
    if (m_protoTable)
        m_protoTable->setRowCount(0);
}
