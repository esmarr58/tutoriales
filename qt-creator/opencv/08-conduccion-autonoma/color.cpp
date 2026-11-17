#include "color.h"

const std::vector<RangoHsv> &rangosDe(Color color)
{
    //                                            hMin hMax sMin vMin
    static const std::vector<RangoHsv> rojo     = {{0, 10, 120, 100}, {170, 179, 120, 100}};
    static const std::vector<RangoHsv> verde    = {{50, 80, 150, 100}};
    static const std::vector<RangoHsv> azul     = {{100, 125, 120, 100}};
    // Amarillo con S y V altas: el pasto (H cerca de 38, S y V cerca de 150) no debe entrar.
    static const std::vector<RangoHsv> amarillo = {{20, 34, 180, 180}};
    static const std::vector<RangoHsv> ninguno;
    switch (color) {
    case Color::Rojo: return rojo;
    case Color::Verde: return verde;
    case Color::Azul: return azul;
    case Color::Amarillo: return amarillo;
    case Color::Ninguno: break;
    }
    return ninguno;
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

bool esDisco(const Mancha &m, const ParametrosDisco &p)
{
    if (m.area < p.areaMin || m.ancho <= 0 || m.alto <= 0)
        return false;
    const double aspecto = static_cast<double>(m.ancho) / m.alto;
    const double llenado = static_cast<double>(m.area) / (m.ancho * m.alto);
    return aspecto >= p.aspectoMin && aspecto <= p.aspectoMax && llenado >= p.llenadoMin;
}

Color colorDominante(int areaRojo, int areaVerde, int areaAzul, int areaAmarillo, int areaMin)
{
    if (areaRojo >= areaMin)
        return Color::Rojo;   // el rojo manda: ante la duda, alto
    Color mejor = Color::Verde;
    int area = areaVerde;
    if (areaAzul > area) { mejor = Color::Azul; area = areaAzul; }
    if (areaAmarillo > area) { mejor = Color::Amarillo; area = areaAmarillo; }
    return area >= areaMin ? mejor : Color::Ninguno;
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
