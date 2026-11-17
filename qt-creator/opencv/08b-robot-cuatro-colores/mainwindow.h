#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "fsm_p5.h"
#include "zona.h"

#include <QCamera>
#include <QElapsedTimer>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMediaCaptureSession>
#include <QMediaPlayer>
#include <QPushButton>
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
    void arrancar(const QString &fuente, const QString &robot, bool automatico, const QString &aprender);

protected:
    void resizeEvent(QResizeEvent *evento) override;

private:
    // ---- Igual que en el 07 y el 08: la cámara o el stream ----
    void construirInterfaz();
    void encenderApagar();          // botón principal: lee la fuente (número o URL)
    void pedirPermiso();            // Android pregunta al usuario; el escritorio ya lo concede
    void abrirCamara();             // crea la QCamera elegida y la conecta a la sesión
    void abrirStream(const QUrl &direccion);  // video por red: http://…
    void apagarCamara();            // apaga la fuente que esté encendida (cámara o stream)
    void cambiarCamara();           // pasa a la siguiente cámara de la lista
    void alLlegarCuadro(const QVideoFrame &cuadro);
    void procesarPendiente();       // convierte, MIRA LA ZONA y muestra SOLO el último cuadro

    // ---- El robot y la máquina de estados (como en el 08) ----
    void conectarRobot();           // botón Conectar / Desconectar (QWebSocket)
    void iniciarDetener();          // botón Iniciar / Detener
    void decidir();                 // UN ciclo de decisión: lo llama relojDecision cada 125 ms
    void enviarAccion(Accion accion);            // Accion -> JSON «mover» -> WebSocket
    void mostrarDecision();         // estado, color firme, orden y mensajes por segundo
    void aprender(Color color);     // botones Aprender: el rango de ese color = lo que hay en el centro

    QLineEdit *campoFuente;         // «0», «1»… = cámara; «http://…» = streaming
    QLabel *etiquetaVideo;          // aquí dibujamos cada cuadro ya procesado por OpenCV
    QPushButton *botonCamara;       // Encender / Apagar
    QPushButton *botonCambiar;      // otra cámara (frontal, trasera)
    QLabel *etiquetaEstado;         // qué cámara, tamaño del cuadro y cuadros por segundo

    QLineEdit *campoRobot;          // ws://IP:81 del robot
    QPushButton *botonRobot;        // Conectar / Desconectar
    QPushButton *botonAuto;         // Iniciar / Detener
    QLabel *etiquetaDecision;       // qué decidió la máquina de estados
    QLabel *etiquetaRangos;         // qué rango se aprendió la última vez

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

    Zona ultimaZona;                // lo que se vio en el último cuadro
    QElapsedTimer desdeCuadro;      // hace cuánto llegó el último cuadro (vigilancia del video)

    QWebSocket robot;               // comandos JSON al robot
    QTimer relojDecision;           // 125 ms: 8 decisiones por segundo
    QTimer relojLatido;             // 5 s: mantiene viva la conexión
    MaquinaColores maquina;         // la máquina de estados (fsm_p5.h)
    Accion ultimaAccion = Accion::Parar;   // lo último que se decidió, para mostrarlo
    QString ultimoMensaje;          // último JSON enviado (un «parar» no se repite)
    QElapsedTimer relojMensajes;    // para medir mensajes por segundo
    int mensajes = 0;
    double mps = 0;
};
#endif // MAINWINDOW_H
