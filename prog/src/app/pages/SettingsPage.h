#ifndef SETTINGS_PAGE_H
#define SETTINGS_PAGE_H

#include <QWidget>
#include "io/SerialPortConfig.h"

class ElaComboBox;
class QCheckBox;
class QGroupBox;
class QPushButton;
class QSpinBox;

class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);

    int refreshIntervalMs() const;
    bool autoConnectEnabled() const;

signals:
    void refreshIntervalChanged(int ms);
    void autoConnectEnabledChanged(bool enabled);
    void portsRefreshed();
    void connectRequested(const SerialPortConfig &config);
    void disconnectRequested();

public slots:
    void setConnectionState(bool connected);

private slots:
    void refreshPorts();
    void onConnectClicked();
    void saveSettings();
    void loadSettings();
    void updateTheme();

private:
    void setupUi();
    void updateManualControls();

    ElaComboBox *m_portCombo    = nullptr;
    ElaComboBox *m_baudCombo    = nullptr;
    QCheckBox   *m_autoConnectCheck = nullptr;
    QSpinBox    *m_refreshSpin  = nullptr;
    QPushButton *m_refreshBtn = nullptr;
    QPushButton *m_connectBtn   = nullptr;
    QGroupBox   *m_serialGroup  = nullptr;
    QGroupBox   *m_displayGroup = nullptr;
    bool         m_connected    = false;
};

#endif // SETTINGS_PAGE_H
