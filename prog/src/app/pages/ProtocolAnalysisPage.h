#ifndef PROTOCOL_ANALYSIS_PAGE_H
#define PROTOCOL_ANALYSIS_PAGE_H

#include <QWidget>
#include "protocol/ChargerTypes.h"

class ElaDoubleSpinBox;
class ElaPushButton;
class QLabel;
class QTableWidget;

class ProtocolAnalysisPage : public QWidget
{
    Q_OBJECT

public:
    explicit ProtocolAnalysisPage(QWidget *parent = nullptr);

signals:
    void sendSetDecoyPdo(int index);
    void sendSetDecoyPps(int voltageMv);
    void sendSetDecoyMode(const QString &mode);

public slots:
    void onPdPacketReceived(ChargerProtocol::PdPacketData data);
    void onProtocolInfoReceived(ChargerProtocol::MonitoringData data);
    void onInaStatusReceived(ChargerProtocol::MonitoringData data);
    void clearLog();

private slots:
    void updateTheme();
    void onPdoClicked();
    void onPpsClicked();
    void onDecoyModeClicked();
    void clearPdPackets();

private:
    void setupUi();
    void updateDecoyModeButton();

    QTableWidget     *m_pdTable       = nullptr;  // PD packets table
    QTableWidget     *m_protoTable    = nullptr;  // PDO/PPS table
    QWidget          *m_pdLabel       = nullptr;
    QWidget          *m_tableLabel    = nullptr;
    QLabel           *m_voltageLabel  = nullptr;
    QLabel           *m_currentLabel  = nullptr;
    QLabel           *m_powerLabel    = nullptr;
    ElaPushButton    *m_clearPdBtn    = nullptr;
    ElaPushButton    *m_decoyModeBtn  = nullptr;
    ElaDoubleSpinBox *m_ppsVoltageSpin = nullptr;
    ElaPushButton    *m_ppsBtn        = nullptr;

    QVector<int> m_pdoIndices;
    QString m_decoyMode = QStringLiteral("trick");

    // Cache last Source_Capabilities PDOs for RDO type detection
    QVector<quint32> m_lastSourcePdos;
};

#endif // PROTOCOL_ANALYSIS_PAGE_H
