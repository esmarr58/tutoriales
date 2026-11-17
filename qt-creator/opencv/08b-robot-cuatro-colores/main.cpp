#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Opcional, para probar desde la terminal sin tocar botones (lo usa pruebas/prueba_escritorio.py):
    //   robot_colores --fuente http://127.0.0.1:8772/stream.mjpg --robot ws://127.0.0.1:8771 --auto
    QCommandLineParser opciones;
    opciones.addHelpOption();
    opciones.addOption({"fuente", "Número de cámara o URL del stream; se enciende al abrir.", "fuente"});
    opciones.addOption({"robot", "URL del robot (ws://IP:PUERTO); se conecta al abrir.", "url"});
    opciones.addOption({"auto", "Pulsa Iniciar 3 s después de abrir."});
    opciones.addOption({"aprender", "Pulsa el botón Aprender de ese color a los 2.5 s.", "color"});
    opciones.process(a);

    MainWindow w;
    w.show();
    if (opciones.isSet("fuente") || opciones.isSet("robot"))
        w.arrancar(opciones.value("fuente"), opciones.value("robot"), opciones.isSet("auto"),
                   opciones.value("aprender"));
    return QApplication::exec();
}
