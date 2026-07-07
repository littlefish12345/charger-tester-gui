#include "ProtocolParser.h"
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonParseError>

// Match <chipId,command> prefix. Uplink chipId is accepted as-is and is not
// compared against the configured downlink chipId.
static const QRegularExpression HEADER_RE(
    QStringLiteral(R"(^<([^,>]+),(\d{3})>)")
);

QVector<ChargerProtocol::ParsedFrame> ProtocolParser::parse(QByteArray &buffer)
{
    QVector<ChargerProtocol::ParsedFrame> frames;

    if (buffer.isEmpty())
        return frames;

    QString text = QString::fromUtf8(buffer);

    int pos = 0;
    while (pos < text.size()) {
        int start = text.indexOf('<', pos);
        if (start < 0)
            break;

        QString remainder = text.mid(start);
        int consumed = 0;
        ChargerProtocol::ParsedFrame frame = extractFrame(remainder, consumed);

        if (frame.valid) {
            frames.append(frame);
            pos = start + consumed;
        } else {
            int nextLt = text.indexOf('<', start + 1);
            if (nextLt < 0) {
                pos = start;
                break;
            }
            pos = start + 1;
        }
    }

    if (pos > 0)
        buffer.remove(0, text.left(pos).toUtf8().size());

    return frames;
}

ChargerProtocol::ParsedFrame ProtocolParser::extractFrame(const QString &text, int &consumed)
{
    ChargerProtocol::ParsedFrame frame;
    consumed = 0;

    QRegularExpressionMatch match = HEADER_RE.match(text);
    if (!match.hasMatch())
        return frame;

    frame.chipId  = match.captured(1).toUpper();
    frame.command = match.captured(2).toInt();

    int jsonStart = match.capturedEnd();
    if (jsonStart >= text.size() || text[jsonStart] != '{')
        return frame;

    // Count braces to find the complete JSON (handles nested objects/arrays)
    int depth = 0;
    bool inString = false;
    int jsonEnd = -1;
    for (int i = jsonStart; i < text.size(); ++i) {
        QChar ch = text[i];
        if (inString) {
            if (ch == '\\') {
                ++i; // skip escaped char
            } else if (ch == '"') {
                inString = false;
            }
        } else {
            if (ch == '"') {
                inString = true;
            } else if (ch == '{' || ch == '[') {
                ++depth;
            } else if (ch == '}' || ch == ']') {
                --depth;
                if (depth == 0) {
                    jsonEnd = i + 1;
                    break;
                }
            }
        }
    }

    if (jsonEnd < 0)
        return frame; // incomplete JSON, wait for more data

    QString jsonStr = text.mid(jsonStart, jsonEnd - jsonStart);
    frame.rawData = text.left(jsonEnd);

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError)
        return frame;

    if (!doc.isObject())
        return frame;

    frame.json  = doc.object();
    frame.valid = true;
    consumed    = jsonEnd;

    return frame;
}
