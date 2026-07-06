#include "ThemeUtils.h"

ThemeColors ThemeColors::current()
{
    auto mode = eTheme->getThemeMode();
    return (mode == ElaThemeType::Light) ? light() : dark();
}

ThemeColors ThemeColors::dark()
{
    ThemeColors c;
    c.cardBg        = QColor("#1E1E2E");
    c.cardBorder    = QColor("#333355");
    c.titleText     = QColor("#CCCCDD");
    c.subtitleText  = QColor("#888899");
    c.primaryText   = QColor("#E0E0E0");
    c.secondaryText = QColor("#AAAAAA");
    c.accentBlue    = QColor("#4FC3F7");
    c.accentGreen   = QColor("#81C784");
    c.accentOrange  = QColor("#FFB74D");
    c.chartBg       = QColor("#1A1A2E");
    c.chartGrid     = QColor("#2A2A45");
    c.chartText     = QColor("#888899");
    c.tableBg       = QColor("#1A1A2E");
    c.tableBorder   = QColor("#333355");
    c.tableHeaderBg = QColor("#222244");
    c.tableText     = QColor("#CCCCDD");
    c.logBg         = QColor("#1A1A2E");
    c.logText       = QColor("#CCCCDD");
    c.btnDefaultBg  = QColor("#333355");
    c.btnDefaultText = QColor("#CCCCDD");
    c.btnConnectBg   = QColor("#2E7D32");
    c.btnConnectText = QColor("#FFFFFF");
    c.btnDisconnectBg = QColor("#C62828");
    c.btnDisconnectText = QColor("#FFFFFF");
    c.statusOk      = QColor("#81C784");
    c.statusErr     = QColor("#FF5252");
    return c;
}

ThemeColors ThemeColors::light()
{
    ThemeColors c;
    c.cardBg        = QColor("#F5F5F5");
    c.cardBorder    = QColor("#D0D0D0");
    c.titleText     = QColor("#333333");
    c.subtitleText  = QColor("#666666");
    c.primaryText   = QColor("#222222");
    c.secondaryText = QColor("#555555");
    c.accentBlue    = QColor("#1565C0");
    c.accentGreen   = QColor("#2E7D32");
    c.accentOrange  = QColor("#E65100");
    c.chartBg       = QColor("#FFFFFF");
    c.chartGrid     = QColor("#E0E0E0");
    c.chartText     = QColor("#777777");
    c.tableBg       = QColor("#FFFFFF");
    c.tableBorder   = QColor("#D0D0D0");
    c.tableHeaderBg = QColor("#EEEEEE");
    c.tableText     = QColor("#333333");
    c.logBg         = QColor("#FAFAFA");
    c.logText       = QColor("#333333");
    c.btnDefaultBg  = QColor("#E0E0E0");
    c.btnDefaultText = QColor("#333333");
    c.btnConnectBg   = QColor("#43A047");
    c.btnConnectText = QColor("#FFFFFF");
    c.btnDisconnectBg = QColor("#E53935");
    c.btnDisconnectText = QColor("#FFFFFF");
    c.statusOk      = QColor("#2E7D32");
    c.statusErr     = QColor("#C62828");
    return c;
}
