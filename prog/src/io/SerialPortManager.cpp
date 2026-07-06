#include "SerialPortManager.h"
#include "SerialWorker.h"
#include "protocol/CommandBuilder.h"
#include <QJsonArray>

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
        startReconnectTimer();
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
        resp.command = frame.json.value("command").toString().toInt();
        resp.result  = frame.json.value("result").toString().toInt();
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
                entry.index     = obj.value("index").toInt();
                entry.voltageMv = obj.value("V").toString().toDouble();
                entry.currentMa = obj.value("I").toString().toDouble();
                data.pdoList.append(entry);
            }
        }

        if (frame.json.contains("pps") && frame.json["pps"].isObject()) {
            QJsonObject ppsObj = frame.json["pps"].toObject();
            data.pps.minVoltageMv = ppsObj.value("min_V").toString().toDouble();
            data.pps.maxVoltageMv = ppsObj.value("max_V").toString().toDouble();
            data.pps.currentMa    = ppsObj.value("I").toString().toDouble();
            data.pps.valid        = true;
        }

        if (!data.pdoList.isEmpty()) {
            data.voltageMv = data.pdoList.first().voltageMv;
            data.currentMa = data.pdoList.first().currentMa;
        }

        emit protocolInfoReceived(data);
        break;
    }
    case Command::REPORT_STATUS: {
        // 202: running V/I status
        MonitoringData data;
        data.voltageMv = frame.json.value("V").toString().toDouble();
        data.currentMa = frame.json.value("I").toString().toDouble();
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
