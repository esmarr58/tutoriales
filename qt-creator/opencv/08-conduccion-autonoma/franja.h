#ifndef FRANJA_H
#define FRANJA_H

#include <cstdint>
#include <vector>

// Lógica pura (sin Qt y sin OpenCV): busca las dos rayas del carril en el PERFIL de una franja.
//
// El perfil es UNA fila de números de 0 a 255: el gris promedio de cada columna de la franja
// (lo calcula percepcion.cpp con cv::reduce). Para 640 columnas son 640 números.

enum class Lineas { Ninguna, SoloIzquierda, SoloDerecha, Dos };

struct ParametrosFranja {
    int umbral = 170;            // gris a partir del cual una columna cuenta como «raya»
    bool rayasOscuras = false;   // true: tiras negras sobre piso claro (pista del robot real)
    double anchoMin = 0.25;      // carril más angosto que se acepta (fracción del ancho de la imagen)
    double anchoMax = 0.80;      // carril más ancho que se acepta
};

struct Carril {
    Lineas lineas = Lineas::Ninguna;
    int izquierda = -1;   // columna donde empieza la raya izquierda, vista desde el centro (-1: no se vio)
    int derecha = -1;     // columna donde empieza la raya derecha, vista desde el centro (-1: no se vio)
    int centro = 0;       // columna del centro del carril
    int error = 0;        // centro del carril - centro de la imagen (px). Negativo: el carril queda a la izquierda
    int ancho = 0;        // derecha - izquierda (px)
};

// anchoReferencia: ancho del carril (px) medido la última vez que se vieron las DOS rayas.
// Con una sola raya, el centro se estima con ese ancho.
Carril analizarFranja(const std::vector<std::uint8_t> &perfil, const ParametrosFranja &p,
                      int anchoReferencia);

const char *nombre(Lineas lineas);

#endif // FRANJA_H
