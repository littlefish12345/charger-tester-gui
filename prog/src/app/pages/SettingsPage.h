#ifndef SETTINGS_PAGE_H
#define SETTINGS_PAGE_H

#include <QWidget>
#include "io/SerialPortConfig.h"

class ElaComboBox;
class QCheckBox;
class QGroupBox;
class QLineEdit;
class QPushButton;
class QSpinBox;

class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);

    int refreshIntervalMs() const;
    bool autoConnectEnabled() const;
    QString chipId() const;
    void refreshPortList();
    void selectPort(const QString &portName);

signals:
    void refreshIntervalChanged(int ms);
    void autoConnectEnabledChanged(bool enabled);
    void chipIdChanged(const QString &chipId);
    void portsRefreshed();
    void connectRequested(const SerialPortConfig &config);
    void disconnectRequested();

public slots:
    void setConnectionState(bool connected);

private slots:
    void onConnectClicked();
    void saveSettings();
    void loadSettings();
    void updateTheme();

private:
    void setupUi();
    void updateManualControls();
    bool hasSelectablePort() const;

    ElaComboBox *m_portCombo    = nullptr;
    ElaComboBox *m_baudCombo    = nullptr;
    QLineEdit   *m_chipIdEdit   = nullptr;
    QCheckBox   *m_autoConnectCheck = nullptr;
    QSpinBox    *m_refreshSpin  = nullptr;
    QPushButton *m_refreshBtn = nullptr;
    QPushButton *m_connectBtn   = nullptr;
    QGroupBox   *m_serialGroup  = nullptr;
    QGroupBox   *m_displayGroup = nullptr;
    bool         m_connected    = false;
    bool         m_connecting   = false;
};

#endif // SETTINGS_PAGE_H
