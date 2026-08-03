#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("opt-gui");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("OPENOS");

    // Use Fusion style for consistent look across platforms
    app.setStyle("Fusion");

    MainWindow window;
    window.show();

    return app.exec();
}
