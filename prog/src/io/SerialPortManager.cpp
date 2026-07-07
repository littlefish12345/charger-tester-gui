#include "SerialPortManager.h"
#include "SerialWorker.h"
#include "protocol/CommandBuilder.h"
#include <QJsonArray>
#include <QJsonValue>

namespace {

int jsonInt(const QJsonValue &value, int fallback = 0)
{
    if (value.isDouble())
        return value.toInt(fallback);

    if (value.isString()) {
        bool ok = false;
        const int result = value.toString().trimmed().toInt(&ok);
        if (ok)
            return result;
    }

    return fallback;
}

double jsonDouble(const QJsonValue &value, double fallback = 0)
{
    if (value.isDouble())
        return value.toDouble(fallback);

    if (value.isString()) {
        bool ok = false;
        const double result = value.toString().trimmed().toDouble(&ok);
        if (ok)
            return result;
    }

    return fallback;
}

} // namespace

SerialPortManager::SerialPortManager(QObject *parent)
    : QObject(parent)
    , m_builder(new CommandBuilder)
    , m_reconnectTimer(new QTimer(this))
{
    m_reconnectTimer->setInterval(RECONNECT_MS);
    connect(m_reconnectTimer, &QTimer::timeout, this, &SerialPortManager::onReconnectTimeout);
}

SerialPortManager::~SerialPortManager()
{
    disconnectFromPort();
}

void SerialPortManager::connectToPort(const SerialPortConfig &config)
{
    m_config = config;

    // Clean up previous
    if (m_thread) {
        disconnectFromPort();
    }

    m_worker = new SerialWorker;
    m_thread = new QThread(this);

    m_worker->moveToThread(m_thread);

    // Worker lifecycle
    connect(m_thread, &QThread::started, m_worker, [this, config]() {
        m_worker->start(config);
    });
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);

    // Worker signals
    connect(m_worker, &SerialWorker::frameReceived, this, &SerialPortManager::onFrameReceived);
    connect(m_worker, &SerialWorker::connected, this, [this]() {
        m_reconnectAttempts = 0;
        stopReconnectTimer();
        emit connected();
    });
    connect(m_worker, &SerialWorker::disconnected, this, [this]() {
        emit disconnected();
    });
    connect(m_worker, &SerialWorker::errorOccurred, this, [this](const QString &err) {
        emit errorOccurred(err);
        disconnectFromPort();
    });

    m_thread->start();
}

void SerialPortManager::disconnectFromPort()
{
    stopReconnectTimer();

    if (m_worker) {
        const auto connectionType =
            (m_thread && m_thread->isRunning() && QThread::currentThread() != m_thread)
                ? Qt::BlockingQueuedConnection
                : Qt::DirectConnection;
        QMetaObject::invokeMethod(m_worker, "stop", connectionType);
    }
    m_worker = nullptr;

    if (m_thread) {
        m_thread->quit();
        m_thread->wait(3000);
        m_thread->deleteLater();
        m_thread = nullptr;
    }
}

bool SerialPortManager::isConnected() const
{
    return m_worker != nullptr && m_thread != nullptr && m_thread->isRunning();
}

void SerialPortManager::sendSetPower(int currentMa, int voltageMv)
{
    QByteArray data = m_builder->buildSetPower(currentMa, voltageMv);
    sendRawData(data);
}

void SerialPortManager::sendSetMode(const QString &mode)
{
    QByteArray data = m_builder->buildSetMode(mode);
    sendRawData(data);
}

void SerialPortManager::sendSetDecoyPdo(int index)
{
    QByteArray data = m_builder->buildSetDecoyPdo(index);
    sendRawData(data);
}

void SerialPortManager::sendSetDecoyPps(int voltageMv)
{
    QByteArray data = m_builder->buildSetDecoyPps(voltageMv);
    sendRawData(data);
}

void SerialPortManager::sendSetDecoyMode(const QString &mode)
{
    QByteArray data = m_builder->buildSetDecoyMode(mode);
    sendRawData(data);
}

void SerialPortManager::sendRawData(const QByteArray &data)
{
    if (m_worker) {
        QMetaObject::invokeMethod(m_worker, "sendData",
                                  Qt::QueuedConnection,
                                  Q_ARG(QByteArray, data));
    }
}

void SerialPortManager::setChipId(const QString &chipId)
{
    m_builder->setChipId(chipId);
}

void SerialPortManager::onFrameReceived(ChargerProtocol::ParsedFrame frame)
{
    emit rawFrameReceived(frame.rawData);
    routeFrame(frame);
}

void SerialPortManager::routeFrame(const ChargerProtocol::ParsedFrame &frame)
{
    using namespace ChargerProtocol;

    switch (frame.command) {
    case Command::RESPONSE: {
        CommandResponse resp;
        resp.command = jsonInt(frame.json.value("command"));
        resp.result  = jsonInt(frame.json.value("result"), -1);
        resp.valid   = true;
        emit commandResponseReceived(resp);
        break;
    }
    case Command::REPORT_PROTOCOLS: {
        // 201: PDO/PPS protocol info
        MonitoringData data;

        if (frame.json.contains("pdo") && frame.json["pdo"].isArray()) {
            const auto pdoArr = frame.json["pdo"].toArray();
            for (const auto &val : pdoArr) {
                QJsonObject obj = val.toObject();
                PdoEntry entry;
                entry.index     = jsonInt(obj.value("index"), data.pdoList.size());
                entry.voltageMv = jsonDouble(obj.value("V"));
                entry.currentMa = jsonDouble(obj.value("I"));
                data.pdoList.append(entry);
            }
        }

        if (frame.json.contains("pps") && frame.json["pps"].isObject()) {
            QJsonObject ppsObj = frame.json["pps"].toObject();
            data.pps.minVoltageMv = jsonDouble(ppsObj.value("min_V"));
            data.pps.maxVoltageMv = jsonDouble(ppsObj.value("max_V"));
            data.pps.currentMa    = jsonDouble(ppsObj.value("I"));
            data.pps.valid        = true;
        }

        if (!data.pdoList.isEmpty()) {
            data.voltageMv = data.pdoList.first().voltageMv;
            data.currentMa = data.pdoList.first().currentMa;
        }

        emit protocolInfoReceived(data);
        break;
    }
    case Command::REPORT_PD_PACKET: {
        // 202: PD listener packet bytes
        PdPacketData data;
        data.rawData = frame.rawData;

        if (frame.json.contains("pack") && frame.json["pack"].isArray()) {
            const auto packArr = frame.json["pack"].toArray();
            for (const auto &val : packArr) {
                int byte = 0;
                bool ok = false;

                if (val.isDouble()) {
                    byte = val.toInt();
                    ok = true;
                } else if (val.isString()) {
                    QString text = val.toString().trimmed();
                    int base = 10;
                    if (text.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive)) {
                        text = text.mid(2);
                        base = 16;
                    }
                    byte = text.toInt(&ok, base);
                }

                if (ok)
                    data.bytes.append(qBound(0, byte, 255));
            }
        }

        data.valid = !data.bytes.isEmpty();
        emit pdPacketReceived(data);
        break;
    }
    case Command::REPORT_STATUS: {
        // 203: INA228 running V/I status
        MonitoringData data;
        data.voltageMv = jsonDouble(frame.json.value("V"));
        data.currentMa = jsonDouble(frame.json.value("I"));
        emit monitoringDataReceived(data);
        break;
    }
    default:
        break;
    }
}

void SerialPortManager::onReconnectTimeout()
{
    if (m_reconnectAttempts >= MAX_RECONNECT) {
        stopReconnectTimer();
        emit errorOccurred(QStringLiteral("Reconnection failed after %1 attempts").arg(MAX_RECONNECT));
        disconnectFromPort();
        return;
    }

    m_reconnectAttempts++;

    // Clean up and retry
    if (m_worker) {
        QMetaObject::invokeMethod(m_worker, "stop", Qt::QueuedConnection);
    }
    if (m_thread) {
        m_thread->quit();
        m_thread->wait(1000);
        m_thread->deleteLater();
        m_thread = nullptr;
    }

    m_worker = new SerialWorker;
    m_thread = new QThread(this);
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started, m_worker, [this]() {
        m_worker->start(m_config);
    });
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &SerialWorker::frameReceived, this, &SerialPortManager::onFrameReceived);
    connect(m_worker, &SerialWorker::connected, this, [this]() {
        m_reconnectAttempts = 0;
        stopReconnectTimer();
        emit connected();
    });
    connect(m_worker, &SerialWorker::disconnected, this, [this]() {
        emit disconnected();
    });
    connect(m_worker, &SerialWorker::errorOccurred, this, [this](const QString &err) {
        Q_UNUSED(err)
        // Reconnect timer already running, will retry
    });

    m_thread->start();
}

void SerialPortManager::startReconnectTimer()
{
    if (!m_reconnectTimer->isActive()) {
        m_reconnectTimer->start();
    }
}

void SerialPortManager::stopReconnectTimer()
{
    m_reconnectTimer->stop();
}
