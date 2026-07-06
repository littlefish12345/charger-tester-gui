#ifndef SERIAL_PORT_MANAGER_H
#define SERIAL_PORT_MANAGER_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include "SerialPortConfig.h"
#include "protocol/ChargerTypes.h"

class SerialWorker;
class CommandBuilder;

class SerialPortManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialPortManager(QObject *parent = nullptr);
    ~SerialPortManager() override;

    void connectToPort(const SerialPortConfig &config);
    void disconnectFromPort();
    bool isConnected() const;

    void sendSetPower(int currentMa, int voltageMv);
    void sendSetMode(const QString &mode);
    void sendSetDecoyPdo(int index);
    void sendSetDecoyPps(int voltageMv);
    void sendRawData(const QByteArray &data);

    void setChipId(const QString &chipId);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &error);
    void monitoringDataReceived(ChargerProtocol::MonitoringData data);
    void commandResponseReceived(ChargerProtocol::CommandResponse response);
    void protocolInfoReceived(ChargerProtocol::MonitoringData data);
    void rawFrameReceived(const QString &rawData);

private slots:
    void onFrameReceived(ChargerProtocol::ParsedFrame frame);
    void onReconnectTimeout();

private:
    void startReconnectTimer();
    void stopReconnectTimer();
    void routeFrame(const ChargerProtocol::ParsedFrame &frame);

    SerialWorker   *m_worker  = nullptr;
    QThread        *m_thread  = nullptr;
    CommandBuilder *m_builder = nullptr;
    QTimer         *m_reconnectTimer = nullptr;
    SerialPortConfig m_config;
    int              m_reconnectAttempts = 0;
    static constexpr int MAX_RECONNECT   = 5;
    static constexpr int RECONNECT_MS    = 2000;
};

#endif // SERIAL_PORT_MANAGER_H
