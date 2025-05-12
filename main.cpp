#include "mainwindow.h"
#include "logindialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoginDialog loginDialog;
    if (loginDialog.exec() == QDialog::Accepted) {
            MainWindow w;
            w.showMaximized(); // Показывает сразу на весь экран
            w.setMinimumSize(800, 600);  // Установить минимальные размеры
            w.show();
            return a.exec();
    }
    return 0;
}
