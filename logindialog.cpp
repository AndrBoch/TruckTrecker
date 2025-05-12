#include "logindialog.h"
#include "ui_Logindialog.h"


LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    db = nullptr;
    if (db != nullptr) {
        db->close();
        QMessageBox::information(this, "Ошибка", "База данных не открыта", QMessageBox::Ok);
    }
    else {
        db = new QSqlDatabase(QSqlDatabase::addDatabase("QPSQL"));
        QString dbName("Diplom_db");
        QString host("127.0.0.1");
        QString user("postgres");
        QString password("1234");
        db->setDatabaseName(dbName);
        db->setHostName(host);
        db->setUserName(user);
        db->setPassword(password);
        db->setPort(5432); // 5433
    }
    if (!db->open()) {
        QMessageBox::warning(this, "Ошибка!", "База данных не открылась!", QMessageBox::Ok);
        delete db;
        db = nullptr;
    }
    else {

    }

}

LoginDialog::~LoginDialog()
{
    delete ui;
}

// ВХОД
void LoginDialog::on_pushButton_login_clicked()
{
    QString login = ui->lineEdit_Login->text();
    QString password = ui->lineEdit_Password->text();

    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE login = :login AND password = crypt(:password, password)");
    query.bindValue(":login", login);
    query.bindValue(":password", password);
    query.exec();

    if (query.next()) {
        accept(); // Успешный вход
    } else {
        ui->labelStatus->setText("Неверный логин или пароль");
    }
}
// РЕГИСТРАЦИЯ
void LoginDialog::on_pushButton_registration_clicked()
{
    QString login = ui->lineEdit_Login->text();
        QString password = ui->lineEdit_Password->text();

        if (login.isEmpty() || password.isEmpty()) {
            ui->labelStatus->setText("Введите логин и пароль");
            return;
        }

        // Проверка на существующего пользователя
        QSqlQuery checkQuery;
        checkQuery.prepare("SELECT id FROM users WHERE login = :login");
        checkQuery.bindValue(":login", login);
        checkQuery.exec();

        if (checkQuery.next()) {
            ui->labelStatus->setText("Пользователь с таким логином уже существует");
            return;
        }

        // Регистрация
        QSqlQuery registerQuery;
        registerQuery.prepare("INSERT INTO users (login, password) VALUES (:login, crypt(:password, gen_salt('bf')))");
        registerQuery.bindValue(":login", login);
        registerQuery.bindValue(":password", password);

        if (registerQuery.exec()) {
            ui->labelStatus->setText("Регистрация прошла успешно");
        } else {
            ui->labelStatus->setText("Ошибка регистрации: " + registerQuery.lastError().text());
        }
}
