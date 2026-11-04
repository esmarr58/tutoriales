#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
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
    void aplicarAjustes();

    QLabel *etiquetaImagen;   // muestra la imagen escalada
    QPushButton *botonDibujar;
    QCheckBox *casillaGris;
    QLabel *etiquetaContraste;
    QSlider *deslizadorContraste;   // alpha = valor / 10
    QLabel *etiquetaBrillo;
    QSlider *deslizadorBrillo;      // beta = valor
    QLabel *muestraColor;     // cuadro pintado con el color del píxel
    QLabel *etiquetaPixel;    // (x, y)  B G R
    QLabel *etiquetaEstado;   // dice qué pasó: archivo, ancho × alto · canales
    cv::Mat imagen;           // la imagen cargada (3 canales, BGR)
    cv::Mat resultado;        // lo que se ve: gris o color, con brillo y contraste
};
#endif // MAINWINDOW_H
