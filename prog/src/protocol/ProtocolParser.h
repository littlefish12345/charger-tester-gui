#ifndef PROTOCOL_PARSER_H
#define PROTOCOL_PARSER_H

#include <QByteArray>
#include <QVector>
#include "ChargerTypes.h"

class ProtocolParser
{
public:
    ProtocolParser() = default;

    // Feed raw bytes into the parser. Returns completed frames extracted from the buffer.
    // The input buffer is modified: consumed bytes are removed.
    QVector<ChargerProtocol::ParsedFrame> parse(QByteArray &buffer);

private:
    // Try to extract one complete frame from the beginning of a string view.
    // Returns a valid ParsedFrame if successful, otherwise valid=false.
    ChargerProtocol::ParsedFrame extractFrame(const QString &text, int &consumed);
};

#endif // PROTOCOL_PARSER_H
