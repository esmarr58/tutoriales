#include "percepcion.h"

#include <opencv2/imgproc.hpp>

std::vector<std::uint8_t> perfilDeFranja(const cv::Mat &bgr, const ParametrosVision &p)
{
    // 1) Recorte: solo las filas de la franja. No copia píxeles: es una «ventana» al cuadro.
    const int y = static_cast<int>(bgr.rows * p.filaFranja);
    const int filas = std::min(p.filasFranja, bgr.rows - y);
    const cv::Mat recorte = bgr(cv::Rect(0, y, bgr.cols, filas));

    // 2) A gris: un número por píxel (video 5).
    cv::Mat gris;
    cv::cvtColor(recorte, gris, cv::COLOR_BGR2GRAY);

    // 3) Las 8 filas se vuelven UNA: el promedio de cada columna. Queda 1 x ancho.
    cv::Mat promedio;
    cv::reduce(gris, promedio, 0, cv::REDUCE_AVG, CV_8U);

    return std::vector<std::uint8_t>(promedio.ptr<uchar>(0), promedio.ptr<uchar>(0) + promedio.cols);
}

cv::Mat mascaraDe(const cv::Mat &hsv, Color color)
{
    cv::Mat mascara = cv::Mat::zeros(hsv.size(), CV_8UC1);
    for (const RangoHsv &r : rangosDe(color)) {
        cv::Mat parte;
        cv::inRange(hsv, cv::Scalar(r.hMin, r.sMin, r.vMin), cv::Scalar(r.hMax, 255, 255), parte);
        cv::bitwise_or(mascara, parte, mascara);   // el rojo: rango bajo O rango alto
    }
    return mascara;
}

Vista percibir(const cv::Mat &bgr, const ParametrosVision &p, int &anchoReferencia)
{
    Vista vista;
    if (bgr.empty() || bgr.channels() != 3)
        return vista;

    // ---- Percepción 1: las dos rayas en la franja ----
    const int yFranja = static_cast<int>(bgr.rows * p.filaFranja);
    vista.franja = cv::Rect(0, yFranja, bgr.cols, std::min(p.filasFranja, bgr.rows - yFranja));
    if (anchoReferencia <= 0)
        anchoReferencia = static_cast<int>(p.anchoInicial * bgr.cols);
    vista.carril = analizarFranja(perfilDeFranja(bgr, p), p.franja, anchoReferencia);
    if (vista.carril.lineas == Lineas::Dos)
        anchoReferencia = vista.carril.ancho;   // se recuerda para cuando solo se vea una raya

    // ---- Percepción 2: las paletas de color en su zona ----
    const int y0 = static_cast<int>(bgr.rows * p.zonaColorArriba);
    const int y1 = static_cast<int>(bgr.rows * p.zonaColorAbajo);
    vista.zonaColor = cv::Rect(0, y0, bgr.cols, y1 - y0);
    cv::Mat hsv;
    cv::cvtColor(bgr(vista.zonaColor), hsv, cv::COLOR_BGR2HSV);

    int areas[5] = {0, 0, 0, 0, 0};       // índice = Color
    cv::Rect cajas[5];
    for (Color color : {Color::Rojo, Color::Verde, Color::Azul, Color::Amarillo}) {
        // Cada mancha conectada de la máscara, con su área y su caja: una fila de `tabla`.
        cv::Mat etiquetas, tabla, centros;
        const int n = cv::connectedComponentsWithStats(mascaraDe(hsv, color), etiquetas, tabla, centros, 8);
        for (int i = 1; i < n; ++i) {     // la etiqueta 0 es el fondo
            Mancha m;
            m.area = tabla.at<int>(i, cv::CC_STAT_AREA);
            m.ancho = tabla.at<int>(i, cv::CC_STAT_WIDTH);
            m.alto = tabla.at<int>(i, cv::CC_STAT_HEIGHT);
            const int indice = static_cast<int>(color);
            if (esDisco(m, p.disco) && m.area > areas[indice]) {
                areas[indice] = m.area;
                cajas[indice] = cv::Rect(tabla.at<int>(i, cv::CC_STAT_LEFT),
                                         tabla.at<int>(i, cv::CC_STAT_TOP) + y0, m.ancho, m.alto);
            }
        }
    }
    vista.color = colorDominante(areas[1], areas[2], areas[3], areas[4], p.disco.areaMin);
    vista.areaDisco = areas[static_cast<int>(vista.color)];
    vista.cajaDisco = cajas[static_cast<int>(vista.color)];
    return vista;
}

void dibujarVista(cv::Mat &bgr, const Vista &vista)
{
    if (bgr.empty())
        return;
    const cv::Scalar amarillo(0, 255, 255), cian(255, 255, 0), magenta(255, 0, 255), blanco(255, 255, 255);
    cv::rectangle(bgr, vista.zonaColor, cian, 1);                  // zona de las paletas
    cv::rectangle(bgr, vista.franja, amarillo, 2);                 // la franja
    const int y = vista.franja.y + vista.franja.height / 2;
    cv::line(bgr, cv::Point(bgr.cols / 2, y - 22), cv::Point(bgr.cols / 2, y + 22), blanco, 2);   // centro de la imagen
    if (vista.carril.izquierda >= 0)
        cv::circle(bgr, cv::Point(vista.carril.izquierda, y), 8, magenta, cv::FILLED);
    if (vista.carril.derecha >= 0)
        cv::circle(bgr, cv::Point(vista.carril.derecha, y), 8, magenta, cv::FILLED);
    if (vista.carril.lineas != Lineas::Ninguna)                    // centro del carril
        cv::drawMarker(bgr, cv::Point(vista.carril.centro, y), cv::Scalar(0, 255, 0), cv::MARKER_CROSS, 30, 3);
    if (vista.color != Color::Ninguno)
        cv::rectangle(bgr, vista.cajaDisco, magenta, 3);
}
