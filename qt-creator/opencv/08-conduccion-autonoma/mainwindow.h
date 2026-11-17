#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "fsm.h"
#include "percepcion.h"

#include <QCamera>
#include <QCheckBox>
#include <QElapsedTimer>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMediaCaptureSession>
#include <QMediaPlayer>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QVideoFrame>
#include <QVideoSink>
#include <QWebSocket>
#include <opencv2/core.hpp>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Para pruebas desde la terminal: llena los campos y pulsa los botones por ti.
    void arrancar(const QString &fuente, const QString &robot, bool automatico);

protected:
    void resizeEvent(QResizeEvent *evento) override;

private:
    // ---- Igual que en el 07: la cámara o el stream ----
    void construirInterfaz();
    void encenderApagar();          // botón principal: lee la fuente (número o URL)
    void pedirPermiso();            // Android pregunta al usuario; el escritorio ya lo concede
    void abrirCamara();             // crea la QCamera elegida y la conecta a la sesión
    void abrirStream(const QUrl &direccion);  // video por red: http://…, rtsp://…
    void apagarCamara();            // apaga la fuente que esté encendida (cámara o stream)
    void cambiarCamara();           // pasa a la siguiente cámara de la lista
    void alLlegarCuadro(const QVideoFrame &cuadro);
    void procesarPendiente();       // convierte, PERCIBE y muestra SOLO el último cuadro

    // ---- Nuevo en el 08: el robot y la máquina de estados ----
    void conectarRobot();           // botón Conectar / Desconectar (QWebSocket)
    void alRecibirTexto(const QString &texto);   // respuestas JSON del robot
    void iniciarDetener();          // botón Iniciar / Detener (conducción autónoma)
    void decidir();                 // UN ciclo de decisión: lo llama relojDecision cada 125 ms
    void enviarOrden(const Orden &orden);        // Orden -> JSON -> WebSocket
    void mostrarDecision();         // estado, marcha, color y mensajes por segundo

    QLineEdit *campoFuente;         // «0», «1»… = cámara; «http://…» = streaming
    QLabel *etiquetaVideo;          // aquí dibujamos cada cuadro ya procesado por OpenCV
    QPushButton *botonCamara;       // Encender / Apagar
    QPushButton *botonCambiar;      // otra cámara (frontal, trasera, otra webcam)
    QLabel *etiquetaEstado;         // qué cámara, tamaño del cuadro y cuadros por segundo

    QLineEdit *campoRobot;          // ws://IP:PUERTO del robot (virtual o real)
    QPushButton *botonRobot;        // Conectar / Desconectar
    QPushButton *botonAuto;         // Iniciar / Detener
    QLabel *etiquetaDecision;       // qué decidió la máquina de estados
    QSlider *deslizadorUmbral;      // gris a partir del cual una columna es raya
    QLabel *etiquetaUmbral;
    QCheckBox *casillaOscuras;      // tiras negras sobre piso claro (pista real)
    QCheckBox *casillaReal;         // robot real: «mover» en lugar de «conducir»

    QMediaCaptureSession sesion;    // une la cámara con quien recibe los cuadros
    QVideoSink sumidero;            // recibe cada cuadro (QVideoFrame)
    QCamera *camara = nullptr;
    int indiceCamara = 0;           // posición en QMediaDevices::videoInputs()
    QMediaPlayer *reproductor = nullptr;  // reproduce el stream y entrega sus cuadros
    QString nombreFuente;           // para la línea de estado

    QVideoFrame pendiente;          // último cuadro recibido, aún sin convertir
    bool programado = false;        // ya hay una conversión esperando en la cola de eventos
    cv::Mat resultado;              // último cuadro en OpenCV (color BGR)
    QElapsedTimer reloj;            // para medir cuadros por segundo
    int cuadros = 0;                // cuadros mostrados desde la última medición
    double cps = 0;                 // cuadros por segundo medidos

    ParametrosVision vision;        // franja, umbral, zona de color
    int anchoReferencia = 0;        // ancho del carril (px) la última vez que se vieron dos rayas
    Vista ultimaVista;              // lo que se vio en el último cuadro
    QElapsedTimer desdeCuadro;      // hace cuánto llegó el último cuadro (vigilancia del video)

    QWebSocket robot;               // comandos JSON al robot
    QTimer relojDecision;           // 125 ms: 8 decisiones por segundo
    QTimer relojLatido;             // 5 s: mantiene viva la conexión y la telemetría
    MaquinaConduccion maquina;      // la máquina de estados (fsm.h)
    QString ultimoMensaje;          // último JSON enviado (un «parar» no se repite)
    QString ultimaRespuesta;        // lo último que dijo el robot (telemetría, errores)
    QElapsedTimer relojMensajes;    // para medir mensajes por segundo
    int mensajes = 0;
    double mps = 0;
};
#endif // MAINWINDOW_H
