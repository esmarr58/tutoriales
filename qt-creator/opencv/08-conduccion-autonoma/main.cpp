#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Opcional, para probar desde la terminal sin tocar botones:
    //   camara_opencv --fuente http://127.0.0.1:8080/stream.mjpg
    //                 --robot ws://127.0.0.1:8081 --auto
    QCommandLineParser opciones;
    opciones.addHelpOption();
    opciones.addOption({"fuente", "Número de cámara o URL del stream; se enciende al abrir.",
                        "fuente"});
    opciones.addOption({"robot", "URL del robot (ws://IP:PUERTO); se conecta al abrir.",
                        "url"});
    opciones.addOption({"auto", "Inicia la conducción autónoma 3 s después de abrir."});
    opciones.process(a);

    MainWindow w;
    w.show();
    if (opciones.isSet("fuente") || opciones.isSet("robot"))
        w.arrancar(opciones.value("fuente"), opciones.value("robot"), opciones.isSet("auto"));
    return QApplication::exec();
}
