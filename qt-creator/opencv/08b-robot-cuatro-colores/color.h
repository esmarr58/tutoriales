#ifndef COLOR_H
#define COLOR_H

#include <vector>

// Lógica pura (sin Qt y sin OpenCV): los cuatro colores de las hojas y sus rangos HSV.

enum class Color { Ninguno, Rojo, Verde, Azul, Amarillo };

// Un rango en HSV de OpenCV: H de 0 a 179, S y V de 0 a 255. El máximo de S y V siempre es 255.
struct RangoHsv {
    int hMin, hMax;   // tono
    int sMin;         // saturación mínima: descarta blancos y grises
    int vMin;         // brillo mínimo: descarta sombras
};

// Rangos vigentes de cada color. Arrancan con un punto de partida amplio, pensado para hojas de
// papel (NO medido en un teléfono), y cambian con fijarRangos() al pulsar «Aprender».
// El rojo usa DOS rangos: su tono cruza el 0 (está cerca de 0 y cerca de 179).
const std::vector<RangoHsv> &rangosDe(Color color);
void fijarRangos(Color color, const std::vector<RangoHsv> &rangos);

// «Aprender»: con la hoja llenando el recuadro, el píxel central mide (h, s, v).
// Devuelve el rango alrededor de ese tono (dos rangos si cruza el 0).
std::vector<RangoHsv> rangosAprendidos(int h, int s, int v);

// Clasifica UN píxel HSV con los rangos vigentes.
Color clasificarHsv(int h, int s, int v);

const char *nombre(Color color);

#endif // COLOR_H
