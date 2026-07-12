#include "ProtocolParser.h"
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonParseError>

// Match <chipId,command>{json} on a single line
// chipId: any chars except ',' and '>', command: 3 digits
static const QRegularExpression FRAME_RE(
    QStringLiteral(R"(^<([^,>]+),(\d{3})>(\{.*\})\s*$)")
);

QVector<ChargerProtocol::ParsedFrame> ProtocolParser::parse(QByteArray &buffer)
{
    QVector<ChargerProtocol::ParsedFrame> frames;

    if (buffer.isEmpty())
        return frames;

    QString text = QString::fromUtf8(buffer);

    // Find the last newline (handles \n and \r\n mixed)
    int lastEnd = text.lastIndexOf('\n');
    if (lastEnd < 0)
        return frames;

    // Keep incomplete last line in buffer (skip \n and optional preceding \r)
    int skip = 1;
    if (lastEnd > 0 && text[lastEnd - 1] == '\r') {
        lastEnd--;
        skip = 2;
    }
    QString complete = text.left(lastEnd);
    buffer = text.mid(lastEnd + skip).toUtf8();

    // Split on \n, strip \r
    const QStringList lines = complete.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        QString clean = line.trimmed();
        if (clean.endsWith('\r'))
            clean.chop(1);
        QRegularExpressionMatch match = FRAME_RE.match(clean);
        if (!match.hasMatch())
            continue;

        ChargerProtocol::ParsedFrame frame;
        frame.chipId  = match.captured(1);
        frame.command = match.captured(2).toInt();
        frame.rawData = match.captured(0);

        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson(match.captured(3).toUtf8(), &jsonError);
        if (jsonError.error != QJsonParseError::NoError)
            continue;

        if (!doc.isObject())
            continue;

        frame.json  = doc.object();
        frame.valid = true;
        frames.append(frame);
    }

    return frames;
}
