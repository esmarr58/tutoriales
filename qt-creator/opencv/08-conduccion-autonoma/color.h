#ifndef COLOR_H
#define COLOR_H

#include <vector>

// Lógica pura (sin Qt y sin OpenCV): los cuatro colores de las paletas y la prueba «¿es un disco?».

enum class Color { Ninguno, Rojo, Verde, Azul, Amarillo };

// Un rango en HSV de OpenCV: H de 0 a 179, S y V de 0 a 255. El máximo de S y V siempre es 255.
struct RangoHsv {
    int hMin, hMax;   // tono
    int sMin;         // saturación mínima: descarta blancos y grises
    int vMin;         // brillo mínimo: descarta sombras
};

// Rangos de cada color, calibrados con cuadros reales del mapa Circuito.
// El rojo devuelve DOS rangos: su tono cruza el 0 (está en 0-10 y en 170-179).
const std::vector<RangoHsv> &rangosDe(Color color);

// Clasifica UN píxel HSV. Sirve para probar los rangos a mano y en las pruebas.
Color clasificarHsv(int h, int s, int v);

// Una mancha de un color: cuántos píxeles tiene y la caja que la encierra.
struct Mancha {
    int area = 0;    // píxeles de la mancha (los huecos no cuentan)
    int ancho = 0;   // caja envolvente
    int alto = 0;
};

struct ParametrosDisco {
    int areaMin = 300;          // px: más chica, está lejos o es ruido
    double aspectoMin = 0.70;   // ancho / alto: un disco de frente da cerca de 1
    double aspectoMax = 1.40;
    double llenadoMin = 0.60;   // área / (ancho x alto): un disco da pi/4 = 0.785; un anillo, menos
};

// La trampa del rojo: no todo lo rojo es una paleta. Filtra por tamaño y por forma.
bool esDisco(const Mancha &m, const ParametrosDisco &p);

// Con el área del mejor disco de cada color: el rojo manda; si no, el más grande.
Color colorDominante(int areaRojo, int areaVerde, int areaAzul, int areaAmarillo, int areaMin);

const char *nombre(Color color);

#endif // COLOR_H
