#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    //, mapView(new QWebEngineView(this))
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentWidget(ui->pageList);

    //QIcon appIcon(":/icon/truck_icon.png"); // путь к ресурсу
    //QApplication::setWindowIcon(appIcon);
    //this->setWindowIcon(QIcon(":/icon/truck_icon.png"));

    if (!mapView) {
        mapView = new QWebEngineView(this);
        QVBoxLayout *layout = new QVBoxLayout(ui->mapPlaceholder);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(mapView);
    }

    if (!routeView) {
        routeView = new QWebEngineView(this);
        QVBoxLayout *layout = new QVBoxLayout(ui->mapPlaceholder_route);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(routeView);
    }
    if (!updateTimer){
        updateTimer = new QTimer(this);
    }



    loadVehiclesTable();

    connect(ui->btnList, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(ui->btnBackToList, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(ui->btnAdd, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(ui->btnRoutes, &QPushButton::clicked, this, &MainWindow::onStopClicked);

    ui->textEdit_info->hide();
    ui->textEdit_info->clear();
    //ui->textEdit_info->

    ui->dateEditRouteDate->setDate(QDate::currentDate());
    ui->label_hideNumber->hide();

    ui->lineEdit_search->setPlaceholderText("Например: A123BC116");
    QRegularExpression regExp("^[ABEKMHOPCTYX]\\d{3}[ABEKMHOPCTYX]{2}\\d{2,3}$");
    ui->lineEdit_search->setValidator(new QRegularExpressionValidator(regExp, this));

    ui->lineEdit_searchRoute->setPlaceholderText("Например: A123BC116");
    ui->lineEdit_searchRoute->setValidator(new QRegularExpressionValidator(regExp, this));

    ui->lineEditPlate->setPlaceholderText("Например: A123BC116");
    ui->lineEditPlate->setValidator(new QRegularExpressionValidator(regExp, this));

    QStringList validRegions = {
        "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24",
        "25", "26", "27", "28", "29", "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "40", "41", "42", "43", "44", "45", "46", "47", "48",
        "49", "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "70", "71", "72",
        "73", "74", "75", "76", "77", "78", "79", "80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "90", "91", "92", "93", "94", "95", "96",
        "97", "98", "99", "102", "113", "116", "121", "123", "124", "125", "134", "136", "138", "142", "150", "152", "154", "159", "161", "163", "164",
        "173", "174", "177", "178", "186", "190", "196", "197", "199", "716", "750", "761", "763", "777", "790", "799", "890", "999"
    };
    ui->tableRoutes->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableVehicles->setEditTriggers(QAbstractItemView::NoEditTriggers);

//    connect(ui->lineEdit_search, &QLineEdit::textChanged, this, [=](const QString &text){
//        QRegularExpressionMatch match = regExp.match(text);
//        if (match.hasMatch()) {
//            QString regionCode = match.captured(3); // Группа с регионом
//            if (validRegions.contains(regionCode)) {
//                ui->lineEdit_search->setStyleSheet(""); // валидный стиль
//            } else {
//                ui->lineEdit_search->setStyleSheet("background-color: #ffcccc"); // подсветка ошибки
//            }
//        } else {
//            ui->lineEdit_search->setStyleSheet("background-color: #ffcccc");
//        }
//    });
}

MainWindow::~MainWindow()
{

    if (mapView) {
        // Очищаем содержимое WebEngine
        mapView->setUrl(QUrl()); // или mapView->setHtml("");
        //mapView->setHtml("");
        // Удаляем объект
        mapView->deleteLater();
        delete mapView;
        mapView = nullptr;
    }
    if (routeView) {
        // Очищаем содержимое WebEngine
        routeView->setUrl(QUrl()); // или routeView->setHtml("");

        // Удаляем объект
        delete routeView;
        routeView = nullptr;
    }

    delete ui;
}

void MainWindow::updateMap(const QString& plateNumber) {
    QSqlQuery query;
    query.prepare("SELECT plate_number, latitude, longitude, timestamp FROM truck_positions WHERE plate_number = :plate ORDER BY timestamp DESC LIMIT 1");
    query.bindValue(":plate", plateNumber);

    if (!query.exec() || !query.next()) {
        qDebug() << "SQL error or no data:" << query.lastError().text();
        return;
    }

    QString id = query.value(0).toString();
    double lat = query.value(1).toDouble();
    double lon = query.value(2).toDouble();
    QDateTime timestamp = QDateTime::fromString(query.value(3).toString(), Qt::ISODate);

    // Обновляем карту
    QString js = "clearMarkers();\n";
    js += QString("addOrUpdateMarker('%1', %2, %3);\n").arg(id).arg(lat).arg(lon);
    if (mapView && mapReady && mapView->page()) {
        mapView->page()->runJavaScript(js);
    }

    // Отображаем данные в textEdit_info
    ui->textEdit_info->show();



    QString formattedTime = timestamp.toString("yyyy-MM-dd HH:mm:ss");

    ui->textEdit_info->clear();
    ui->textEdit_info->append("Номер: " + id);
    ui->textEdit_info->append("Время: " + formattedTime);




    // Обратное геокодирование (по координатам получаем адрес)
    QString url = QString("https://nominatim.openstreetmap.org/reverse?format=json&lat=%1&lon=%2&zoom=18&addressdetails=1")
                  .arg(lat).arg(lon);

    QNetworkRequest request((QUrl(url)));
    request.setRawHeader("User-Agent", "Qt Application");

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkReply* reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj = doc.object();
            QString address = obj["display_name"].toString();
            ui->textEdit_info->append("Адрес: " + address);
        } else {
            ui->textEdit_info->append("Адрес: [не удалось получить]");
        }
        reply->deleteLater();
        manager->deleteLater();
    });
}

void MainWindow::on_btnList_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    loadVehiclesTable();
}

void MainWindow::on_lineEdit_search_textChanged()
{
    loadVehiclesTable();
}

void MainWindow::loadVehiclesTable()
{
    QString searchText = ui->lineEdit_search->text();
    QString queryStr = QString(
        "SELECT id, plate_number, driver_name, status "
        "FROM trucks "
        "WHERE plate_number LIKE '%1%%'"
    ).arg(searchText);
    QSqlQuery query;
    if (!query.exec(queryStr)) {
        qDebug() << "Ошибка запроса:" << query.lastError().text();
        return;
    }

    ui->tableVehicles->clear();
    ui->tableVehicles->setRowCount(0);
    ui->tableVehicles->setColumnCount(5);
    QStringList headers = {"Гос номер", "ФИО водителя", "Статус", "Отслеживание", "Создать маршрут"};
    ui->tableVehicles->setHorizontalHeaderLabels(headers);
    //ui->tableVehicles->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    int row = 0;
    while (query.next()) {
        ui->tableVehicles->insertRow(row);

        // Столбцы: Гос номер, ФИО, Статус
        ui->tableVehicles->setItem(row, 0, new QTableWidgetItem(query.value("plate_number").toString()));
        ui->tableVehicles->setItem(row, 1, new QTableWidgetItem(query.value("driver_name").toString()));
        ui->tableVehicles->setItem(row, 2, new QTableWidgetItem(query.value("status").toString()));

        // Кнопка "Отслеживание"
        QPushButton* btnTrack = new QPushButton("Отследить");
        btnTrack->setProperty("vehicle_plateNumber", query.value("plate_number"));  // Передаём ID как свойство

        connect(btnTrack, &QPushButton::clicked, this, &MainWindow::onTrackVehicleClicked);

        ui->tableVehicles->setCellWidget(row, 3, btnTrack);

        // Кнопка "Создать маршрут"
        QPushButton* btnCreateRoute = new QPushButton("Создать маршрут");
        btnCreateRoute->setProperty("vehicle_plateNumber", query.value("plate_number"));  // Передаём госномер как свойство

        connect(btnCreateRoute, &QPushButton::clicked, this, &MainWindow::onCreateRouteClicked);

        ui->tableVehicles->setCellWidget(row, 4, btnCreateRoute);  // Кнопка в последний столбец
        if (query.value("status").toString() == "on_route"){
            btnCreateRoute->setEnabled(false);
        }
        else {
            btnCreateRoute->setEnabled(true);
        }
        row++;
    }

    ui->tableVehicles->resizeColumnsToContents();
    auto header = ui->tableVehicles->horizontalHeader();

    // Первые 3 столбца — растягиваем
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::Stretch);

    // Последний столбец — фиксированной ширины
    header->setSectionResizeMode(3, QHeaderView::Fixed);
    ui->tableVehicles->setColumnWidth(3, 120);  // Задаём ширину в пикселях
    header->setSectionResizeMode(4, QHeaderView::Fixed);
    ui->tableVehicles->setColumnWidth(4, 200);  // Задаём ширину в пикселях
}

void MainWindow::onCreateRouteClicked()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    QString vehiclePlateNumber = btn->property("vehicle_plateNumber").toString();

    // Открываем форму для ввода данных о маршруте
    ui->stackedWidget->setCurrentWidget(ui->pageCreateRoute);

    // Загружаем госномер выбранного автомобиля
    QString vehiclePlate = "Создание маршрута для:" + vehiclePlateNumber;
    ui->label_CR_plateNumber->setText(vehiclePlate);
    ui->label_hideNumber->setText(vehiclePlateNumber);
}


void MainWindow::onTrackVehicleClicked()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    currentTrackedPlateNumber = btn->property("vehicle_plateNumber").toString();
    trackVehicle(currentTrackedPlateNumber);


    updateMap(currentTrackedPlateNumber);
    disconnect(updateTimer, nullptr, nullptr, nullptr);
    connect(updateTimer, &QTimer::timeout, this, [this]() {
        updateMap(currentTrackedPlateNumber);
    });

    if (!updateTimer->isActive()) {
        updateTimer->start(10000);
    }
}

void MainWindow::onStopClicked() {
    if (updateTimer->isActive()) {
        updateTimer->stop();
        ui->textEdit_info->hide();
        ui->textEdit_info->clear();
    }
}
void MainWindow::on_btnAddVehicle_clicked()
{
    QString plate = ui->lineEditPlate->text().trimmed();
    QString driver = ui->lineEditDriver->text().trimmed();
    //QString status = ui->lineEditStatus->text().trimmed();

    if (plate.isEmpty() || driver.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните все поля.");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO trucks (plate_number, driver_name) VALUES (:plate, :driver)");
    query.bindValue(":plate", plate);
    query.bindValue(":driver", driver);
    //query.bindValue(":status", status);

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка добавления", query.lastError().text());
        return;
    }

    QMessageBox::information(this, "Успех", "Авто успешно добавлено!");

    // Очистка полей
    ui->lineEditPlate->clear();
    ui->lineEditDriver->clear();
    //ui->lineEditStatus->clear();
}

void MainWindow::on_btnAdd_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_btnBackToList_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageList);
    loadVehiclesTable();

}

// Вывод карты
void MainWindow::setupMap(double latitude, double longitude)
{
    QString html = QString(R"(
    <!DOCTYPE html>
    <html>
    <head>
        <meta charset="utf-8" />
        <title>Truck Tracker</title>
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <style>
            html, body, #map { height: 100%; margin: 0; padding: 0; }
        </style>
        <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css" />
        <script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js"></script>
    </head>
    <body>
        <div id="map"></div>
        <script>
            var map = L.map('map', { attributionControl: false }).setView([%1, %2], 13);

            L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
                maxZoom: 19
            }).addTo(map);

            var markers = {};

            // Машина-иконка (можно заменить на свою)
            var carIcon = L.icon({
                iconUrl: 'https://cdn-icons-png.flaticon.com/512/5465/5465675.png',
                iconSize: [32, 32],
                iconAnchor: [16, 16],
                popupAnchor: [0, -16]
            });

            function addOrUpdateMarker(id, lat, lon) {
                if (markers[id]) {
                    markers[id].setLatLng([lat, lon]);
                } else {
                    markers[id] = L.marker([lat, lon], { icon: carIcon }).addTo(map).bindPopup(id);
                }
            }

            function clearMarkers() {
                for (var id in markers) {
                    map.removeLayer(markers[id]);
                }
                markers = {};
            }

            // Добавляем начальную метку
            addOrUpdateMarker("Машина", %1, %2);
        </script>
    </body>
    </html>
    )").arg(latitude).arg(longitude);

    mapView->setHtml(html);
    connect(mapView, &QWebEngineView::loadFinished, this, [=](bool ok) {
        if (ok) {
            mapReady = true;
        }
    });
}

void MainWindow::trackVehicle(const QString& licensePlate)
{
    QSqlQuery query;
    query.prepare(R"(
        SELECT latitude, longitude
        FROM truck_positions
        WHERE plate_number = :plate
        ORDER BY timestamp DESC
        LIMIT 1
    )");
    query.bindValue(":plate", licensePlate);

    if (query.exec() && query.next()) {
        double lat = query.value(0).toDouble();
        double lon = query.value(1).toDouble();

        setupMap(lat, lon);
        ui->stackedWidget->setCurrentWidget(ui->pageTracking);
    } else {
        QMessageBox::warning(this, "Ошибка", "Положение автомобиля не найдено.");
        ui->stackedWidget->setCurrentWidget(ui->pageList);
    }
}

void MainWindow::on_btnSaveRoute_clicked()
{
    QString vehiclePlate = ui->label_hideNumber->text();
//    QString vehiclePlate = "Создание маршрута для:" +
//    ui->label_CR_plateNumber->setText("");
    QString routeName = ui->lineEditRouteName->text();
    QDate routeDate = ui->dateEditRouteDate->date();

    if (routeName.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните все поля.");
        return;
    }

    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO routes (plate_number, route_date, route_name, start_time)
        VALUES (:plate, :route_date, :route_name, NOW())
    )");
    query.bindValue(":plate", vehiclePlate);
    query.bindValue(":route_date", routeDate.toString("yyyy-MM-dd"));
    query.bindValue(":route_name", routeName);

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать маршрут.\n" + query.lastError().text());
        return;
    }
    QSqlQuery updateTruckStatus;
    updateTruckStatus.prepare("UPDATE trucks SET status = 'on_route' WHERE plate_number = :plate");
    updateTruckStatus.bindValue(":plate", vehiclePlate);
    if (!updateTruckStatus.exec()) {
        qDebug() << "Ошибка при установке статуса on_route:" << updateTruckStatus.lastError().text();
    }
    QMessageBox::information(this, "Успех", "Маршрут успешно создан.");

    // Очистить форму после сохранения
    //ui->comboBoxVehicle->setCurrentIndex(0);

    ui->label_hideNumber->clear();
    ui->lineEditRouteName->clear();
    ui->dateEditRouteDate->setDate(QDate::currentDate());

    // Переключаемся обратно на страницу списка
    ui->stackedWidget->setCurrentWidget(ui->pageList);
}

void MainWindow::on_btnRoutes_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageRoutesList);
    loadRoutesTable();
}

void MainWindow::loadRoutesTable()
{
    QString searchText = ui->lineEdit_searchRoute->text();
    QString queryStr = QString(
        "SELECT id, plate_number, route_date, route_name, status "
        "FROM routes "
        "WHERE plate_number LIKE '%1%%' "
        "ORDER BY status ASC, id ASC;"
    ).arg(searchText);
    QSqlQuery query;
        if (!query.exec(queryStr)) {
            qDebug() << "Ошибка запроса:" << query.lastError().text();
            return;
        }

        ui->tableRoutes->clear();
        ui->tableRoutes->setRowCount(0);
        ui->tableRoutes->setColumnCount(7); // Добавляем 7 столбца
        QStringList headers = {"ID маршрута", "Гос номер", "Дата маршрута", "Название маршрута", "Статус", "Показать маршрут", "Завершить Маршрут"};
        ui->tableRoutes->setHorizontalHeaderLabels(headers);

        QString statusout;


        int row = 0;
        while (query.next()) {
            ui->tableRoutes->insertRow(row);
            QString status = query.value("status").toString();
            if (status == "active") {
                statusout = "Активен";
            }
            else {
                statusout = "Завершен";
            }
            ui->tableRoutes->setItem(row, 0, new QTableWidgetItem(query.value("id").toString()));
            ui->tableRoutes->setItem(row, 1, new QTableWidgetItem(query.value("plate_number").toString()));
            ui->tableRoutes->setItem(row, 2, new QTableWidgetItem(query.value("route_date").toString()));
            ui->tableRoutes->setItem(row, 3, new QTableWidgetItem(query.value("route_name").toString()));
            ui->tableRoutes->setItem(row, 4, new QTableWidgetItem(statusout));
            // Кнопка "Отслеживание"
            QPushButton* btnShowRoute = new QPushButton("Показать");
            btnShowRoute->setProperty("route_id", query.value("id"));  // Передаём ID как свойство
            btnShowRoute->setProperty("value_plate_number", query.value("plate_number"));  // Передаём plate_number как свойство
            connect(btnShowRoute, &QPushButton::clicked, this, &MainWindow::onShowRouteClicked);

            ui->tableRoutes->setCellWidget(row, 5, btnShowRoute);

            QPushButton* btnEndRoute = new QPushButton("Завершить");
            btnEndRoute->setProperty("value_plate_number", query.value("plate_number"));  // Передаём plate_number как свойство

            connect(btnEndRoute, &QPushButton::clicked, this, &MainWindow::onStopRouteClicked);

            ui->tableRoutes->setCellWidget(row, 6, btnEndRoute);
            if (status == "active") {
                btnShowRoute->setEnabled(false);
                btnEndRoute->setEnabled(true);
            }
            else {
                btnShowRoute->setEnabled(true);
                btnEndRoute->setEnabled(false);
            }

            row++;
        }

        ui->tableRoutes->resizeColumnsToContents();
        auto header = ui->tableRoutes->horizontalHeader();

        header->setSectionResizeMode(0, QHeaderView::Stretch);
        header->setSectionResizeMode(1, QHeaderView::Stretch);
        header->setSectionResizeMode(2, QHeaderView::Stretch);
        header->setSectionResizeMode(3, QHeaderView::Stretch);
        header->setSectionResizeMode(4, QHeaderView::Stretch);

        header->setSectionResizeMode(5, QHeaderView::Fixed);
        ui->tableVehicles->setColumnWidth(5, 120);  // Задаём ширину в пикселях
        header->setSectionResizeMode(6, QHeaderView::Fixed);
        ui->tableVehicles->setColumnWidth(5, 120);  // Задаём ширину в пикселях

}
void MainWindow::onStopRouteClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    QString plateNumber = button->property("value_plate_number").toString();

    // Найдём активный маршрут по номеру машины
    QSqlQuery routeQuery;
    routeQuery.prepare("SELECT id, start_time FROM routes WHERE plate_number = :plate AND status = 'active'");
    routeQuery.bindValue(":plate", plateNumber);
    if (!routeQuery.exec() || !routeQuery.next()) {
        qDebug() << "Активный маршрут не найден для" << plateNumber;
        return;
    }

    int routeId = routeQuery.value("id").toInt();
    QDateTime startTime = routeQuery.value("start_time").toDateTime();

    // Первая точка после начала
    QSqlQuery firstPointQuery;
    firstPointQuery.prepare(R"(
        SELECT latitude, longitude, timestamp
        FROM route_points
        WHERE plate_number = :plate AND timestamp >= :start
        ORDER BY timestamp ASC
        LIMIT 1
    )");
    firstPointQuery.bindValue(":plate", plateNumber);
    firstPointQuery.bindValue(":start", startTime);
    if (!firstPointQuery.exec() || !firstPointQuery.next()) {
        qDebug() << "Начальная точка маршрута не найдена";
        return;
    }
    double startLat = firstPointQuery.value("latitude").toDouble();
    double startLon = firstPointQuery.value("longitude").toDouble();

    // Последняя точка до текущего времени
    QDateTime now = QDateTime::currentDateTime();
    QSqlQuery lastPointQuery;
    lastPointQuery.prepare(R"(
        SELECT latitude, longitude, timestamp
        FROM route_points
        WHERE plate_number = :plate AND timestamp <= :now
        ORDER BY timestamp DESC
        LIMIT 1
    )");
    lastPointQuery.bindValue(":plate", plateNumber);
    lastPointQuery.bindValue(":now", now);
    if (!lastPointQuery.exec() || !lastPointQuery.next()) {
        qDebug() << "Конечная точка маршрута не найдена";
        return;
    }
    double endLat = lastPointQuery.value("latitude").toDouble();
    double endLon = lastPointQuery.value("longitude").toDouble();
    QDateTime endTime = lastPointQuery.value("timestamp").toDateTime();

    // Обновим маршрут
    QSqlQuery updateQuery;
    updateQuery.prepare(R"(
        UPDATE routes SET
            status = 'completed',
            start_latitude = :startLat,
            start_longitude = :startLon,
            end_latitude = :endLat,
            end_longitude = :endLon,
            end_time = :endTime
        WHERE id = :id
    )");
    updateQuery.bindValue(":startLat", startLat);
    updateQuery.bindValue(":startLon", startLon);
    updateQuery.bindValue(":endLat", endLat);
    updateQuery.bindValue(":endLon", endLon);
    updateQuery.bindValue(":endTime", endTime);
    updateQuery.bindValue(":id", routeId);

    if (!updateQuery.exec()) {
        qDebug() << "Ошибка при обновлении маршрута:" << updateQuery.lastError().text();
    } else {
        button->setEnabled(false); // делаем кнопку некликабельной
        qDebug() << "Маршрут завершён успешно для" << plateNumber;

    }
    QSqlQuery updateTruckStatus;
    updateTruckStatus.prepare("UPDATE trucks SET status = 'waiting' WHERE plate_number = :plate");
    updateTruckStatus.bindValue(":plate", plateNumber);
    if (!updateTruckStatus.exec()) {
        qDebug() << "Ошибка при установке статуса waiting:" << updateTruckStatus.lastError().text();
    }
    loadRoutesTable();
}
void MainWindow::onShowRouteClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    int routeId = button->property("route_id").toInt();
    QString plateNumber = button->property("value_plate_number").toString();

    // Получаем время начала и конца маршрута
    QSqlQuery routeQuery;
    routeQuery.prepare("SELECT start_time, end_time FROM routes WHERE id = :id");
    routeQuery.bindValue(":id", routeId);
    if (!routeQuery.exec() || !routeQuery.next()) {
        qDebug() << "Не удалось найти маршрут по ID" << routeId;
        return;
    }

    QDateTime startTime = routeQuery.value("start_time").toDateTime();
    QDateTime endTime = routeQuery.value("end_time").toDateTime();

    // Получаем точки маршрута
    QSqlQuery pointsQuery;
    pointsQuery.prepare(R"(
        SELECT latitude, longitude
        FROM route_points
        WHERE plate_number = :plate
        AND timestamp >= :start AND timestamp <= :end
        ORDER BY timestamp ASC
    )");
    pointsQuery.bindValue(":plate", plateNumber);
    pointsQuery.bindValue(":start", startTime);
    pointsQuery.bindValue(":end", endTime);

    QVector<QPair<double, double>> routePoints;

    if (pointsQuery.exec()) {
        while (pointsQuery.next()) {
            double lat = pointsQuery.value("latitude").toDouble();
            double lon = pointsQuery.value("longitude").toDouble();
            routePoints.append(qMakePair(lat, lon));
        }
    } else {
        qDebug() << "Ошибка при получении точек маршрута:" << pointsQuery.lastError().text();
        return;
    }

    if (routePoints.isEmpty()) {
        qDebug() << "Точки маршрута не найдены";
        return;
    }

    // Переключаемся на экран с картой и показываем маршрут
    showMapWithRoute(routePoints);
}

void MainWindow::showMapWithRoute(const QVector<QPair<double, double>>& routePoints)
{
    // Переключаем на экран с картой
    ui->stackedWidget->setCurrentWidget(ui->pageRouteTracking);

    // Если routeView еще не создан, создаем его
//    if (!routeView) {
//        routeView = new QWebEngineView(this);
//        QVBoxLayout *layout = new QVBoxLayout(ui->mapPlaceholder_route);
//        layout->setContentsMargins(0, 0, 0, 0);
//        layout->addWidget(routeView);
//    }

    // Преобразуем координаты маршрута в формат для JavaScript
    QString routePointsStr = "[";
    for (int i = 0; i < routePoints.size(); ++i) {
        const auto& point = routePoints[i];
        routePointsStr += QString("[%1, %2]").arg(point.first).arg(point.second);
        if (i != routePoints.size() - 1) {
            routePointsStr += ", ";
        }
    }
    routePointsStr += "]";

    // Находим среднюю точку маршрута (для центровки карты)
    double totalLat = 0.0, totalLon = 0.0;
    for (const auto& point : routePoints) {
        totalLat += point.first;
        totalLon += point.second;
    }
    double centerLat = totalLat / routePoints.size();
    double centerLon = totalLon / routePoints.size();

    // Формируем HTML с JavaScript для отображения карты и маршрута
    QString html = QString(R"(
    <!DOCTYPE html>
    <html>
    <head>
        <meta charset="utf-8" />
        <title>Маршрут</title>
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <style>
            html, body, #map { height: 100%; margin: 0; }
        </style>
        <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css" />
        <script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js"></script>
    </head>
    <body>
        <div id="map"></div>
        <script>
            var map = L.map('map', { attributionControl: false }).setView([%1, %2], 13);
            L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
                maxZoom: 19,
                attribution: '© OpenStreetMap'
            }).addTo(map);

            var route = %3;

            // Отображаем маршрут
            var latlngs = route.map(function(coord) {
                return [coord[0], coord[1]];
            });
            var polyline = L.polyline(latlngs, {color: 'blue'}).addTo(map);

            // Устанавливаем масштаб карты так, чтобы маршрут был видим
            map.fitBounds(polyline.getBounds());

            // Добавляем маркер старта
            var startMarker = L.marker([route[0][0], route[0][1]]).addTo(map)
                .bindPopup("Старт");

            // Добавляем маркер финиша
            var finishMarker = L.marker([route[route.length - 1][0], route[route.length - 1][1]]).addTo(map)
                .bindPopup("Финиш");
        </script>
    </body>
    </html>
    )").arg(centerLat).arg(centerLon).arg(routePointsStr);

    // Отправляем HTML код в QWebEngineView
    routeView->setHtml(html);

    // Подключаем сигнал для отслеживания завершения загрузки страницы
    connect(routeView, &QWebEngineView::loadFinished, this, [=](bool ok) {
        if (ok) {
            qDebug() << "Map loaded successfully";
        } else {
            qDebug() << "Failed to load map";
        }
    });

}



void MainWindow::on_lineEdit_searchRoute_textChanged()
{
   loadRoutesTable();
}

void MainWindow::on_btnBackToList_2_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageRoutesList);
}
