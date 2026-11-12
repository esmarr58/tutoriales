#include "mainwindow.h"
#include "puente.h"

#include <QApplication>
#include <QCameraDevice>
#include <QHBoxLayout>
#include <QMediaDevices>
#include <QPermissions>
#include <QPixmap>
#include <QVBoxLayout>
#include <opencv2/imgproc.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    construirInterfaz();
    // La sesión entrega los cuadros de la cámara al sumidero; el sumidero nos avisa de cada uno.
    sesion.setVideoSink(&sumidero);
    connect(&sumidero, &QVideoSink::videoFrameChanged, this, &MainWindow::alLlegarCuadro);
}

MainWindow::~MainWindow()
{
    apagarCamara();
}

void MainWindow::construirInterfaz()
{
    setWindowTitle("Cámara OpenCV");
    auto *central = new QWidget(this);
    // Letra de 20 pt: la heredan todos los widgets y se lee en el teléfono.
    QFont letra = central->font();
    letra.setPointSize(20);
    central->setFont(letra);

    etiquetaVideo = new QLabel("Cámara apagada");
    etiquetaVideo->setAlignment(Qt::AlignCenter);
    etiquetaVideo->setMinimumSize(160, 120);
    // Ignored: la etiqueta usa el espacio que le da el layout, no el tamaño del cuadro.
    etiquetaVideo->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    etiquetaVideo->setStyleSheet("background: black; color: white; font-size: 20pt;");

    botonCamara = new QPushButton("Encender cámara");
    botonCamara->setMinimumHeight(64);   // grande para el dedo
    botonCambiar = new QPushButton("Cambiar cámara");
    botonCambiar->setMinimumHeight(64);
    botonCambiar->setEnabled(false);     // se activa con la cámara encendida y más de una cámara

    casillaGris = new QCheckBox("Escala de grises (OpenCV)");
    casillaGris->setMinimumHeight(56);
    // Una hoja de estilo en el widget deja de heredar la letra del padre: repetimos 20 pt.
    casillaGris->setStyleSheet("QCheckBox { font-size: 20pt; }"
                               "QCheckBox::indicator { width: 36px; height: 36px; }");

    etiquetaEstado = new QLabel("Pulsa «Encender cámara»");
    etiquetaEstado->setAlignment(Qt::AlignCenter);
    etiquetaEstado->setWordWrap(true);

    auto *filaBotones = new QHBoxLayout;
    filaBotones->addWidget(botonCamara);
    filaBotones->addWidget(botonCambiar);

    // Video arriba (se estira: factor 1) y controles abajo: sirve en ventana y en teléfono.
    auto *columna = new QVBoxLayout(central);
    columna->addWidget(etiquetaVideo, 1);
    columna->addWidget(etiquetaEstado);
    columna->addWidget(casillaGris);
    columna->addLayout(filaBotones);
    setCentralWidget(central);
    resize(900, 800);

    connect(botonCamara, &QPushButton::clicked, this, &MainWindow::encenderApagar);
    connect(botonCambiar, &QPushButton::clicked, this, &MainWindow::cambiarCamara);
    // Con la cámara apagada, la casilla vuelve a procesar el último cuadro.
    connect(casillaGris, &QCheckBox::toggled, this, &MainWindow::procesarPendiente);
}

void MainWindow::encenderApagar()
{
    if (camara)
        apagarCamara();
    else
        pedirPermiso();
}

void MainWindow::pedirPermiso()
{
    QCameraPermission permiso;
    switch (qApp->checkPermission(permiso)) {
    case Qt::PermissionStatus::Undetermined:
        // Primera vez en Android: aparece el diálogo del sistema. Al responder, se llama la lambda.
        etiquetaEstado->setText("Pidiendo permiso para usar la cámara…");
        qApp->requestPermission(permiso, this, [this](const QPermission &respuesta) {
            if (respuesta.status() == Qt::PermissionStatus::Granted)
                abrirCamara();
            else
                etiquetaEstado->setText("Sin permiso no hay cámara.");
        });
        return;
    case Qt::PermissionStatus::Denied:
        etiquetaEstado->setText("Permiso denegado: actívalo en Ajustes › Aplicaciones.");
        return;
    case Qt::PermissionStatus::Granted:
        abrirCamara();
        return;
    }
}

void MainWindow::abrirCamara()
{
    const QList<QCameraDevice> camaras = QMediaDevices::videoInputs();
    if (camaras.isEmpty()) {
        etiquetaEstado->setText("No se encontró ninguna cámara.");
        return;
    }
    if (indiceCamara >= camaras.size())
        indiceCamara = 0;
    const QCameraDevice elegida = camaras[indiceCamara];

    camara = new QCamera(elegida, this);
    // Formato: el más pequeño de al menos 640 px de ancho. Más píxeles = más trabajo por cuadro.
    QCameraFormat mejor;
    for (const QCameraFormat &f : elegida.videoFormats()) {
        const QSize r = f.resolution();
        if (r.width() < 640)
            continue;
        if (mejor.isNull() || r.width() * r.height()
                                  < mejor.resolution().width() * mejor.resolution().height())
            mejor = f;
    }
    if (!mejor.isNull())
        camara->setCameraFormat(mejor);

    connect(camara, &QCamera::errorOccurred, this, [this](QCamera::Error, const QString &texto) {
        etiquetaEstado->setText("Error de cámara: " + texto);
    });
    sesion.setCamera(camara);
    camara->start();

    botonCamara->setText("Apagar cámara");
    botonCambiar->setEnabled(camaras.size() > 1);
    etiquetaEstado->setText("Abriendo: " + elegida.description());
    cuadros = 0;
    cps = 0;
    reloj.start();
}

void MainWindow::apagarCamara()
{
    if (!camara)
        return;
    camara->stop();
    sesion.setCamera(nullptr);
    delete camara;
    camara = nullptr;
    pendiente = QVideoFrame();
    botonCamara->setText("Encender cámara");
    botonCambiar->setEnabled(false);
    etiquetaEstado->setText("Cámara apagada");
}

void MainWindow::cambiarCamara()
{
    const int total = QMediaDevices::videoInputs().size();
    apagarCamara();
    indiceCamara = (indiceCamara + 1) % qMax(1, total);
    abrirCamara();
}

// La cámara manda de 15 a 30 cuadros por segundo. Convertir cada uno es caro: si el equipo es
// lento, los cuadros se acumulan en la cola de eventos y la memoria crece hasta que Android
// cierra la aplicación. Por eso solo guardamos el ÚLTIMO cuadro y dejamos UNA conversión
// pendiente; los cuadros que llegan mientras tanto reemplazan al anterior (se descartan).
void MainWindow::alLlegarCuadro(const QVideoFrame &cuadro)
{
    pendiente = cuadro;              // copia barata: comparte los datos del cuadro
    if (!programado) {
        programado = true;
        QMetaObject::invokeMethod(this, &MainWindow::procesarPendiente, Qt::QueuedConnection);
    }
}

void MainWindow::procesarPendiente()
{
    programado = false;
    if (pendiente.isValid()) {
        // 1) QVideoFrame -> QImage: Qt convierte el formato de la cámara (YUV) a píxeles RGB
        //    y lo gira como se ve en pantalla.
        const QImage imagen = pendiente.toImage();
        pendiente = QVideoFrame();
        if (imagen.isNull())
            return;
        // 2) QImage -> cv::Mat BGR, con el puente del video 2.
        resultado = aMat(imagen);
        ++cuadros;
    } else if (resultado.empty()) {
        return;
    }
    // 3) Aquí trabaja OpenCV: hoy, solo la escala de grises.
    cv::Mat vista = resultado;
    if (casillaGris->isChecked())
        cv::cvtColor(resultado, vista, cv::COLOR_BGR2GRAY);
    // 4) cv::Mat -> QImage -> pantalla.
    etiquetaVideo->setPixmap(QPixmap::fromImage(aQImage(vista))
                                 .scaled(etiquetaVideo->size(), Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation));

    // Cada segundo: cuántos cuadros alcanzamos a mostrar.
    if (camara && reloj.elapsed() >= 1000) {
        cps = cuadros * 1000.0 / reloj.restart();
        cuadros = 0;
        etiquetaEstado->setText(QString("%1 · %2 × %3 · %4 · %5 cuadros/s")
                                    .arg(camara->cameraDevice().description())
                                    .arg(vista.cols).arg(vista.rows)
                                    .arg(vista.channels() == 1 ? "1 canal" : "3 canales")
                                    .arg(cps, 0, 'f', 1));
    }
}

void MainWindow::resizeEvent(QResizeEvent *evento)
{
    QMainWindow::resizeEvent(evento);
    procesarPendiente();   // vuelve a escalar el último cuadro al nuevo tamaño
}
