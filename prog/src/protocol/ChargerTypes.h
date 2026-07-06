#ifndef CHARGER_TYPES_H
#define CHARGER_TYPES_H

#include <QString>
#include <QJsonObject>
#include <QVector>

namespace ChargerProtocol {

// Power mode constants (command 102)
namespace PowerMode {
    constexpr const char* XT_CC          = "11";
    constexpr const char* XT_CV          = "12";
    constexpr const char* TYPEC_DECOY    = "20";
    constexpr const char* TYPEC_DECOY_CC = "21";
}

// Command codes
namespace Command {
    constexpr int SET_POWER        = 101;
    constexpr int SET_MODE         = 102;
    constexpr int SET_DECOY        = 103;
    constexpr int RESPONSE         = 200;
    constexpr int REPORT_PROTOCOLS = 201;
    constexpr int REPORT_STATUS    = 202;
}

// PDO entry (from 202 report)
struct PdoEntry {
    int    index = 0;
    double voltageMv = 0;
    double currentMa = 0;

    double voltageV() const { return voltageMv / 1000.0; }
    double currentA() const { return currentMa  / 1000.0; }
};

// PPS info (from 202 report)
struct PpsInfo {
    double minVoltageMv = 0;
    double maxVoltageMv = 0;
    double currentMa    = 0;
    bool   valid        = false;
};

// Parsed frame from the text protocol
struct ParsedFrame {
    QString    chipId;
    int        command  = 0;
    QJsonObject json;
    bool       valid    = false;
    QString    rawData;
};

// Monitoring data (from command 202)
struct MonitoringData {
    QVector<PdoEntry> pdoList;
    PpsInfo           pps;
    double currentMa  = 0;
    double voltageMv  = 0;

    double currentA() const { return currentMa / 1000.0; }
    double voltageV() const { return voltageMv / 1000.0; }
    double powerW()   const { return currentA() * voltageV(); }
};

// Command response (from command 200)
struct CommandResponse {
    int  command = 0;
    int  result  = -1;
    bool valid   = false;
};

} // namespace ChargerProtocol

#endif // CHARGER_TYPES_H
