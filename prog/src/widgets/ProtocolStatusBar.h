#ifndef PROTOCOL_STATUS_BAR_H
#define PROTOCOL_STATUS_BAR_H

#include <QWidget>

class QLabel;

class ProtocolStatusBar : public QWidget
{
    Q_OBJECT

public:
    explicit ProtocolStatusBar(QWidget *parent = nullptr);

    void setProtocolInfo(const QString &info);
    void setPowerMode(const QString &mode);
    void setConnectionStatus(bool connected);

private slots:
    void updateTheme();

private:
    void setupUi();
    QString modeToString(const QString &mode) const;

    QLabel *m_protocolLabel = nullptr;
    QLabel *m_modeLabel     = nullptr;
    QLabel *m_statusLabel   = nullptr;
};

#endif // PROTOCOL_STATUS_BAR_H
