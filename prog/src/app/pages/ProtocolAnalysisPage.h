#ifndef PROTOCOL_ANALYSIS_PAGE_H
#define PROTOCOL_ANALYSIS_PAGE_H

#include <QWidget>
#include "protocol/ChargerTypes.h"

class ElaDoubleSpinBox;
class ElaPushButton;
class QTableWidget;

class ProtocolAnalysisPage : public QWidget
{
    Q_OBJECT

public:
    explicit ProtocolAnalysisPage(QWidget *parent = nullptr);

signals:
    void sendSetDecoyPdo(int index);
    void sendSetDecoyPps(int voltageMv);

public slots:
    void appendPdPacket(const QString &rawData);
    void onProtocolInfoReceived(ChargerProtocol::MonitoringData data);
    void clearLog();

private slots:
    void updateTheme();
    void onPdoClicked();
    void onPpsClicked();

private:
    void setupUi();

    QTableWidget     *m_pdTable       = nullptr;  // PD packets table
    QTableWidget     *m_protoTable    = nullptr;  // PDO/PPS table
    QWidget          *m_pdLabel       = nullptr;
    QWidget          *m_tableLabel    = nullptr;
    ElaDoubleSpinBox *m_ppsVoltageSpin = nullptr;
    ElaPushButton    *m_ppsBtn        = nullptr;

    QVector<int> m_pdoIndices;
};

#endif // PROTOCOL_ANALYSIS_PAGE_H
