#ifndef ZONA_H
#define ZONA_H

#include "color.h"

#include <opencv2/core.hpp>

// Aquí está TODO el OpenCV del programa: de un cuadro (cv::Mat BGR) dice de qué color es el centro.

struct Zona {
    Color color = Color::Ninguno;   // color que llena la zona en ESTE cuadro (sin antirrebote)
    int porcentaje = 0;             // cuánto de la zona llena ese color (0 a 100)
    int h = 0, s = 0, v = 0;        // HSV del píxel CENTRAL: para calibrar y para «Aprender»
    cv::Rect caja;                  // la zona: el tercio central de la imagen
};

// Máscara de un color (blanco = píxel de ese color). El rojo une sus dos rangos.
cv::Mat mascaraDe(const cv::Mat &hsv, Color color);

// Un color cuenta si llena al menos `minimo` por ciento de la zona.
Zona mirarZona(const cv::Mat &bgr, int minimo = 40);

// Dibuja el recuadro de la zona y dos renglones de texto (arriba y abajo). Solo letras sin acento.
void dibujarZona(cv::Mat &bgr, const Zona &zona, const char *arriba, const char *abajo);

#endif // ZONA_H
