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
    QDir().mkpath(carpeta);
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

void MainWindow::on_botonGuardar_clicked()
{
    const QString nombre = ui->entradaNombre->text().trimmed();
    if (nombre.isEmpty()) {
        ui->etiquetaEstado->setText("Escribe el nombre del producto.");
        return;
    }

    QSqlQuery consulta;
    consulta.prepare("INSERT INTO productos (nombre, precio, existencia) "
                     "VALUES (:nombre, :precio, :existencia)");
    consulta.bindValue(":nombre", nombre);
    consulta.bindValue(":precio", ui->spinPrecio->value());
    consulta.bindValue(":existencia", ui->spinExistencia->value());

    if (consulta.exec()) {
        ui->etiquetaEstado->setText("Guardado con id " + consulta.lastInsertId().toString() + ".");
        ui->entradaNombre->clear();
        on_botonComprobar_clicked();
        on_botonCargar_clicked();
    } else {
        ui->etiquetaEstado->setText("No se guardó: " + consulta.lastError().text());
    }
}

void MainWindow::on_botonCargar_clicked()
{
    QSqlQuery consulta("SELECT id, nombre, precio, existencia FROM productos ORDER BY id");
    ui->tablaProductos->setRowCount(0);

    int fila = 0;
    while (consulta.next()) {
        ui->tablaProductos->insertRow(fila);
        for (int columna = 0; columna < 4; ++columna) {
            ui->tablaProductos->setItem(fila, columna,
                new QTableWidgetItem(consulta.value(columna).toString()));
        }
        ++fila;
    }
    ui->tablaProductos->resizeColumnsToContents();
    ui->etiquetaEstado->setText(QString("Productos cargados: %1.").arg(fila));
}

void MainWindow::on_tablaProductos_cellClicked(int row, int column)
{
    Q_UNUSED(column);
    idSeleccionado = ui->tablaProductos->item(row, 0)->text().toInt();
    ui->etiquetaIdValor->setText(QString::number(idSeleccionado));
    ui->entradaNombre->setText(ui->tablaProductos->item(row, 1)->text());
    ui->spinPrecio->setValue(ui->tablaProductos->item(row, 2)->text().toDouble());
    ui->spinExistencia->setValue(ui->tablaProductos->item(row, 3)->text().toInt());
}

void MainWindow::on_botonActualizar_clicked()
{
    if (idSeleccionado < 0) {
        ui->etiquetaEstado->setText("Primero toca un producto de la tabla.");
        return;
    }
    const QString nombre = ui->entradaNombre->text().trimmed();
    if (nombre.isEmpty()) {
        ui->etiquetaEstado->setText("Escribe el nombre del producto.");
        return;
    }

    QSqlQuery consulta;
    consulta.prepare("UPDATE productos SET nombre = :nombre, precio = :precio, "
                     "existencia = :existencia WHERE id = :id");
    consulta.bindValue(":nombre", nombre);
    consulta.bindValue(":precio", ui->spinPrecio->value());
    consulta.bindValue(":existencia", ui->spinExistencia->value());
    consulta.bindValue(":id", idSeleccionado);

    if (consulta.exec()) {
        on_botonCargar_clicked();
        ui->etiquetaEstado->setText(QString("Filas actualizadas: %1.")
                                    .arg(consulta.numRowsAffected()));
    } else {
        ui->etiquetaEstado->setText("No se actualizó: " + consulta.lastError().text());
    }
}
