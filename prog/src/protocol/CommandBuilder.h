#ifndef COMMAND_BUILDER_H
#define COMMAND_BUILDER_H

#include <QString>
#include <QJsonObject>
#include <QByteArray>

class CommandBuilder
{
public:
    explicit CommandBuilder(const QString &chipId = QStringLiteral("20520501"));

    void setChipId(const QString &chipId);

    // 101: set power target (mA, mV)
    QByteArray buildSetPower(int currentMa, int voltageMv);

    // 102: set power mode ("11"/"12"/"20"/"21")
    QByteArray buildSetMode(const QString &mode);

    // 103: set decoy target — PDO mode (select by index)
    QByteArray buildSetDecoyPdo(int index);

    // 103: set decoy target — PPS mode (set target voltage mV)
    QByteArray buildSetDecoyPps(int voltageMv);

    // 104: switch decoy/listen mode ("trick"/"listen")
    QByteArray buildSetDecoyMode(const QString &mode);

private:
    QByteArray buildFrame(int command, const QJsonObject &json);
    QString m_chipId;
};

#endif // COMMAND_BUILDER_H
