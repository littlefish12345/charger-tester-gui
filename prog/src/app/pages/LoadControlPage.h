#ifndef LOAD_CONTROL_PAGE_H
#define LOAD_CONTROL_PAGE_H

#include <QWidget>
#include "protocol/ChargerTypes.h"

class ElaDoubleSpinBox;
class ElaPushButton;
class QLabel;
class QGroupBox;

class LoadControlPage : public QWidget
{
    Q_OBJECT

public:
    explicit LoadControlPage(QWidget *parent = nullptr);

signals:
    void sendSetMode(const QString &mode);
    void sendSetPower(int currentMa, int voltageMv);
    void sendSetDecoyPdo(int index);
    void sendSetDecoyPps(int voltageMv);

public slots:
    void onCommandResponse(ChargerProtocol::CommandResponse response);
    void onMonitoringData(ChargerProtocol::MonitoringData data);

private slots:
    void onSendClicked();
    void updateTheme();

private:
    void setupUi();

    ElaDoubleSpinBox *m_currentSpin  = nullptr;
    ElaPushButton    *m_sendButton   = nullptr;
    QLabel           *m_statusLabel  = nullptr;
    QGroupBox        *m_currentGroup = nullptr;

    // Monitoring display
    QLabel *m_voltageValue  = nullptr;
    QLabel *m_currentValue  = nullptr;
    QLabel *m_powerValue    = nullptr;
    QLabel *m_tempValue     = nullptr;
    QLabel *m_fanValue      = nullptr;
};

#endif // LOAD_CONTROL_PAGE_H
