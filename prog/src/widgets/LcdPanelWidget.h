#ifndef LCD_PANEL_WIDGET_H
#define LCD_PANEL_WIDGET_H

#include <QWidget>

class ElaLCDNumber;

class LcdPanelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LcdPanelWidget(QWidget *parent = nullptr);

    void setVoltage(double voltageV);
    void setCurrent(double currentA);
    void setPower(double powerW);
    void clear();

private slots:
    void updateTheme();

private:
    void setupUi();

    ElaLCDNumber *m_voltageLcd   = nullptr;
    ElaLCDNumber *m_currentLcd   = nullptr;
    ElaLCDNumber *m_powerLcd     = nullptr;
    QWidget      *m_voltageCard  = nullptr;
    QWidget      *m_currentCard  = nullptr;
    QWidget      *m_powerCard    = nullptr;
    QWidget      *m_voltageTitle = nullptr;
    QWidget      *m_currentTitle = nullptr;
    QWidget      *m_powerTitle   = nullptr;
    QWidget      *m_voltageUnit  = nullptr;
    QWidget      *m_currentUnit  = nullptr;
    QWidget      *m_powerUnit    = nullptr;
};

#endif // LCD_PANEL_WIDGET_H
