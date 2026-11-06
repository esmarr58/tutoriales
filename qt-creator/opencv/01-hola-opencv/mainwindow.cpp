#include "mainwindow.h"

#include <QVBoxLayout>
#include <opencv2/core.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Visor OpenCV");
    auto *central = new QWidget(this);
    // Letra de 20 pt: la heredan todos los widgets y se lee en el teléfono.
    QFont letra = central->font();
    letra.setPointSize(20);
    central->setFont(letra);

    // cv::getVersionString() viene de OpenCV: si compila y se ve, OpenCV quedó enlazado.
    etiquetaVersion = new QLabel(QString("Hola, OpenCV %1")
                                     .arg(QString::fromStdString(cv::getVersionString())));
    etiquetaVersion->setAlignment(Qt::AlignCenter);
    etiquetaVersion->setWordWrap(true);

    auto *capas = new QVBoxLayout(central);
    capas->addWidget(etiquetaVersion);
    setCentralWidget(central);
    resize(900, 600);
}

MainWindow::~MainWindow() = default;
