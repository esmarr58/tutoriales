#include "franja.h"

Carril analizarFranja(const std::vector<std::uint8_t> &perfil, const ParametrosFranja &p,
                      int anchoReferencia)
{
    Carril c;
    const int ancho = static_cast<int>(perfil.size());
    const int mitad = ancho / 2;
    c.centro = mitad;
    if (ancho == 0)
        return c;

    // ¿La columna x es raya? Clara sobre asfalto oscuro, u oscura sobre piso claro.
    auto esRaya = [&](int x) {
        return p.rayasOscuras ? perfil[x] <= p.umbral : perfil[x] >= p.umbral;
    };

    // Del centro hacia afuera: la primera columna que es raya, a cada lado.
    for (int x = mitad - 1; x >= 0; --x) {
        if (esRaya(x)) { c.izquierda = x; break; }
    }
    for (int x = mitad; x < ancho; ++x) {
        if (esRaya(x)) { c.derecha = x; break; }
    }

    if (c.izquierda >= 0 && c.derecha >= 0) {
        c.ancho = c.derecha - c.izquierda;
        if (c.ancho < p.anchoMin * ancho) {
            // Demasiado angosto: no es el carril
            // (la meta a cuadros, un reflejo, una raya al centro).
            c.lineas = Lineas::Ninguna;
            return c;
        }
        if (c.ancho <= p.anchoMax * ancho) {
            c.lineas = Lineas::Dos;
            c.centro = (c.izquierda + c.derecha) / 2;
            c.error = c.centro - mitad;
            return c;
        }
        // Demasiado ancho (la pista se abre en dos): nos quedamos con la raya derecha.
        c.izquierda = -1;
    }
    if (c.izquierda >= 0) {
        c.lineas = Lineas::SoloIzquierda;
        c.ancho = anchoReferencia;
        c.centro = c.izquierda + anchoReferencia / 2;
        c.error = c.centro - mitad;
    } else if (c.derecha >= 0) {
        c.lineas = Lineas::SoloDerecha;
        c.ancho = anchoReferencia;
        c.centro = c.derecha - anchoReferencia / 2;
        c.error = c.centro - mitad;
    }
    return c;   // sin rayas: Lineas::Ninguna, error 0
}

const char *nombre(Lineas lineas)
{
    switch (lineas) {
    case Lineas::Ninguna: return "ninguna";
    case Lineas::SoloIzquierda: return "solo izquierda";
    case Lineas::SoloDerecha: return "solo derecha";
    case Lineas::Dos: return "dos";
    }
    return "?";
}
