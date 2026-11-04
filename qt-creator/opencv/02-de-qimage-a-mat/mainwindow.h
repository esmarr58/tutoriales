#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <opencv2/core.hpp>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *evento) override;

private:
    void construirInterfaz();
    void cargarMuestra(const QString &ruta);
    void mostrarImagen();

    QLabel *etiquetaImagen;   // muestra la imagen escalada
    QLabel *etiquetaEstado;   // dice qué pasó: archivo, ancho × alto · canales
    cv::Mat imagen;           // la imagen en formato de OpenCV
};
#endif // MAINWINDOW_H
