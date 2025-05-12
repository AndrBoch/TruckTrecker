#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QDebug>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:


    void on_btnList_clicked();

    void loadVehiclesTable();

    void onTrackVehicleClicked();

    void on_btnAddVehicle_clicked();

    void on_btnAdd_clicked();

    void on_btnBackToList_clicked();

    void setupMap(double latitude, double longitude);

    void trackVehicle(const QString& licensePlate);

    void updateMap();

    void onCreateRouteClicked();

    void on_btnSaveRoute_clicked();

    void on_btnRoutes_clicked();

    void loadRoutesTable();

    void onShowRouteClicked();

    void onStopRouteClicked();

    void showMapWithRoute(const QVector<QPair<double, double>>& routePoints);

    void on_lineEdit_search_textChanged();

    void on_lineEdit_searchRoute_textChanged();

    void on_btnBackToList_2_clicked();

private:
    Ui::MainWindow *ui;

    QWebEngineView *mapView = nullptr;
    QWebEngineView *routeView = nullptr;

    bool mapReady = false;
};
#endif // MAINWINDOW_H
