#include "mainwindow.h"
#include "puente.h"

#include <QApplication>
#include <QCameraDevice>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMediaDevices>
#include <QPermissions>
#include <QPixmap>
#include <QUrl>
#include <QVBoxLayout>
#include <opencv2/imgproc.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    construirInterfaz();
    // Cámara o stream entregan cada cuadro al mismo sumidero; el sumidero nos avisa.
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

    botonCamara = new QPushButton("Encender");
    botonCamara->setMinimumHeight(64);   // grande para el dedo
    botonCambiar = new QPushButton("Cambiar cámara");
    botonCambiar->setMinimumHeight(64);
    botonCambiar->setEnabled(false);     // se activa si la cámara está encendida y hay otra

    casillaGris = new QCheckBox("Escala de grises (OpenCV)");
    casillaGris->setMinimumHeight(56);
    // Una hoja de estilo en el widget deja de heredar la letra del padre: repetimos 20 pt.
    casillaGris->setStyleSheet("QCheckBox { font-size: 20pt; }"
                               "QCheckBox::indicator { width: 36px; height: 36px; }");

    // Como en OpenCV: un número es una cámara local; una URL es un video por red.
    campoFuente = new QLineEdit("0");
    campoFuente->setMinimumHeight(56);
    campoFuente->setPlaceholderText("0, 1… o http://equipo:8080/video");
    campoFuente->setInputMethodHints(Qt::ImhUrlCharactersOnly | Qt::ImhNoAutoUppercase);

    etiquetaEstado = new QLabel("Escribe la fuente y pulsa «Encender»");
    etiquetaEstado->setAlignment(Qt::AlignCenter);
    etiquetaEstado->setWordWrap(true);

    auto *filaFuente = new QFormLayout;
    filaFuente->addRow("Fuente:", campoFuente);

    auto *filaBotones = new QHBoxLayout;
    filaBotones->addWidget(botonCamara);
    filaBotones->addWidget(botonCambiar);

    // Video arriba (se estira: factor 1) y controles abajo: sirve en ventana y en teléfono.
    auto *columna = new QVBoxLayout(central);
    columna->addWidget(etiquetaVideo, 1);
    columna->addWidget(etiquetaEstado);
    columna->addWidget(casillaGris);
    columna->addLayout(filaFuente);
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
    if (camara || reproductor) {
        apagarCamara();
        return;
    }
    const QString texto = campoFuente->text().trimmed();
    bool esNumero = false;
    const int numero = texto.toInt(&esNumero);
    if (esNumero) {
        indiceCamara = numero;        // «0» = la primera cámara de la lista
        pedirPermiso();
    } else {
        abrirStream(QUrl::fromUserInput(texto));
    }
}

void MainWindow::pedirPermiso()
{
    QCameraPermission permiso;
    switch (qApp->checkPermission(permiso)) {
    case Qt::PermissionStatus::Undetermined:
        // Primera vez en Android: diálogo del sistema; al responder, corre la lambda.
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
    // Formato: el más pequeño con al menos 640 px de ancho (más píxeles, más trabajo).
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

    connect(camara, &QCamera::errorOccurred, this,
            [this](QCamera::Error, const QString &texto) {
        etiquetaEstado->setText("Error de cámara: " + texto);
    });
    sesion.setVideoSink(&sumidero);
    sesion.setCamera(camara);
    camara->start();

    campoFuente->setText(QString::number(indiceCamara));
    nombreFuente = elegida.description();
    botonCamara->setText("Apagar");
    botonCambiar->setEnabled(camaras.size() > 1);
    etiquetaEstado->setText("Abriendo: " + elegida.description());
    cuadros = 0;
    cps = 0;
    reloj.start();
}

// Un stream (cámara IP, ESP32-CAM, teléfono con una aplicación de cámara IP) no es QCamera:
// lo reproduce un QMediaPlayer, que entrega sus cuadros al MISMO sumidero. De ahí en
// adelante el camino es idéntico: QVideoFrame -> QImage -> cv::Mat.
void MainWindow::abrirStream(const QUrl &direccion)
{
    if (!direccion.isValid() || direccion.scheme().isEmpty()) {
        etiquetaEstado->setText("Fuente no válida: escribe 0 o una URL http://…");
        return;
    }
    sesion.setVideoSink(nullptr);     // el sumidero atiende a una sola fuente a la vez
    reproductor = new QMediaPlayer(this);
    reproductor->setVideoSink(&sumidero);
    connect(reproductor, &QMediaPlayer::errorOccurred, this,
            [this](QMediaPlayer::Error, const QString &texto) {
        apagarCamara();               // no hay video: deja el botón listo para reintentar
        etiquetaEstado->setText("Error del stream: " + texto);
    });
    reproductor->setSource(direccion);
    reproductor->play();

    nombreFuente = direccion.host();
    botonCamara->setText("Apagar");
    botonCambiar->setEnabled(false);
    etiquetaEstado->setText("Conectando con " + direccion.toString() + "…");
    cuadros = 0;
    cps = 0;
    reloj.start();
}

void MainWindow::apagarCamara()
{
    if (camara) {
        camara->stop();
        sesion.setCamera(nullptr);
        delete camara;
        camara = nullptr;
    }
    if (reproductor) {
        reproductor->stop();
        reproductor->deleteLater();   // no delete: puede llamarse desde una señal suya
        reproductor = nullptr;
    }
    pendiente = QVideoFrame();
    botonCamara->setText("Encender");
    botonCambiar->setEnabled(false);
    etiquetaEstado->setText("Apagada");
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
    if ((camara || reproductor) && reloj.elapsed() >= 1000) {
        cps = cuadros * 1000.0 / reloj.restart();
        cuadros = 0;
        etiquetaEstado->setText(QString("%1 · %2 × %3 · %4 · %5 cuadros/s")
                                    .arg(nombreFuente)
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
