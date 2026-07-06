#ifndef THEME_UTILS_H
#define THEME_UTILS_H

#include <QColor>
#include <ElaTheme.h>

// Helper to get theme-aware colors
struct ThemeColors {
    QColor cardBg;
    QColor cardBorder;
    QColor titleText;
    QColor subtitleText;
    QColor primaryText;
    QColor secondaryText;
    QColor accentBlue;
    QColor accentGreen;
    QColor accentOrange;
    QColor chartBg;
    QColor chartGrid;
    QColor chartText;
    QColor tableBg;
    QColor tableBorder;
    QColor tableHeaderBg;
    QColor tableText;
    QColor logBg;
    QColor logText;
    QColor btnDefaultBg;
    QColor btnDefaultText;
    QColor btnConnectBg;
    QColor btnConnectText;
    QColor btnDisconnectBg;
    QColor btnDisconnectText;
    QColor statusOk;
    QColor statusErr;

    static ThemeColors current();
    static ThemeColors dark();
    static ThemeColors light();
};

#endif // THEME_UTILS_H
