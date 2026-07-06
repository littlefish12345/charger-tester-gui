#include <QApplication>
#include <QFont>
#include <ElaApplication.h>
#include <ElaTheme.h>
#include "app/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("ChargerTester");

    // Initialize ElaWidgetTools
    ElaApplication::getInstance()->init();

    // Set dark theme
    eTheme->setThemeMode(ElaThemeType::Dark);

    // Set CJK font
    QFont font = app.font();
    font.setPixelSize(14);
    app.setFont(font);

    MainWindow window;
    window.show();

    return app.exec();
}
