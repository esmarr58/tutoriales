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
    cargarMuestra(rutas[0]);
}

MainWindow::~MainWindow() {}

void MainWindow::construirInterfaz()
{
    setWindowTitle("Inspector de imágenes");
    auto *central = new QWidget(this);
    // Letra de 20 pt: la heredan todos los widgets y se lee en el teléfono.
    QFont letra = central->font();
    letra.setPointSize(20);
    central->setFont(letra);

    etiquetaImagen = new QLabel;
    etiquetaImagen->setAlignment(Qt::AlignCenter);
    // Mínimo pequeño: al arrancar en horizontal, imagen + panel deben caber en el ancho del
    // teléfono (unos 390 puntos); si no, la ventana queda más ancha que la pantalla.
    etiquetaImagen->setMinimumSize(120, 90);
    // Ignored: la etiqueta usa el espacio que le da el layout, no el tamaño de la imagen.
    etiquetaImagen->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    // Queremos enterarnos de los clics (y toques) sobre la imagen.
    etiquetaImagen->installEventFilter(this);

    // Selector de muestra: tres botones marcables en un QButtonGroup (como pestañas).
    // El id de cada botón es su posición en 'rutas'. Se marca el elegido en azul; la hoja
    // de estilo propia ya no hereda la letra: repetimos 20 pt.
    grupoMuestras = new QButtonGroup(this);
    auto *filaMuestras = new QHBoxLayout;
    const QStringList nombres = {"Formas", "Colores", "Células"};
    for (int i = 0; i < nombres.size(); ++i) {
        auto *boton = new QPushButton(nombres[i]);
        boton->setCheckable(true);
        boton->setMinimumHeight(64);
        boton->setStyleSheet("QPushButton { font-size: 20pt; border: 2px solid #2a6fdb;"
                             " border-radius: 8px; }"
                             "QPushButton:checked { background: #2a6fdb; color: white; }");
        grupoMuestras->addButton(boton, i);
        filaMuestras->addWidget(boton);
    }
    grupoMuestras->button(0)->setChecked(true);

    muestraColor = new QLabel;
    muestraColor->setFixedSize(64, 64);
    muestraColor->setStyleSheet("border: 2px solid gray;");
    etiquetaPixel = new QLabel("Toca la imagen para leer un píxel");
    etiquetaPixel->setWordWrap(true);

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

    botonRestablecer = new QPushButton("Restablecer");
    botonRestablecer->setMinimumHeight(64);   // grande para el dedo

    etiquetaEstado = new QLabel;
    etiquetaEstado->setAlignment(Qt::AlignCenter);
    etiquetaEstado->setWordWrap(true);

    // Fila del píxel: cuadro de color a la izquierda y el texto a la derecha.
    auto *filaPixel = new QHBoxLayout;
    filaPixel->addWidget(muestraColor);
    filaPixel->addWidget(etiquetaPixel, 1);

    // Panel con todos los controles, uno debajo de otro.
    panel = new QWidget;
    auto *columna = new QVBoxLayout(panel);
    columna->setContentsMargins(0, 0, 0, 0);
    columna->addLayout(filaMuestras);
    columna->addWidget(etiquetaEstado);
    columna->addLayout(filaPixel);
    columna->addWidget(casillaGris);
    columna->addWidget(etiquetaContraste);
    columna->addWidget(deslizadorContraste);
    columna->addWidget(etiquetaBrillo);
    columna->addWidget(deslizadorBrillo);
    columna->addWidget(botonRestablecer);
    columna->addStretch();

    // Imagen (se estira: factor 1) y panel. La dirección la decide acomodar().
    capas = new QBoxLayout(QBoxLayout::TopToBottom, central);
    capas->addWidget(etiquetaImagen, 1);
    capas->addWidget(panel);
    setCentralWidget(central);

    // Una lambda: idClicked trae el id del botón pulsado; cargamos esa muestra.
    connect(grupoMuestras, &QButtonGroup::idClicked, this, [this](int id) {
        cargarMuestra(rutas[id]);
    });
    connect(casillaGris, &QCheckBox::toggled, this, &MainWindow::aplicarAjustes);
    connect(deslizadorContraste, &QSlider::valueChanged, this, &MainWindow::aplicarAjustes);
    connect(deslizadorBrillo, &QSlider::valueChanged, this, &MainWindow::aplicarAjustes);
    connect(botonRestablecer, &QPushButton::clicked, this, &MainWindow::restablecer);
    resize(1100, 700);
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
    pixelElegido = cv::Point(-1, -1);   // imagen nueva: ningún píxel elegido
    etiquetaPixel->setText("Toca la imagen para leer un píxel");
    muestraColor->setStyleSheet("border: 2px solid gray;");
    aplicarAjustes();
}

void MainWindow::restablecer()
{
    // Valores neutros: color, alpha = 1.0 y beta = 0 (la imagen tal cual).
    casillaGris->setChecked(false);
    deslizadorContraste->setValue(10);
    deslizadorBrillo->setValue(0);
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
    mostrarPixel();   // el píxel elegido se actualiza al mover los deslizadores
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
        pixelElegido = cv::Point(-1, -1);
        etiquetaPixel->setText("Fuera de la imagen");
        muestraColor->setStyleSheet("border: 2px solid gray;");
        return;
    }
    pixelElegido = cv::Point(x, y);
    mostrarPixel();
}

void MainWindow::mostrarPixel()
{
    if (pixelElegido.x < 0 || resultado.empty())
        return;
    const int x = pixelElegido.x;
    const int y = pixelElegido.y;
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

void MainWindow::acomodar()
{
    // Ventana más ancha que alta (escritorio): imagen a la izquierda y panel a la derecha.
    // Más alta que ancha (teléfono vertical): imagen arriba y panel abajo.
    // setStretch(posición, factor): cómo se reparten el espacio la imagen (0) y el panel (1).
    if (width() > height()) {
        capas->setDirection(QBoxLayout::LeftToRight);
        capas->setStretch(0, 3);   // imagen: 3 partes del ancho
        capas->setStretch(1, 2);   // panel: 2 partes
    } else {
        capas->setDirection(QBoxLayout::TopToBottom);
        capas->setStretch(0, 1);   // la imagen se queda con el alto que sobra
        capas->setStretch(1, 0);   // el panel, solo lo que necesita
    }
}

void MainWindow::resizeEvent(QResizeEvent *evento)
{
    QMainWindow::resizeEvent(evento);
    acomodar();
    mostrarImagen();   // la ventana cambió de tamaño (o el teléfono giró)
}
