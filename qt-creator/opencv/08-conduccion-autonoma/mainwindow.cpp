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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    construirInterfaz();
    // Cámara o stream entregan cada cuadro al mismo sumidero; el sumidero nos avisa.
    connect(&sumidero, &QVideoSink::videoFrameChanged, this, &MainWindow::alLlegarCuadro);

    // El robot: un QWebSocket como en la aplicación de la Práctica 3.
    connect(&robot, &QWebSocket::connected, this, [this] {
        botonRobot->setText("Desconectar");
        ultimoMensaje.clear();
        // El primer latido activa la telemetría.
        robot.sendTextMessage(R"({"tipo":"latido"})");
        relojLatido.start(5000);
        mostrarDecision();
    });
    connect(&robot, &QWebSocket::disconnected, this, [this] {
        botonRobot->setText("Conectar");
        relojLatido.stop();
        mostrarDecision();
    });
    connect(&robot, &QWebSocket::textMessageReceived, this, &MainWindow::alRecibirTexto);
    connect(&relojLatido, &QTimer::timeout, this, [this] {
        robot.sendTextMessage(R"({"tipo":"latido"})");
    });

    // El ciclo de decisión: 8 veces por segundo, llegue el video a 5
    // o a 30 cuadros por segundo.
    connect(&relojDecision, &QTimer::timeout, this, &MainWindow::decidir);
    relojDecision.start(125);
    relojMensajes.start();
}

MainWindow::~MainWindow()
{
    apagarCamara();
}

void MainWindow::construirInterfaz()
{
    setWindowTitle("Conducción autónoma");
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
    campoFuente = new QLineEdit("http://127.0.0.1:8080/stream.mjpg");
    campoFuente->setMinimumHeight(56);
    campoFuente->setPlaceholderText("0, 1… o http://equipo:8080/stream.mjpg");
    campoFuente->setInputMethodHints(Qt::ImhUrlCharactersOnly | Qt::ImhNoAutoUppercase);

    etiquetaEstado = new QLabel("Escribe la fuente y pulsa «Encender»");
    etiquetaEstado->setAlignment(Qt::AlignCenter);
    etiquetaEstado->setWordWrap(true);

    // ---- Nuevo en el 08 ----
    campoRobot = new QLineEdit("ws://127.0.0.1:8081");
    campoRobot->setMinimumHeight(56);
    campoRobot->setPlaceholderText("ws://IP:8081 (virtual) o ws://IP:81 (ESP32-S3)");
    campoRobot->setInputMethodHints(Qt::ImhUrlCharactersOnly | Qt::ImhNoAutoUppercase);
    botonRobot = new QPushButton("Conectar");
    botonRobot->setMinimumHeight(64);
    botonAuto = new QPushButton("Iniciar");
    botonAuto->setMinimumHeight(64);

    etiquetaDecision = new QLabel("ESPERA");
    etiquetaDecision->setAlignment(Qt::AlignCenter);
    etiquetaDecision->setWordWrap(true);
    etiquetaDecision->setStyleSheet("font-size: 20pt; font-weight: bold;");

    deslizadorUmbral = new QSlider(Qt::Horizontal);
    deslizadorUmbral->setRange(0, 255);
    deslizadorUmbral->setValue(vision.franja.umbral);
    deslizadorUmbral->setMinimumHeight(48);
    etiquetaUmbral = new QLabel(QString("Umbral %1").arg(vision.franja.umbral));
    casillaOscuras = new QCheckBox("Rayas oscuras");
    casillaReal = new QCheckBox("Robot real");
    for (QCheckBox *casilla : {casillaOscuras, casillaReal}) {
        casilla->setMinimumHeight(56);
        // Una hoja de estilo en el widget deja de heredar la letra del padre: repetimos 20 pt.
        casilla->setStyleSheet("QCheckBox { font-size: 20pt; }"
                               "QCheckBox::indicator { width: 36px; height: 36px; }");
    }

    auto *filaFuente = new QFormLayout;
    filaFuente->addRow("Fuente:", campoFuente);
    filaFuente->addRow("Robot:", campoRobot);

    auto *filaUmbral = new QHBoxLayout;
    filaUmbral->addWidget(etiquetaUmbral);
    filaUmbral->addWidget(deslizadorUmbral, 1);

    auto *filaCasillas = new QHBoxLayout;
    filaCasillas->addWidget(casillaOscuras);
    filaCasillas->addWidget(casillaReal);

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
    columna->addLayout(filaUmbral);
    columna->addLayout(filaCasillas);
    columna->addLayout(filaFuente);
    columna->addLayout(filaBotones);
    columna->addLayout(filaRobot);
    setCentralWidget(central);
    resize(900, 1000);

    connect(botonCamara, &QPushButton::clicked, this, &MainWindow::encenderApagar);
    connect(botonCambiar, &QPushButton::clicked, this, &MainWindow::cambiarCamara);
    connect(botonRobot, &QPushButton::clicked, this, &MainWindow::conectarRobot);
    connect(botonAuto, &QPushButton::clicked, this, &MainWindow::iniciarDetener);
    connect(deslizadorUmbral, &QSlider::valueChanged, this, [this](int valor) {
        vision.franja.umbral = valor;
        etiquetaUmbral->setText(QString("Umbral %1").arg(valor));
        procesarPendiente();   // con la cámara apagada, vuelve a analizar el último cuadro
    });
    connect(casillaOscuras, &QCheckBox::toggled, this, [this](bool oscuras) {
        vision.franja.rayasOscuras = oscuras;
        procesarPendiente();
    });
}

void MainWindow::arrancar(const QString &fuente, const QString &direccionRobot,
                          bool automatico)
{
    if (!fuente.isEmpty()) {
        campoFuente->setText(fuente);
        encenderApagar();
    }
    if (!direccionRobot.isEmpty()) {
        campoRobot->setText(direccionRobot);
        conectarRobot();
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
    anchoReferencia = 0;              // otra cámara, otro ancho de carril
    cuadros = 0;
    cps = 0;
    reloj.start();
}

// Un stream (el robot virtual, una cámara IP, una ESP32-CAM) no es QCamera: lo reproduce un
// QMediaPlayer, que entrega sus cuadros al MISMO sumidero. De ahí en adelante el camino es
// idéntico: QVideoFrame -> QImage -> cv::Mat.
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
    anchoReferencia = 0;
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
        desdeCuadro.start();         // llegó video: la vigilancia vuelve a cero
    } else if (resultado.empty()) {
        return;
    }
    // 3) Aquí trabaja OpenCV: la franja y las paletas. El resultado es pequeño (una Vista):
    //    la máquina de estados no ve píxeles, ve «dos rayas, error -12, paleta roja».
    ultimaVista = percibir(resultado, vision, anchoReferencia);
    cv::Mat vista = resultado.clone();
    dibujarVista(vista, ultimaVista);
    // 4) cv::Mat -> QImage -> pantalla.
    etiquetaVideo->setPixmap(QPixmap::fromImage(aQImage(vista))
                                 .scaled(etiquetaVideo->size(), Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation));

    // Cada segundo: cuántos cuadros alcanzamos a mostrar.
    if ((camara || reproductor) && reloj.elapsed() >= 1000) {
        cps = cuadros * 1000.0 / reloj.restart();
        cuadros = 0;
        etiquetaEstado->setText(QString("%1 · %2 × %3 · %4 cuadros/s · "
                                        "rayas: %5 · error %6 px")
                                    .arg(nombreFuente)
                                    .arg(vista.cols).arg(vista.rows)
                                    .arg(cps, 0, 'f', 1)
                                    .arg(nombre(ultimaVista.carril.lineas))
                                    .arg(ultimaVista.carril.error));
    }
}

void MainWindow::resizeEvent(QResizeEvent *evento)
{
    QMainWindow::resizeEvent(evento);
    procesarPendiente();   // vuelve a escalar el último cuadro al nuevo tamaño
}

// ------------------------------------------------------------------ nuevo en el 08

void MainWindow::conectarRobot()
{
    if (robot.state() != QAbstractSocket::UnconnectedState) {
        robot.close();
        return;
    }
    const QUrl direccion = QUrl::fromUserInput(campoRobot->text().trimmed());
    if (!direccion.isValid() || !direccion.scheme().startsWith("ws")) {
        ultimaRespuesta = "Robot no válido: escribe ws://IP:PUERTO";
        mostrarDecision();
        return;
    }
    botonRobot->setText("Conectando…");
    robot.open(direccion);
}

void MainWindow::alRecibirTexto(const QString &texto)
{
    const QJsonObject json = QJsonDocument::fromJson(texto.toUtf8()).object();
    const QString tipo = json.value("tipo").toString();
    if (tipo == "telemetria" && json.contains("kmh"))
        ultimaRespuesta = QString("%1 km/h").arg(json.value("kmh").toDouble(), 0, 'f', 1);
    else if (tipo == "mover_err")
        ultimaRespuesta = "El robot no obedece: " + json.value("motivo").toString();
    else if (tipo == "infraccion")
        ultimaRespuesta = "Infracción: " + json.value("clase").toString();
    // Los demás (saludo, latido_ok, conducir_ack, mover_ack…) no se muestran.
}

void MainWindow::iniciarDetener()
{
    if (maquina.estado() == Estado::Espera || maquina.estado() == Estado::Paro)
        maquina.iniciar();
    else
        maquina.detener();
    decidir();   // que el cambio se note de inmediato, sin esperar al reloj
}

// UN ciclo de decisión. Nunca se manda un mensaje por cuadro: el video puede llegar
// a 30 cuadros por segundo, pero aquí se decide 8 veces por segundo con lo último
// que se vio.
void MainWindow::decidir()
{
    // Vigilancias: sin robot o sin video reciente no se puede conducir.
    if (robot.state() != QAbstractSocket::ConnectedState)
        maquina.paroDeSeguridad("sin conexión con el robot");
    else if (!desdeCuadro.isValid() || desdeCuadro.elapsed() > 1000)
        maquina.paroDeSeguridad("sin video");

    Medida medida;
    medida.lineas = ultimaVista.carril.lineas;
    medida.error = ultimaVista.carril.error;
    medida.color = ultimaVista.color;

    const Orden orden = maquina.paso(medida, relojDecision.interval());
    enviarOrden(orden);
    mostrarDecision();
}

void MainWindow::enviarOrden(const Orden &orden)
{
    if (robot.state() != QAbstractSocket::ConnectedState)
        return;

    QJsonObject json;
    bool esParar = false;
    if (casillaReal->isChecked()) {
        // Robot real (practica4.ino): solo entiende «mover» con accion y ms.
        const MovimientoReal movimiento = aRobotReal(orden);
        json["tipo"] = "mover";
        json["accion"] = movimiento.accion;
        esParar = movimiento.ms == 0;
        if (!esParar) {
            json["ms"] = movimiento.ms;
            // El ESP32-S3 ignora «vel» (sus motores son todo o nada). El coche virtual sí lo
            // lee: sin «vel», «mover» acelera al 100 % y se sale de la pista.
            json["vel"] = 30;
        }
    } else if (orden.tipo == Orden::Tipo::Parar) {
        json["tipo"] = "mover";
        json["accion"] = "parar";
        esParar = true;
    } else {
        // Robot virtual: control continuo. ms es una vigilancia: si en 500 ms no llega otro
        // «conducir», el coche frena solo.
        json["tipo"] = "conducir";
        json["acelerador"] = orden.acelerador;
        json["direccion"] = orden.direccion;
        json["freno"] = orden.freno;
        json["ms"] = 500;
    }
    const QString mensaje =
        QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact));

    // «parar» se manda UNA vez, al cambiar la decisión. Lo que mueve al robot sí se repite en
    // cada ciclo, porque el robot se detiene si deja de recibirlo.
    if (esParar && mensaje == ultimoMensaje)
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
    const bool detenida = estado == Estado::Espera || estado == Estado::Paro;
    botonAuto->setText(detenida ? "Iniciar" : "Detener");
    QString texto = QString("%1 · marcha %2 · paleta %3 · %4 mensajes/s")
                        .arg(nombre(estado)).arg(nombre(maquina.marcha()))
                        .arg(nombre(maquina.colorFirme())).arg(mps, 0, 'f', 0);
    if (estado == Estado::Paro)
        texto += QString("\nPARO: %1").arg(maquina.motivo());
    if (robot.state() != QAbstractSocket::ConnectedState)
        texto += "\nRobot sin conectar";
    else if (!ultimaRespuesta.isEmpty())
        texto += "\n" + ultimaRespuesta;
    etiquetaDecision->setText(texto);
}
