#include "SerialWorker.h"
#include "protocol/ProtocolParser.h"

SerialWorker::SerialWorker(QObject *parent)
    : QObject(parent)
    , m_parser(new ProtocolParser)
{
}

SerialWorker::~SerialWorker()
{
    stop();
    delete m_parser;
}

void SerialWorker::start(const SerialPortConfig &config)
{
    if (m_serial) {
        stop();
    }

    m_serial = new QSerialPort(this);
    m_serial->setPortName(config.portName);
    m_serial->setBaudRate(config.baudRate);
    m_serial->setDataBits(config.dataBits);
    m_serial->setParity(config.parity);
    m_serial->setStopBits(config.stopBits);
    m_serial->setFlowControl(config.flowControl);

    connect(m_serial, &QSerialPort::readyRead, this, &SerialWorker::onReadyRead);

    if (m_serial->open(QIODevice::ReadWrite)) {
        m_buffer.clear();
        emit connected();
    } else {
        emit errorOccurred(m_serial->errorString());
        delete m_serial;
        m_serial = nullptr;
    }
}

void SerialWorker::stop()
{
    if (m_serial) {
        m_serial->close();
        delete m_serial;
        m_serial = nullptr;
    }
    m_buffer.clear();
    emit disconnected();
}

void SerialWorker::sendData(const QByteArray &data)
{
    if (m_serial && m_serial->isOpen()) {
        m_serial->write(data);
    }
}

void SerialWorker::onReadyRead()
{
    if (!m_serial)
        return;

    m_buffer.append(m_serial->readAll());

    auto frames = m_parser->parse(m_buffer);
    for (const auto &frame : frames) {
        emit frameReceived(frame);
    }
}
