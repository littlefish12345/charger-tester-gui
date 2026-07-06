#ifndef SERIAL_PORT_CONFIG_H
#define SERIAL_PORT_CONFIG_H

#include <QString>
#include <QSerialPort>

struct SerialPortConfig {
    QString             portName;
    int                 baudRate    = 115200;
    QSerialPort::DataBits dataBits  = QSerialPort::Data8;
    QSerialPort::Parity   parity    = QSerialPort::NoParity;
    QSerialPort::StopBits stopBits  = QSerialPort::OneStop;
    QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;
};

#endif // SERIAL_PORT_CONFIG_H
