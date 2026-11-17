#include "mainwindow.h"
#include "puente.h"

#include <QApplication>
#include <QCameraDevice>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMediaDevices>
#include <QPermissions>
#include <QPixmap>
#include <QUrl>
#include <QVBoxLayout>
#include <opencv2/imgproc.hpp>

// ====================== PRÁCTICA 5: ESCRIBE AQUÍ TU NOMBRE Y TU CÓDIGO ======================
// Sale escrito sobre la imagen, en tu captura y en tu video. Sin acentos ni eñe.
static const char *ALUMNO = "TU NOMBRE - TU CODIGO";
// ============================================================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    construirInterfaz();
    // Cámara o stream entregan cada cuadro al mismo sumidero; el sumidero nos avisa.
    connect(&sumidero, &QVideoSink::videoFrameChanged, this, &MainWindow::alLlegarCuadro);

    // El robot: un QWebSocket como en la app de la Práctica 3.
    connect(&robot, &QWebSocket::connected, this, [this] {
        botonRobot->setText("Desconectar");
        ultimoMensaje.clear();
        robot.sendTextMessage(R"({"tipo":"latido"})");
        relojLatido.start(5000);
        mostrarDecision();
    });
    connect(&robot, &QWebSocket::disconnected, this, [this] {
        botonRobot->setText("Conectar");
        relojLatido.stop();
        mostrarDecision();
    });
    connect(&relojLatido, &QTimer::timeout, this, [this] {
        robot.sendTextMessage(R"({"tipo":"latido"})");
    });

    // El ciclo de decisión: 8 veces por segundo, llegue el video a 5 o a 30 cuadros por segundo.
    connect(&relojDecision, &QTimer::timeout, this, &MainWindow::decidir);
    relojDecision.start(125);
    relojMensajes.start();
    mostrarDecision();   // la etiqueta nace con sus tres renglones: el layout le da su alto
}

MainWindow::~MainWindow()
{
    apagarCamara();
}

void MainWindow::construirInterfaz()
{
    setWindowTitle("Robot de cuatro colores");
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

    // Como en OpenCV: un número es una cámara local; una URL es un video por red.
    campoFuente = new QLineEdit("0");
    campoFuente->setMinimumHeight(56);
    campoFuente->setPlaceholderText("0, 1… o http://equipo:8080/video");
    campoFuente->setInputMethodHints(Qt::ImhUrlCharactersOnly | Qt::ImhNoAutoUppercase);

    etiquetaEstado = new QLabel("Escribe la fuente y pulsa «Encender»");
    etiquetaEstado->setAlignment(Qt::AlignCenter);
    etiquetaEstado->setWordWrap(true);

    campoRobot = new QLineEdit("ws://192.168.1.50:81");
    campoRobot->setMinimumHeight(56);
    campoRobot->setPlaceholderText("ws://IP-de-tu-ESP32:81");
    campoRobot->setInputMethodHints(Qt::ImhUrlCharactersOnly | Qt::ImhNoAutoUppercase);
    botonRobot = new QPushButton("Conectar");
    botonRobot->setMinimumHeight(64);
    botonAuto = new QPushButton("Iniciar");
    botonAuto->setMinimumHeight(64);

    etiquetaDecision = new QLabel("ESPERA");
    etiquetaDecision->setAlignment(Qt::AlignCenter);
    etiquetaDecision->setWordWrap(true);
    etiquetaDecision->setStyleSheet("font-size: 20pt; font-weight: bold;");

    // «Aprender»: con la hoja llenando el recuadro, un toque ajusta el rango de ese color
    // a lo que ve TU cámara con TU luz. No hace falta recompilar para calibrar.
    etiquetaRangos = new QLabel("Aprender: llena el recuadro con la hoja y toca su color");
    etiquetaRangos->setAlignment(Qt::AlignCenter);
    etiquetaRangos->setWordWrap(true);
    auto *filaAprender = new QHBoxLayout;
    for (Color color : {Color::Verde, Color::Rojo, Color::Azul, Color::Amarillo}) {
        auto *boton = new QPushButton(nombre(color));
        boton->setMinimumHeight(56);
        filaAprender->addWidget(boton);
        connect(boton, &QPushButton::clicked, this, [this, color] { aprender(color); });
    }

    auto *filaFuente = new QFormLayout;
    filaFuente->addRow("Fuente:", campoFuente);
    filaFuente->addRow("Robot:", campoRobot);

    auto *filaBotones = new QHBoxLayout;
    filaBotones->addWidget(botonCamara);
    filaBotones->addWidget(botonCambiar);

    auto *filaRobot = new QHBoxLayout;
    filaRobot->addWidget(botonRobot);
    filaRobot->addWidget(botonAuto);

    // Video arriba (se estira: factor 1) y controles abajo: sirve en ventana y en teléfono.
    auto *columna = new QVBoxLayout(central);
    columna->addWidget(etiquetaVideo, 1);
    columna->addWidget(etiquetaEstado);
    columna->addWidget(etiquetaDecision);
    columna->addWidget(etiquetaRangos);
    columna->addLayout(filaAprender);
    columna->addLayout(filaFuente);
    columna->addLayout(filaBotones);
    columna->addLayout(filaRobot);
    setCentralWidget(central);
    resize(800, 900);

    connect(botonCamara, &QPushButton::clicked, this, &MainWindow::encenderApagar);
    connect(botonCambiar, &QPushButton::clicked, this, &MainWindow::cambiarCamara);
    connect(botonRobot, &QPushButton::clicked, this, &MainWindow::conectarRobot);
    connect(botonAuto, &QPushButton::clicked, this, &MainWindow::iniciarDetener);
}

void MainWindow::arrancar(const QString &fuente, const QString &direccionRobot, bool automatico,
                          const QString &colorPorAprender)
{
    if (!fuente.isEmpty()) {
        campoFuente->setText(fuente);
        encenderApagar();
    }
    if (!direccionRobot.isEmpty()) {
        campoRobot->setText(direccionRobot);
        conectarRobot();
    }
    for (Color color : {Color::Verde, Color::Rojo, Color::Azul, Color::Amarillo}) {
        if (colorPorAprender == nombre(color))   // como tocar el botón Aprender, a los 2.5 s
            QTimer::singleShot(2500, this, [this, color] { aprender(color); });
    }
    if (automatico)   // da tiempo a que llegue el primer cuadro y a que conecte el robot
        QTimer::singleShot(3000, this, &MainWindow::iniciarDetener);
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

// Un stream no es QCamera: lo reproduce un QMediaPlayer, que entrega sus cuadros al MISMO
// sumidero. De ahí en adelante el camino es idéntico: QVideoFrame -> QImage -> cv::Mat.
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

// Solo guardamos el ÚLTIMO cuadro y dejamos UNA conversión pendiente; los cuadros que llegan
// mientras tanto reemplazan al anterior (se descartan). Igual que en el 07.
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
        // 1) QVideoFrame -> QImage -> cv::Mat BGR, con el puente del video 2.
        const QImage imagen = pendiente.toImage();
        pendiente = QVideoFrame();
        if (imagen.isNull())
            return;
        resultado = aMat(imagen);
        ++cuadros;
        desdeCuadro.start();         // llegó video: la vigilancia vuelve a cero
    } else if (resultado.empty()) {
        return;
    }
    // 2) Aquí trabaja OpenCV: ¿de qué color es el centro? El resultado es pequeño (una Zona):
    //    la máquina de estados no ve píxeles, ve «verde».
    ultimaZona = mirarZona(resultado);

    // 3) Se escribe sobre el cuadro lo que se vio en ESTE cuadro y tu nombre.
    cv::Mat vista = resultado.clone();
    const QString arriba = QString("%1 -> %2  HSV centro %3 %4 %5")
                               .arg(nombre(ultimaZona.color)).arg(nombre(ordenDe(ultimaZona.color)))
                               .arg(ultimaZona.h).arg(ultimaZona.s).arg(ultimaZona.v);
    dibujarZona(vista, ultimaZona, arriba.toUtf8().constData(), ALUMNO);

    // 4) cv::Mat -> QImage -> pantalla.
    etiquetaVideo->setPixmap(QPixmap::fromImage(aQImage(vista))
                                 .scaled(etiquetaVideo->size(), Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation));

    // Cada segundo: cuántos cuadros alcanzamos a mostrar.
    if ((camara || reproductor) && reloj.elapsed() >= 1000) {
        cps = cuadros * 1000.0 / reloj.restart();
        cuadros = 0;
        etiquetaEstado->setText(QString("%1 · %2 × %3 · %4 cuadros/s · %5 llena el %6 %")
                                    .arg(nombreFuente)
                                    .arg(vista.cols).arg(vista.rows)
                                    .arg(cps, 0, 'f', 1)
                                    .arg(nombre(ultimaZona.color))
                                    .arg(ultimaZona.porcentaje));
    }
}

void MainWindow::resizeEvent(QResizeEvent *evento)
{
    QMainWindow::resizeEvent(evento);
    procesarPendiente();   // vuelve a escalar el último cuadro al nuevo tamaño
}

// ------------------------------------------------------------------ el robot y las decisiones

void MainWindow::conectarRobot()
{
    if (robot.state() != QAbstractSocket::UnconnectedState) {
        robot.close();
        return;
    }
    const QUrl direccion = QUrl::fromUserInput(campoRobot->text().trimmed());
    if (!direccion.isValid() || !direccion.scheme().startsWith("ws")) {
        etiquetaDecision->setText("Robot no válido: escribe ws://IP:81");
        return;
    }
    botonRobot->setText("Conectando…");
    robot.open(direccion);
}

void MainWindow::iniciarDetener()
{
    if (maquina.estado() == Estado::Obedecer)
        maquina.detener();
    else
        maquina.iniciar();
    decidir();   // que el cambio se note de inmediato, sin esperar al reloj
}

// UN ciclo de decisión. Nunca se manda un mensaje por cuadro: el video puede llegar a 30 cuadros
// por segundo, pero aquí se decide 8 veces por segundo con lo último que se vio.
void MainWindow::decidir()
{
    // Vigilancias: sin robot o sin video reciente no se puede obedecer.
    if (robot.state() != QAbstractSocket::ConnectedState)
        maquina.paroDeSeguridad("sin conexión con el robot");
    else if (!desdeCuadro.isValid() || desdeCuadro.elapsed() > 1000)
        maquina.paroDeSeguridad("sin video");

    // Sin video reciente, lo último que se vio ya no vale: cuenta como «ninguno».
    const bool hayVideo = desdeCuadro.isValid() && desdeCuadro.elapsed() <= 1000;
    ultimaAccion = maquina.paso(hayVideo ? ultimaZona.color : Color::Ninguno);
    enviarAccion(ultimaAccion);
    mostrarDecision();
}

void MainWindow::enviarAccion(Accion accion)
{
    if (robot.state() != QAbstractSocket::ConnectedState)
        return;

    // El robot real (firmware de las prácticas 3 y 4) solo entiende «mover» con accion y ms.
    const Movimiento movimiento = movimientoDe(accion);
    QJsonObject json;
    json["tipo"] = "mover";
    json["accion"] = movimiento.accion;
    if (movimiento.ms > 0) {
        json["ms"] = movimiento.ms;
        // El ESP32-S3 ignora «vel» (sus motores son todo o nada). El robot virtual sí lo lee:
        // sin «vel», «mover» acelera al 100 %. Es el mismo JSON que manda el programa 08.
        json["vel"] = 30;
    }
    const QString mensaje = QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact));

    // «parar» se manda UNA vez, al cambiar la decisión. Lo que mueve al robot sí se repite en
    // cada ciclo: el robot se detiene solo si deja de recibirlo (se cumple su ms).
    if (accion == Accion::Parar && mensaje == ultimoMensaje)
        return;
    ultimoMensaje = mensaje;
    robot.sendTextMessage(mensaje);
    ++mensajes;
}

void MainWindow::mostrarDecision()
{
    if (relojMensajes.elapsed() >= 1000) {
        mps = mensajes * 1000.0 / relojMensajes.restart();
        mensajes = 0;
    }
    const Estado estado = maquina.estado();
    botonAuto->setText(estado == Estado::Obedecer ? "Detener" : "Iniciar");
    // Siempre tres renglones cortos: en el teléfono no se parten ni cambian el alto de la etiqueta.
    QString texto = QString("%1 · color firme: %2\norden: %3 · %4 mensajes/s\n")
                        .arg(nombre(estado)).arg(nombre(maquina.colorFirme()))
                        .arg(nombre(ultimaAccion)).arg(mps, 0, 'f', 0);
    if (estado == Estado::Paro)
        texto += maquina.motivo();          // «sin video» o «sin conexión con el robot»
    else if (robot.state() != QAbstractSocket::ConnectedState)
        texto += "Robot sin conectar";
    else
        texto += "Robot conectado";
    etiquetaDecision->setText(texto);
}

// Botones «Aprender»: el rango de ese color pasa a ser lo que mide el píxel central ahora.
void MainWindow::aprender(Color color)
{
    if (resultado.empty()) {
        etiquetaRangos->setText("Primero enciende la cámara.");
        return;
    }
    const std::vector<RangoHsv> rangos = rangosAprendidos(ultimaZona.h, ultimaZona.s, ultimaZona.v);
    fijarRangos(color, rangos);
    QString texto = QString("%1 aprendido: H %2-%3").arg(nombre(color)).arg(rangos[0].hMin).arg(rangos[0].hMax);
    if (rangos.size() > 1)
        texto += QString(" y %1-%2").arg(rangos[1].hMin).arg(rangos[1].hMax);
    texto += QString(" · S min %1 · V min %2").arg(rangos[0].sMin).arg(rangos[0].vMin);
    etiquetaRangos->setText(texto);
    procesarPendiente();   // vuelve a clasificar el cuadro actual con el rango nuevo
}
