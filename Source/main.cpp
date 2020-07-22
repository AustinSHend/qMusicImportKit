#include "mainwindow.h"
#include "settingswindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    // Set an environment variable to prevent a Qt bug per https://stackoverflow.com/a/40502585
    qputenv("QT_NO_FT_CACHE", "1");

    QApplication a(argc, argv);

    // Set the application icon
    a.setWindowIcon(QIcon(":/images/icon.png"));

    // These two lines initialize a storage place for QSettings to live in
    QCoreApplication::setOrganizationName("qMusicImportKit");
    QCoreApplication::setApplicationName("qMusicImportKit");

#if defined(Q_OS_LINUX)
    // Set Qt's path variable to include user-defined locations from their shell configuration
    getShellPATH();
#endif


    MainWindow w;
    w.show();


    return a.exec();
}
