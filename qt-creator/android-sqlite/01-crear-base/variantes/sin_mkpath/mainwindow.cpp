#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDir>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    const QString carpeta = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString ruta = carpeta + "/inventario.db";
    const bool yaExistia = QFile::exists(ruta);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(ruta);
    if (!db.open()) {
        ui->etiquetaEstado->setText("No se abrió la base: " + db.lastError().text());
        return;
    }

    QSqlQuery consulta;
    if (!consulta.exec("CREATE TABLE IF NOT EXISTS productos ("
                       "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                       "nombre TEXT NOT NULL, "
                       "precio REAL, "
                       "existencia INTEGER)")) {
        ui->etiquetaEstado->setText("No se creó la tabla: " + consulta.lastError().text());
        return;
    }

    ui->etiquetaEstado->setText((yaExistia ? "Base encontrada:\n" : "Base nueva creada:\n") + ruta);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_botonComprobar_clicked()
{
    QSqlQuery consulta("SELECT COUNT(*) FROM productos");
    if (consulta.next()) {
        ui->etiquetaConteo->setText("Productos en la tabla: " + consulta.value(0).toString());
    } else {
        ui->etiquetaConteo->setText("Error: " + consulta.lastError().text());
    }
}
