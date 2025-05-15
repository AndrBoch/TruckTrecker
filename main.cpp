#include "mainwindow.h"
#include "logindialog.h"
#include <QFile>

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // Устанавливаем стиль Fusion (рекомендуется)
    a.setStyle("Fusion");

    // Загружаем QSS из ресурсов
    QFile styleFile(":/style/darkstyle.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString style = styleFile.readAll();
        a.setStyleSheet(style);
    }
    QIcon appIcon(":/icon/icon_truck.png"); // путь к ресурсу
    QApplication::setWindowIcon(appIcon);

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
