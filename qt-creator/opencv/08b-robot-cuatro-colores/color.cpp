#include "color.h"

namespace {
// Punto de partida para hojas de papel mate. Son rangos amplios y NO se midieron en un teléfono:
// si tu hoja no se reconoce, usa el botón «Aprender» de ese color (no hace falta recompilar).
//                                    hMin hMax sMin vMin
std::vector<RangoHsv> rojo     = {{0, 12, 90, 70}, {168, 179, 90, 70}};
std::vector<RangoHsv> verde    = {{35, 90, 70, 60}};
std::vector<RangoHsv> azul     = {{95, 130, 80, 60}};
std::vector<RangoHsv> amarillo = {{18, 34, 90, 100}};
const std::vector<RangoHsv> ninguno;

std::vector<RangoHsv> *tabla(Color color)
{
    switch (color) {
    case Color::Rojo: return &rojo;
    case Color::Verde: return &verde;
    case Color::Azul: return &azul;
    case Color::Amarillo: return &amarillo;
    case Color::Ninguno: break;
    }
    return nullptr;
}
}

const std::vector<RangoHsv> &rangosDe(Color color)
{
    const std::vector<RangoHsv> *rangos = tabla(color);
    return rangos ? *rangos : ninguno;
}

void fijarRangos(Color color, const std::vector<RangoHsv> &rangos)
{
    if (std::vector<RangoHsv> *destino = tabla(color))
        *destino = rangos;
}

std::vector<RangoHsv> rangosAprendidos(int h, int s, int v)
{
    const int margen = 12;                          // tono: 12 a cada lado
    const int sMin = s - 70 < 50 ? 50 : s - 70;     // nunca por debajo de 50: sería gris
    const int vMin = v - 90 < 50 ? 50 : v - 90;
    const int desde = h - margen;
    const int hasta = h + margen;
    if (desde < 0)                                  // el tono cruza el 0 por abajo: dos rangos
        return {{0, hasta, sMin, vMin}, {180 + desde, 179, sMin, vMin}};
    if (hasta > 179)                                // cruza por arriba
        return {{desde, 179, sMin, vMin}, {0, hasta - 180, sMin, vMin}};
    return {{desde, hasta, sMin, vMin}};
}

Color clasificarHsv(int h, int s, int v)
{
    for (Color c : {Color::Rojo, Color::Verde, Color::Azul, Color::Amarillo}) {
        for (const RangoHsv &r : rangosDe(c)) {
            if (h >= r.hMin && h <= r.hMax && s >= r.sMin && v >= r.vMin)
                return c;
        }
    }
    return Color::Ninguno;
}

const char *nombre(Color color)
{
    switch (color) {
    case Color::Ninguno: return "ninguno";
    case Color::Rojo: return "rojo";
    case Color::Verde: return "verde";
    case Color::Azul: return "azul";
    case Color::Amarillo: return "amarillo";
    }
    return "?";
}
