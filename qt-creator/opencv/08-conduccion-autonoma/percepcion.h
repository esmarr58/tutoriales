#ifndef PERCEPCION_H
#define PERCEPCION_H

#include "color.h"
#include "franja.h"

#include <opencv2/core.hpp>

// Aquí está TODO el OpenCV del programa: de un cuadro (cv::Mat BGR) saca lo que la máquina de
// estados necesita. Las reglas (dónde está la raya, si la mancha es un disco) viven en franja.cpp
// y color.cpp, que no usan OpenCV y por eso se pueden probar sin cámara.

struct ParametrosVision {
    double filaFranja = 0.75;     // la franja empieza al 75 % del alto de la imagen
    int filasFranja = 8;          // y mide 8 filas
    ParametrosFranja franja;      // umbral, rayas oscuras, anchos aceptados
    double zonaColorArriba = 0.30;   // las paletas se buscan entre el 30 %
    double zonaColorAbajo = 0.55;    // y el 55 % del alto
    ParametrosDisco disco;
    double anchoInicial = 0.47;   // ancho del carril antes de verlo (fracción del ancho de la imagen)
};

// Lo que se vio en un cuadro.
struct Vista {
    Carril carril;
    Color color = Color::Ninguno;   // paleta de este cuadro (sin antirrebote)
    cv::Rect cajaDisco;             // dónde está la paleta, para dibujarla
    int areaDisco = 0;
    cv::Rect franja;                // dónde está la franja, para dibujarla
    cv::Rect zonaColor;
};

// Gris promedio de cada columna de la franja: una fila de `ancho` números de 0 a 255.
std::vector<std::uint8_t> perfilDeFranja(const cv::Mat &bgr, const ParametrosVision &p);

// Máscara de un color (blanco = píxel de ese color). El rojo une sus dos rangos.
cv::Mat mascaraDe(const cv::Mat &hsv, Color color);

// El cuadro completo: franja + paletas. anchoReferencia se actualiza cuando se ven las dos rayas.
Vista percibir(const cv::Mat &bgr, const ParametrosVision &p, int &anchoReferencia);

// Dibuja sobre el cuadro lo que el programa vio (franja, rayas, centro, paleta).
void dibujarVista(cv::Mat &bgr, const Vista &vista);

#endif // PERCEPCION_H
