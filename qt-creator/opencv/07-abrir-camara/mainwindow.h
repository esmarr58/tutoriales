#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCamera>
#include <QCheckBox>
#include <QElapsedTimer>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMediaCaptureSession>
#include <QMediaPlayer>
#include <QPushButton>
#include <QVideoFrame>
#include <QVideoSink>
#include <opencv2/core.hpp>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *evento) override;

private:
    void construirInterfaz();
    void encenderApagar();          // botón principal: lee la fuente (número o URL)
    void pedirPermiso();            // Android pregunta al usuario; el escritorio ya lo concede
    void abrirCamara();             // crea la QCamera elegida y la conecta a la sesión
    void abrirStream(const QUrl &direccion);  // video por red: http://…, rtsp://…
    void apagarCamara();            // apaga la fuente que esté encendida (cámara o stream)
    void cambiarCamara();           // pasa a la siguiente cámara de la lista
    void alLlegarCuadro(const QVideoFrame &cuadro);
    void procesarPendiente();       // convierte y muestra SOLO el último cuadro

    QLineEdit *campoFuente;         // «0», «1»… = cámara; «http://…» = streaming
    QLabel *etiquetaVideo;          // aquí dibujamos cada cuadro ya procesado por OpenCV
    QPushButton *botonCamara;       // Encender / Apagar
    QPushButton *botonCambiar;      // otra cámara (frontal, trasera, otra webcam)
    QCheckBox *casillaGris;         // OpenCV: cvtColor a escala de grises
    QLabel *etiquetaEstado;         // qué cámara, tamaño del cuadro y cuadros por segundo

    QMediaCaptureSession sesion;    // une la cámara con quien recibe los cuadros
    QVideoSink sumidero;            // recibe cada cuadro (QVideoFrame)
    QCamera *camara = nullptr;
    int indiceCamara = 0;           // posición en QMediaDevices::videoInputs()
    QMediaPlayer *reproductor = nullptr;  // reproduce el stream y entrega sus cuadros
    QString nombreFuente;           // para la línea de estado

    QVideoFrame pendiente;          // último cuadro recibido, aún sin convertir
    bool programado = false;        // ya hay una conversión esperando en la cola de eventos
    cv::Mat resultado;              // último cuadro en OpenCV (color BGR o gris)
    QElapsedTimer reloj;            // para medir cuadros por segundo
    int cuadros = 0;                // cuadros mostrados desde la última medición
    double cps = 0;                 // cuadros por segundo medidos
};
#endif // MAINWINDOW_H
