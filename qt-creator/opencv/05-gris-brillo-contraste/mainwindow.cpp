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

    casillaGris = new QCheckBox("Escala de grises");
    casillaGris->setMinimumHeight(56);
    // El cuadrito de la casilla también crece: se toca con el dedo. Una hoja de estilo
    // en el widget deja de heredar la letra del padre: por eso repetimos 20 pt.
    casillaGris->setStyleSheet("QCheckBox { font-size: 20pt; }"
                               "QCheckBox::indicator { width: 36px; height: 36px; }");

    // Contraste: alpha de 0.0 a 3.0 (el deslizador solo da enteros: 0..30 / 10).
    etiquetaContraste = new QLabel;
    deslizadorContraste = new QSlider(Qt::Horizontal);
    deslizadorContraste->setRange(0, 30);
    deslizadorContraste->setValue(10);
    // Brillo: beta de -100 a 100, se suma a cada píxel.
    etiquetaBrillo = new QLabel;
    deslizadorBrillo = new QSlider(Qt::Horizontal);
    deslizadorBrillo->setRange(-100, 100);
    deslizadorBrillo->setValue(0);
    // Deslizadores altos y con una perilla grande para moverlos con el dedo.
    const QString estilo =
        "QSlider::groove:horizontal { height: 12px; border-radius: 6px;"
        " background: #c8c8c8; }"
        "QSlider::handle:horizontal { width: 44px; margin: -16px 0;"
        " border-radius: 22px; background: #2a6fdb; }";
    for (QSlider *deslizador : {deslizadorContraste, deslizadorBrillo}) {
        deslizador->setMinimumHeight(56);
        deslizador->setStyleSheet(estilo);
    }

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
    capas->addWidget(casillaGris);
    capas->addWidget(etiquetaContraste);
    capas->addWidget(deslizadorContraste);
    capas->addWidget(etiquetaBrillo);
    capas->addWidget(deslizadorBrillo);
    capas->addWidget(botonDibujar);
    capas->addWidget(etiquetaEstado);
    setCentralWidget(central);

    // Señal clicked del botón -> nuestra función (puntero a función miembro).
    connect(botonDibujar, &QPushButton::clicked, this, &MainWindow::dibujarConOpenCV);
    // Las tres señales llevan a la misma función. valueChanged(int) trae el valor,
    // pero aplicarAjustes() no lo necesita: lo lee del deslizador.
    connect(casillaGris, &QCheckBox::toggled, this, &MainWindow::aplicarAjustes);
    connect(deslizadorContraste, &QSlider::valueChanged, this, &MainWindow::aplicarAjustes);
    connect(deslizadorBrillo, &QSlider::valueChanged, this, &MainWindow::aplicarAjustes);
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
    aplicarAjustes();
}

void MainWindow::aplicarAjustes()
{
    if (imagen.empty())
        return;
    cv::Mat base;
    if (casillaGris->isChecked())
        cv::cvtColor(imagen, base, cv::COLOR_BGR2GRAY);   // 3 canales -> 1 canal
    else
        base = imagen;

    const double alpha = deslizadorContraste->value() / 10.0;
    const int beta = deslizadorBrillo->value();
    // resultado = alpha · píxel + beta, recortado a 0..255 (no se desborda).
    base.convertTo(resultado, -1, alpha, beta);

    etiquetaContraste->setText(QString("Contraste: alpha = %1").arg(alpha, 0, 'f', 1));
    etiquetaBrillo->setText(QString("Brillo: beta = %1").arg(beta));
    mostrarImagen();
}

void MainWindow::mostrarImagen()
{
    if (resultado.empty())
        return;
    // En el teléfono cada punto lógico son varios píxeles: escalamos a píxeles reales
    // para que la imagen se vea nítida.
    const qreal densidad = devicePixelRatioF();
    QPixmap pixmap = QPixmap::fromImage(aQImage(resultado));
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
    aplicarAjustes();
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
    if (resultado.empty())
        return;
    // La imagen se ve escalada (sin deformar) y centrada dentro de la etiqueta.
    const double escala = std::min(double(etiquetaImagen->width()) / resultado.cols,
                                   double(etiquetaImagen->height()) / resultado.rows);
    const double margenX = (etiquetaImagen->width() - resultado.cols * escala) / 2;
    const double margenY = (etiquetaImagen->height() - resultado.rows * escala) / 2;

    // De coordenadas de la etiqueta a coordenadas de la imagen original.
    const int x = int(std::floor((punto.x() - margenX) / escala));
    const int y = int(std::floor((punto.y() - margenY) / escala));
    if (x < 0 || y < 0 || x >= resultado.cols || y >= resultado.rows) {
        etiquetaPixel->setText("Fuera de la imagen");
        muestraColor->setStyleSheet("border: 2px solid gray;");
        return;
    }

    // Leemos lo que se ve (resultado). at<>(fila, columna): primero y, después x.
    int b, g, r;
    if (resultado.channels() == 1) {
        b = g = r = resultado.at<uchar>(y, x);   // gris: un solo valor
        etiquetaPixel->setText(QString("(%1, %2)  gris=%3").arg(x).arg(y).arg(b));
    } else {
        const cv::Vec3b pixel = resultado.at<cv::Vec3b>(y, x);
        b = pixel[0];
        g = pixel[1];
        r = pixel[2];
        etiquetaPixel->setText(QString("(%1, %2)  B=%3 G=%4 R=%5")
                                   .arg(x).arg(y).arg(b).arg(g).arg(r));
    }
    // Qt escribe los colores en orden R, G, B.
    muestraColor->setStyleSheet(QString("border: 2px solid gray; background: rgb(%1, %2, %3);")
                                    .arg(r).arg(g).arg(b));
}

void MainWindow::resizeEvent(QResizeEvent *evento)
{
    QMainWindow::resizeEvent(evento);
    mostrarImagen();   // la ventana cambió de tamaño (o el teléfono giró)
}
