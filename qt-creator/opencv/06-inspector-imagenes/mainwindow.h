#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QBoxLayout>
#include <QButtonGroup>
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
    void restablecer();
    void acomodar();
    void leerPixel(const QPoint &punto);
    void aplicarAjustes();
    void mostrarPixel();

    QLabel *etiquetaImagen;   // muestra la imagen escalada
    QBoxLayout *capas;        // vertical en el teléfono, horizontal en la ventana ancha
    QWidget *panel;           // todos los controles
    QButtonGroup *grupoMuestras;   // tres botones; solo uno queda marcado
    QPushButton *botonRestablecer;
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
    cv::Point pixelElegido{-1, -1};   // último píxel tocado (-1: ninguno)
    // Muestras de los recursos, en el mismo orden que los botones.
    const QStringList rutas = {":/muestras/formas.png", ":/muestras/colores_bgr.png",
                               ":/muestras/celulas.png"};
};
#endif // MAINWINDOW_H
