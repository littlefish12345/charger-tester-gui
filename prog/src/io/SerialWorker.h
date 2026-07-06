#ifndef SERIAL_WORKER_H
#define SERIAL_WORKER_H

#include <QObject>
#include <QByteArray>
#include <QSerialPort>
#include "SerialPortConfig.h"
#include "protocol/ChargerTypes.h"

class ProtocolParser;

class SerialWorker : public QObject
{
    Q_OBJECT

public:
    explicit SerialWorker(QObject *parent = nullptr);
    ~SerialWorker() override;

public slots:
    void start(const SerialPortConfig &config);
    void stop();
    void sendData(const QByteArray &data);

signals:
    void frameReceived(ChargerProtocol::ParsedFrame frame);
    void errorOccurred(const QString &error);
    void connected();
    void disconnected();

private slots:
    void onReadyRead();

private:
    QSerialPort    *m_serial = nullptr;
    ProtocolParser *m_parser = nullptr;
    QByteArray      m_buffer;
};

#endif // SERIAL_WORKER_H
