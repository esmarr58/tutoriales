#include "mainwindow.h"
#include "puente.h"

#include <QFileInfo>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPixmap>
#include <QVBoxLayout>
#include <cmath>
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
    // Queremos enterarnos de los clics (y toques) sobre la imagen.
    etiquetaImagen->installEventFilter(this);

    botonDibujar = new QPushButton("Dibujar con OpenCV");
    botonDibujar->setMinimumHeight(64);   // grande para el dedo

    muestraColor = new QLabel;
    muestraColor->setFixedSize(64, 64);
    muestraColor->setStyleSheet("border: 2px solid gray;");

    etiquetaPixel = new QLabel("Toca la imagen para leer un píxel");
    etiquetaPixel->setWordWrap(true);

    etiquetaEstado = new QLabel;
    etiquetaEstado->setAlignment(Qt::AlignCenter);
    etiquetaEstado->setWordWrap(true);

    // Fila del píxel: cuadro de color a la izquierda y el texto a la derecha.
    auto *filaPixel = new QHBoxLayout;
    filaPixel->addWidget(muestraColor);
    filaPixel->addWidget(etiquetaPixel, 1);

    // Imagen arriba (se estira: factor 1) y el texto debajo.
    auto *capas = new QVBoxLayout(central);
    capas->addWidget(etiquetaImagen, 1);
    capas->addLayout(filaPixel);
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

bool MainWindow::eventFilter(QObject *objeto, QEvent *evento)
{
    // En Android el toque llega a la QLabel como un clic del ratón.
    if (objeto == etiquetaImagen && evento->type() == QEvent::MouseButtonPress) {
        auto *clic = static_cast<QMouseEvent *>(evento);
        leerPixel(clic->position().toPoint());
        return true;   // ya lo atendimos
    }
    return QMainWindow::eventFilter(objeto, evento);
}

void MainWindow::leerPixel(const QPoint &punto)
{
    if (imagen.empty())
        return;
    // La imagen se ve escalada (sin deformar) y centrada dentro de la etiqueta.
    const double escala = std::min(double(etiquetaImagen->width()) / imagen.cols,
                                   double(etiquetaImagen->height()) / imagen.rows);
    const double margenX = (etiquetaImagen->width() - imagen.cols * escala) / 2;
    const double margenY = (etiquetaImagen->height() - imagen.rows * escala) / 2;

    // De coordenadas de la etiqueta a coordenadas de la imagen original.
    const int x = int(std::floor((punto.x() - margenX) / escala));
    const int y = int(std::floor((punto.y() - margenY) / escala));
    if (x < 0 || y < 0 || x >= imagen.cols || y >= imagen.rows) {
        etiquetaPixel->setText("Fuera de la imagen");
        muestraColor->setStyleSheet("border: 2px solid gray;");
        return;
    }

    // at<cv::Vec3b>(fila, columna): primero y, después x.
    const cv::Vec3b pixel = imagen.at<cv::Vec3b>(y, x);
    const int b = pixel[0], g = pixel[1], r = pixel[2];
    etiquetaPixel->setText(QString("(%1, %2)  B=%3 G=%4 R=%5")
                               .arg(x).arg(y).arg(b).arg(g).arg(r));
    // Qt escribe los colores en orden R, G, B.
    muestraColor->setStyleSheet(QString("border: 2px solid gray; background: rgb(%1, %2, %3);")
                                    .arg(r).arg(g).arg(b));
}

void MainWindow::resizeEvent(QResizeEvent *evento)
{
    QMainWindow::resizeEvent(evento);
    mostrarImagen();   // la ventana cambió de tamaño (o el teléfono giró)
}
