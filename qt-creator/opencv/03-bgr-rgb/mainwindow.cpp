#include "mainwindow.h"
#include "puente.h"

#include <QFileInfo>
#include <QPixmap>
#include <QVBoxLayout>
#include <opencv2/imgproc.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    construirInterfaz();
    cargarMuestra(":/muestras/colores_bgr.png");
}

MainWindow::~MainWindow() {}

void MainWindow::construirInterfaz()
{
    setWindowTitle("Visor OpenCV");
    auto *central = new QWidget(this);
    // Letra de 20 pt: la heredan todos los widgets y se lee en el teléfono.
    QFont letra = central->font();
    letra.setPointSize(20);
    central->setFont(letra);

    etiquetaImagen = new QLabel;
    etiquetaImagen->setAlignment(Qt::AlignCenter);
    etiquetaImagen->setMinimumSize(200, 150);
    // Ignored: la etiqueta usa el espacio que le da el layout, no el tamaño de la imagen.
    etiquetaImagen->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    botonDibujar = new QPushButton("Dibujar con OpenCV");
    botonDibujar->setMinimumHeight(64);   // grande para el dedo

    etiquetaEstado = new QLabel;
    etiquetaEstado->setAlignment(Qt::AlignCenter);
    etiquetaEstado->setWordWrap(true);

    // Imagen arriba (se estira: factor 1) y el texto debajo.
    auto *capas = new QVBoxLayout(central);
    capas->addWidget(etiquetaImagen, 1);
    capas->addWidget(botonDibujar);
    capas->addWidget(etiquetaEstado);
    setCentralWidget(central);

    // Señal clicked del botón -> nuestra función (puntero a función miembro).
    connect(botonDibujar, &QPushButton::clicked, this, &MainWindow::dibujarConOpenCV);
    resize(900, 800);
}

void MainWindow::cargarMuestra(const QString &ruta)
{
    // cv::imread no lee rutas ":/" de recursos: cargamos con QImage y usamos el puente.
    QImage original(ruta);
    if (original.isNull()) {
        etiquetaEstado->setText("No se pudo abrir " + ruta);
        return;
    }
    imagen = aMat(original);
    etiquetaEstado->setText(QString("%1 · %2 × %3 · %4 canales")
                                .arg(QFileInfo(ruta).fileName())
                                .arg(imagen.cols)
                                .arg(imagen.rows)
                                .arg(imagen.channels()));
    mostrarImagen();
}

void MainWindow::mostrarImagen()
{
    if (imagen.empty())
        return;
    // En el teléfono cada punto lógico son varios píxeles: escalamos a píxeles reales
    // para que la imagen se vea nítida.
    const qreal densidad = devicePixelRatioF();
    QPixmap pixmap = QPixmap::fromImage(aQImage(imagen));
    pixmap = pixmap.scaled(etiquetaImagen->size() * densidad, Qt::KeepAspectRatio,
                           Qt::SmoothTransformation);
    pixmap.setDevicePixelRatio(densidad);
    etiquetaImagen->setPixmap(pixmap);
}

void MainWindow::dibujarConOpenCV()
{
    if (imagen.empty())
        return;
    // Para OpenCV un color es (B, G, R): (0, 0, 255) es rojo puro.
    const cv::Scalar rojo(0, 0, 255);
    // Anillo de 12 px alrededor del círculo rojo de colores_bgr.png (centro 170, 130).
    cv::circle(imagen, cv::Point(170, 130), 105, rojo, 12);
    etiquetaEstado->setText("Anillo de OpenCV con Scalar(0, 0, 255): rojo en BGR");
    mostrarImagen();
}

void MainWindow::resizeEvent(QResizeEvent *evento)
{
    QMainWindow::resizeEvent(evento);
    mostrarImagen();   // la ventana cambió de tamaño (o el teléfono giró)
}
