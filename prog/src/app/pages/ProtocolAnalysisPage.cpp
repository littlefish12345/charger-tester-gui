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
#include <QLineEdit>
#include <QTime>
#include <QPushButton>
#include <QStringList>

namespace {

QString hexByte(int byte)
{
    return QStringLiteral("%1")
        .arg(byte & 0xff, 2, 16, QLatin1Char('0'))
        .toUpper();
}

QString hexWord32(quint32 value)
{
    return QStringLiteral("0x%1")
        .arg(value, 8, 16, QLatin1Char('0'))
        .toUpper();
}

QString bytesToHex(const QVector<int> &bytes)
{
    QStringList parts;
    parts.reserve(bytes.size());
    for (int byte : bytes)
        parts << hexByte(byte);
    return parts.join(QLatin1Char(' '));
}

quint32 readLe32(const QVector<int> &bytes, int offset)
{
    return static_cast<quint32>(bytes[offset] & 0xff)
        | (static_cast<quint32>(bytes[offset + 1] & 0xff) << 8)
        | (static_cast<quint32>(bytes[offset + 2] & 0xff) << 16)
        | (static_cast<quint32>(bytes[offset + 3] & 0xff) << 24);
}

QString voltageText(int mv)
{
    return QStringLiteral("%1V").arg(mv / 1000.0, 0, 'f', 2);
}

QString currentText(int ma)
{
    return QStringLiteral("%1A").arg(ma / 1000.0, 0, 'f', 2);
}

QString powerText(int mw)
{
    return QStringLiteral("%1W").arg(mw / 1000.0, 0, 'f', 2);
}

QString specRevisionName(int spec)
{
    switch (spec) {
    case 0: return QStringLiteral("1.0");
    case 1: return QStringLiteral("2.0");
    case 2: return QStringLiteral("3.0");
    default: return QStringLiteral("保留");
    }
}

QString controlMessageName(int type)
{
    switch (type) {
    case 1: return QStringLiteral("GoodCRC");
    case 2: return QStringLiteral("GotoMin");
    case 3: return QStringLiteral("Accept");
    case 4: return QStringLiteral("Reject");
    case 5: return QStringLiteral("Ping");
    case 6: return QStringLiteral("PS_RDY");
    case 7: return QStringLiteral("Get_Source_Cap");
    case 8: return QStringLiteral("Get_Sink_Cap");
    case 9: return QStringLiteral("DR_Swap");
    case 10: return QStringLiteral("PR_Swap");
    case 11: return QStringLiteral("VCONN_Swap");
    case 12: return QStringLiteral("Wait");
    case 13: return QStringLiteral("Soft_Reset");
    case 14: return QStringLiteral("Data_Reset");
    case 15: return QStringLiteral("Data_Reset_Complete");
    case 16: return QStringLiteral("Not_Supported");
    case 17: return QStringLiteral("Get_Source_Cap_Extended");
    case 18: return QStringLiteral("Get_Status");
    case 19: return QStringLiteral("FR_Swap");
    case 20: return QStringLiteral("Get_PPS_Status");
    case 21: return QStringLiteral("Get_Country_Codes");
    case 22: return QStringLiteral("Get_Sink_Cap_Extended");
    case 23: return QStringLiteral("Get_Source_Info");
    case 24: return QStringLiteral("Get_Revision");
    default: return QStringLiteral("未知控制(%1)").arg(type);
    }
}

QString dataMessageName(int type)
{
    switch (type) {
    case 1: return QStringLiteral("Source_Capabilities");
    case 2: return QStringLiteral("Request");
    case 3: return QStringLiteral("BIST");
    case 4: return QStringLiteral("Sink_Capabilities");
    case 5: return QStringLiteral("Battery_Status");
    case 6: return QStringLiteral("Alert");
    case 7: return QStringLiteral("Get_Country_Info");
    case 8: return QStringLiteral("Enter_USB");
    case 9: return QStringLiteral("EPR_Request");
    case 10: return QStringLiteral("EPR_Mode");
    case 11: return QStringLiteral("Source_Info");
    case 12: return QStringLiteral("Revision");
    default: return QStringLiteral("未知数据(%1)").arg(type);
    }
}

QString extendedMessageName(int type)
{
    switch (type) {
    case 1: return QStringLiteral("Source_Capabilities_Extended");
    case 2: return QStringLiteral("Status");
    case 3: return QStringLiteral("Get_Battery_Cap");
    case 4: return QStringLiteral("Get_Battery_Status");
    case 5: return QStringLiteral("Battery_Capabilities");
    case 6: return QStringLiteral("Get_Manufacturer_Info");
    case 7: return QStringLiteral("Manufacturer_Info");
    case 8: return QStringLiteral("Security_Request");
    case 9: return QStringLiteral("Security_Response");
    case 10: return QStringLiteral("Firmware_Update_Request");
    case 11: return QStringLiteral("Firmware_Update_Response");
    case 12: return QStringLiteral("PPS_Status");
    case 13: return QStringLiteral("Country_Info");
    case 14: return QStringLiteral("Country_Codes");
    case 15: return QStringLiteral("Sink_Capabilities_Extended");
    case 16: return QStringLiteral("Extended_Control");
    case 17: return QStringLiteral("EPR_Source_Capabilities");
    case 18: return QStringLiteral("EPR_Sink_Capabilities");
    default: return QStringLiteral("未知扩展(%1)").arg(type);
    }
}

QString decodePdo(quint32 object)
{
    const int supplyType = static_cast<int>((object >> 30) & 0x3);
    switch (supplyType) {
    case 0: {
        const int mv = static_cast<int>(((object >> 10) & 0x3ff) * 50);
        const int ma = static_cast<int>((object & 0x3ff) * 10);
        return QStringLiteral("%1V/%2A").arg(mv / 1000.0, 0, 'f', 1).arg(ma / 1000.0, 0, 'f', 2);
    }
    case 1: {
        const int mvMax = static_cast<int>(((object >> 20) & 0x3ff) * 50);
        const int mvMin = static_cast<int>(((object >> 10) & 0x3ff) * 50);
        const int mw    = static_cast<int>((object & 0x3ff) * 250);
        return QStringLiteral("电池 %1-%2V/%3W").arg(mvMin / 1000.0, 0, 'f', 1).arg(mvMax / 1000.0, 0, 'f', 1).arg(mw / 1000.0, 0, 'f', 2);
    }
    case 2: {
        const int mvMax = static_cast<int>(((object >> 20) & 0x3ff) * 50);
        const int mvMin = static_cast<int>(((object >> 10) & 0x3ff) * 50);
        const int ma    = static_cast<int>((object & 0x3ff) * 10);
        return QStringLiteral("%1-%2V/%3A").arg(mvMin / 1000.0, 0, 'f', 1).arg(mvMax / 1000.0, 0, 'f', 1).arg(ma / 1000.0, 0, 'f', 2);
    }
    case 3: {
        const int augmentedType = static_cast<int>((object >> 28) & 0x3);
        if (augmentedType == 0) {
            const int mvMax = static_cast<int>(((object >> 17) & 0xff) * 100);
            const int mvMin = static_cast<int>(((object >> 8) & 0xff) * 100);
            const int ma    = static_cast<int>((object & 0x7f) * 50);
            return QStringLiteral("PPS %1-%2V/%3A").arg(mvMin / 1000.0, 0, 'f', 1).arg(mvMax / 1000.0, 0, 'f', 1).arg(ma / 1000.0, 0, 'f', 2);
        }
        return QStringLiteral("APDO");
    }
    default:
        return QString();
    }
}

static bool isPpsPdo(const QVector<quint32> &pdos, int objPos)
{
    if (objPos < 1 || objPos > pdos.size())
        return false;
    const quint32 pdo = pdos[objPos - 1];
    return (pdo >> 30) == 3 && ((pdo >> 28) & 0x3) == 0;
}

QString decodeRdo(quint32 object, const QVector<quint32> &sourcePdos)
{
    // Object Position in RDO is already 1-based (1-7 = PDO1-PDO7)
    // Object Position is 1-based
    const int pdoIndex = static_cast<int>((object >> 28) & 0x7);

    if (isPpsPdo(sourcePdos, pdoIndex)) {
        const int ppsMv = static_cast<int>(((object >> 9) & 0x7ff) * 20);
        const int ppsMa = static_cast<int>((object & 0x7f) * 50);
        return QStringLiteral("PDO%1 PPS %2V/%3A  [%4]")
            .arg(pdoIndex).arg(ppsMv / 1000.0, 0, 'f', 2).arg(ppsMa / 1000.0, 0, 'f', 2)
            .arg(hexWord32(object));
    }

    // Fixed/Variable: look up PDO to show its voltage
    const int opMa = static_cast<int>(((object >> 10) & 0x3ff) * 10);
    if (pdoIndex >= 1 && pdoIndex <= sourcePdos.size()) {
        const quint32 pdo = sourcePdos[pdoIndex - 1];
        const int supplyType = static_cast<int>((pdo >> 30) & 0x3);
        double pdoV = 0;
        if (supplyType == 0)
            pdoV = static_cast<int>(((pdo >> 10) & 0x3ff) * 50) / 1000.0;
        else if (supplyType == 2)
            pdoV = static_cast<int>(((pdo >> 10) & 0x3ff) * 50) / 1000.0;
        if (pdoV > 0)
            return QStringLiteral("PDO%1 %2V/%3A").arg(pdoIndex).arg(pdoV, 0, 'f', 1).arg(opMa / 1000.0, 0, 'f', 2);
    }
    return QStringLiteral("PDO%1 %2A").arg(pdoIndex).arg(opMa / 1000.0, 0, 'f', 2);
}

QString decodeDataObject(int number, int messageType, quint32 object, const QVector<quint32> &sourcePdos)
{
    Q_UNUSED(number);
    if (messageType == 1 || messageType == 4)
        return decodePdo(object);

    if (messageType == 2)
        return decodeRdo(object, sourcePdos);

    // Unknown: return raw hex
    return hexWord32(object);
}

void decodePdPacket(const ChargerProtocol::PdPacketData &packet, QVector<quint32> &sourcePdos,
                    QString *direction,
                    QString *messageType,
                    QString *details)
{
    if (packet.bytes.size() < 2) {
        *direction = QStringLiteral("--");
        *messageType = QStringLiteral("无效");
        *details = QStringLiteral("PD 包长度不足，至少需要 2 字节 Header");
        return;
    }

    const quint16 header = static_cast<quint16>((packet.bytes[0] & 0xff)
        | ((packet.bytes[1] & 0xff) << 8));
    const int type = header & 0x1f;
    const int dataRole = (header >> 5) & 0x1;
    const int specRevision = (header >> 6) & 0x3;
    const int powerRole = (header >> 8) & 0x1;
    const int messageId = (header >> 9) & 0x7;
    const int objectCount = (header >> 12) & 0x7;
    const bool extended = (header & 0x8000) != 0;

    *direction = powerRole ? QStringLiteral("SRC") : QStringLiteral("SNK");
    *messageType = extended
        ? QStringLiteral("扩展 %1").arg(extendedMessageName(type))
        : (objectCount == 0
               ? controlMessageName(type)
               : dataMessageName(type));

    if (extended) {
        *details = QStringLiteral("(扩展消息)");
        return;
    }

    if (objectCount == 0) {
        // Control message — generate short description
        switch (type) {
        case 1:  *details = QStringLiteral("CRC确认"); break;
        case 2:  *details = QStringLiteral("转到最小"); break;
        case 3:  *details = QStringLiteral("接受"); break;
        case 4:  *details = QStringLiteral("拒绝"); break;
        case 5:  *details = QStringLiteral("Ping"); break;
        case 6:  *details = QStringLiteral("电源就绪"); break;
        case 7:  *details = QStringLiteral("请求Source能力"); break;
        case 8:  *details = QStringLiteral("请求Sink能力"); break;
        case 9:  break; // DR_Swap
        case 10: break; // PR_Swap
        case 11: break; // VCONN_Swap
        case 12: *details = QStringLiteral("等待"); break;
        case 13: *details = QStringLiteral("软复位"); break;
        case 14: *details = QStringLiteral("数据复位"); break;
        case 15: *details = QStringLiteral("复位完成"); break;
        case 16: *details = QStringLiteral("不支持"); break;
        case 17: *details = QStringLiteral("请求扩展能力"); break;
        case 18: *details = QStringLiteral("请求状态"); break;
        case 19: break; // FR_Swap
        case 20: *details = QStringLiteral("请求PPS状态"); break;
        case 21: *details = QStringLiteral("请求国家码"); break;
        case 22: *details = QStringLiteral("请求扩展Sink能力"); break;
        case 23: *details = QStringLiteral("请求Source信息"); break;
        case 24: *details = QStringLiteral("请求版本"); break;
        default: break;
        }
        return;
    }

    const int availableObjects = qMin(objectCount, (packet.bytes.size() - 2) / 4);

    // Cache Source_Capabilities PDOs for RDO type detection
    if (type == 1) {
        sourcePdos.clear();
        for (int i = 0; i < availableObjects; ++i)
            sourcePdos.append(readLe32(packet.bytes, 2 + i * 4));
    }

    QStringList parts;
    for (int i = 0; i < availableObjects; ++i) {
        const quint32 object = readLe32(packet.bytes, 2 + i * 4);
        parts << decodeDataObject(i + 1, type, object, sourcePdos);
    }

    *details = parts.join(QStringLiteral(", "));
}

} // namespace

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

    auto *pdTitleRow = new QHBoxLayout();
    m_clearPdBtn = new ElaPushButton(QStringLiteral("清除"), pdContainer);
    m_clearPdBtn->setMinimumWidth(72);
    pdTitleRow->addWidget(m_pdLabel);
    pdTitleRow->addStretch();
    pdTitleRow->addWidget(m_clearPdBtn);

    m_pdTable = new QTableWidget(0, 4, pdContainer);
    m_pdTable->setHorizontalHeaderLabels({
        QStringLiteral("时间"), QStringLiteral("来源"), QStringLiteral("类型"),
        QStringLiteral("内容")});
    m_pdTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_pdTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_pdTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_pdTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_pdTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_pdTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pdTable->setTextElideMode(Qt::ElideRight);
    m_pdTable->setWordWrap(false);
    m_pdTable->verticalHeader()->setVisible(false);

    pdLayout->addLayout(pdTitleRow);
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

    auto *modeRow = new QHBoxLayout();
    m_decoyModeBtn = new ElaPushButton(protoContainer);
    m_decoyModeBtn->setMinimumWidth(120);
    updateDecoyModeButton();
    modeRow->addWidget(m_decoyModeBtn);

    auto makeStatusLabel = [&](const QString &text) -> QLabel* {
        auto *label = new QLabel(text, protoContainer);
        label->setMinimumWidth(96);
        return label;
    };
    m_voltageLabel = makeStatusLabel(QStringLiteral("电压 -- V"));
    m_currentLabel = makeStatusLabel(QStringLiteral("电流 -- A"));
    m_powerLabel = makeStatusLabel(QStringLiteral("功率 -- W"));
    modeRow->addSpacing(12);
    modeRow->addWidget(m_voltageLabel);
    modeRow->addWidget(m_currentLabel);
    modeRow->addWidget(m_powerLabel);
    modeRow->addStretch();

    // PPS voltage selector row
    auto *ppsRow = new QHBoxLayout();
    auto *ppsLabel = new QLabel(QStringLiteral("PPS 诱骗电压:"), protoContainer);

    m_ppsVoltageSpin = new ElaDoubleSpinBox(protoContainer);
    m_ppsVoltageSpin->setRange(0, 21.0);
    m_ppsVoltageSpin->setDecimals(2);
    m_ppsVoltageSpin->setSuffix(" V");
    m_ppsVoltageSpin->setSingleStep(0.02);
    m_ppsVoltageSpin->setMinimumWidth(200);
    m_ppsVoltageSpin->setEnabled(false);
    // Fix decimal point input with proper floating-point validator
    m_ppsVoltageSpin->setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);
    if (auto *le = m_ppsVoltageSpin->findChild<QLineEdit*>()) {
        le->setValidator(new QDoubleValidator(0, 21.0, 2, le));
        le->setClearButtonEnabled(false);
    }

    m_ppsBtn = new ElaPushButton(QStringLiteral("确认诱骗"), protoContainer);
    m_ppsBtn->setEnabled(false);
    m_ppsBtn->setMinimumWidth(100);

    ppsRow->addWidget(ppsLabel);
    ppsRow->addWidget(m_ppsVoltageSpin, 1);
    ppsRow->addWidget(m_ppsBtn);

    protoLayout->addWidget(m_tableLabel);
    protoLayout->addLayout(modeRow);
    protoLayout->addWidget(m_protoTable);
    protoLayout->addLayout(ppsRow);

    splitter->addWidget(pdContainer);
    splitter->addWidget(protoContainer);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    layout->addWidget(splitter);

    connect(m_clearPdBtn, &ElaPushButton::clicked, this, &ProtocolAnalysisPage::clearPdPackets);
    connect(m_ppsBtn, &ElaPushButton::clicked, this, &ProtocolAnalysisPage::onPpsClicked);
    connect(m_decoyModeBtn, &ElaPushButton::clicked,
            this, &ProtocolAnalysisPage::onDecoyModeClicked);
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

    auto valueStyle = QStringLiteral("color: %1; font-size: 12px; font-weight: bold;")
                          .arg(c.primaryText.name());
    if (m_voltageLabel) m_voltageLabel->setStyleSheet(valueStyle);
    if (m_currentLabel) m_currentLabel->setStyleSheet(valueStyle);
    if (m_powerLabel)   m_powerLabel->setStyleSheet(valueStyle);

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

void ProtocolAnalysisPage::onPdPacketReceived(ChargerProtocol::PdPacketData data)
{
    if (!m_pdTable)
        return;

    QString direction;
    QString messageType;
    QString details;
    decodePdPacket(data, m_lastSourcePdos, &direction, &messageType, &details);

    int row = m_pdTable->rowCount();
    m_pdTable->insertRow(row);

    QString rawHex = bytesToHex(data.bytes);

    auto timeStr = QTime::currentTime().toString("hh:mm:ss.zzz");
    auto setCell = [&](int column, const QString &text) {
        auto *item = new QTableWidgetItem(text);
        item->setToolTip(rawHex);
        m_pdTable->setItem(row, column, item);
    };

    setCell(0, timeStr);
    setCell(1, direction);
    setCell(2, messageType);
    setCell(3, details);

    // Auto-scroll
    m_pdTable->scrollToBottom();

    // Limit rows to 500
    while (m_pdTable->rowCount() > 500)
        m_pdTable->removeRow(0);
}

void ProtocolAnalysisPage::onInaStatusReceived(ChargerProtocol::MonitoringData data)
{
    if (m_voltageLabel)
        m_voltageLabel->setText(QStringLiteral("电压 %1 V").arg(data.voltageV(), 0, 'f', 3));
    if (m_currentLabel)
        m_currentLabel->setText(QStringLiteral("电流 %1 A").arg(data.currentA(), 0, 'f', 3));
    if (m_powerLabel)
        m_powerLabel->setText(QStringLiteral("功率 %1 W").arg(data.powerW(), 0, 'f', 2));
}

void ProtocolAnalysisPage::onProtocolInfoReceived(ChargerProtocol::MonitoringData data)
{
    if (!m_protoTable)
        return;

    m_protoTable->setRowCount(0);
    m_pdoIndices.clear();

    const bool haveRawPdos = (m_lastSourcePdos.size() == data.pdoList.size());
    for (int i = 0; i < data.pdoList.size(); ++i) {
        const auto &pdo = data.pdoList[i];
        int row = m_protoTable->rowCount();
        m_protoTable->insertRow(row);
        // Use decoded PDO from Source_Capabilities if available, else simple V/A
        QString desc;
        if (haveRawPdos)
            desc = decodePdo(m_lastSourcePdos[i]);
        if (desc.isEmpty())
            desc = QStringLiteral("%1V/%2A").arg(pdo.voltageV(), 0, 'f', 1).arg(pdo.currentA(), 0, 'f', 2);

        m_protoTable->setItem(row, 0, new QTableWidgetItem(
            QStringLiteral("PDO %1  %2V").arg(pdo.index).arg(pdo.voltageV(), 0, 'f', 1)));
        m_protoTable->setItem(row, 1, new QTableWidgetItem(desc));

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
    int mv = qRound(m_ppsVoltageSpin->value() * 1000.0);
    if (mv > 0)
        emit sendSetDecoyPps(mv);
}

void ProtocolAnalysisPage::onDecoyModeClicked()
{
    m_decoyMode = (m_decoyMode == QStringLiteral("listen"))
                      ? QStringLiteral("trick")
                      : QStringLiteral("listen");
    updateDecoyModeButton();
    emit sendSetDecoyMode(m_decoyMode);

    // Clear PDO/PPS when switching to listen
    if (m_decoyMode == QStringLiteral("listen")) {
        m_protoTable->setRowCount(0);
        m_pdoIndices.clear();
        m_lastSourcePdos.clear();
        m_ppsVoltageSpin->setEnabled(false);
        m_ppsBtn->setEnabled(false);
        if (m_voltageLabel) m_voltageLabel->setText(QStringLiteral("电压 -- V"));
        if (m_currentLabel) m_currentLabel->setText(QStringLiteral("电流 -- A"));
        if (m_powerLabel)   m_powerLabel->setText(QStringLiteral("功率 -- W"));
    }
}

void ProtocolAnalysisPage::updateDecoyModeButton()
{
    if (!m_decoyModeBtn)
        return;

    m_decoyModeBtn->setText(m_decoyMode == QStringLiteral("listen")
                                ? QStringLiteral("模式：监听")
                                : QStringLiteral("模式：诱骗"));
}

void ProtocolAnalysisPage::clearPdPackets()
{
    if (m_pdTable)
        m_pdTable->setRowCount(0);
}

void ProtocolAnalysisPage::clearLog()
{
    if (m_pdTable)
        m_pdTable->setRowCount(0);
    if (m_protoTable)
        m_protoTable->setRowCount(0);
}
