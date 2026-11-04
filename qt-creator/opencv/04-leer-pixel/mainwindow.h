#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QPushButton>
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
    bool eventFilter(QObject *objeto, QEvent *evento) override;

private:
    void construirInterfaz();
    void cargarMuestra(const QString &ruta);
    void mostrarImagen();
    void dibujarConOpenCV();
    void leerPixel(const QPoint &punto);

    QLabel *etiquetaImagen;   // muestra la imagen escalada
    QPushButton *botonDibujar;
    QLabel *muestraColor;     // cuadro pintado con el color del píxel
    QLabel *etiquetaPixel;    // (x, y)  B G R
    QLabel *etiquetaEstado;   // dice qué pasó: archivo, ancho × alto · canales
    cv::Mat imagen;           // la imagen en formato de OpenCV
};
#endif // MAINWINDOW_H
