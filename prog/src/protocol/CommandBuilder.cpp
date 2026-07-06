#include "CommandBuilder.h"
#include <QJsonDocument>

CommandBuilder::CommandBuilder(const QString &chipId)
    : m_chipId(chipId)
{
}

void CommandBuilder::setChipId(const QString &chipId)
{
    m_chipId = chipId;
}

QByteArray CommandBuilder::buildSetPower(int currentMa, int voltageMv)
{
    QJsonObject json;
    json["I"] = QString::number(currentMa);
    json["V"] = QString::number(voltageMv);
    return buildFrame(101, json);
}

QByteArray CommandBuilder::buildSetMode(const QString &mode)
{
    QJsonObject json;
    json["mode"] = mode;
    return buildFrame(102, json);
}

QByteArray CommandBuilder::buildSetDecoyPdo(int index)
{
    QJsonObject json;
    json["mode"]  = QStringLiteral("pdo");
    json["index"] = index;
    return buildFrame(103, json);
}

QByteArray CommandBuilder::buildSetDecoyPps(int voltageMv)
{
    QJsonObject json;
    json["mode"] = QStringLiteral("pps");
    json["V"]    = QString::number(voltageMv);
    return buildFrame(103, json);
}

QByteArray CommandBuilder::buildFrame(int command, const QJsonObject &json)
{
    QJsonDocument doc(json);
    QString payload = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    QString frame = QStringLiteral("<%1,%2>%3")
                        .arg(m_chipId)
                        .arg(command, 3, 10, QChar('0'))
                        .arg(payload);
    return frame.toUtf8();
}
