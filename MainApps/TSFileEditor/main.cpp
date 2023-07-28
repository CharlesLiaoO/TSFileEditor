#include "MainWindow.h"
#include <QApplication>
#include <QTranslator>
#include <QDebug>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    QLocale loc(QLocale::Chinese, QLocale::China);
    QLocale loc(QLocale::English, QLocale::UnitedStates);

    QTranslator trQt;
//    qDebug()<< "trQt"<< trQt.load(loc, "qt", "_", "translations");
    qApp->installTranslator(&trQt);

    QTranslator trApp;
    qDebug()<< "trApp"<< trApp.load(loc, "app", "_", ".");
    qApp->installTranslator(&trApp);

    MainWindow w;
    w.show();
//    QMessageBox::information(0, " ", "");
//    QMessageBox::aboutQt(0);
    return a.exec();
}
