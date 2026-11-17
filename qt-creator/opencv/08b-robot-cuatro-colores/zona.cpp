#include "zona.h"

#include <opencv2/imgproc.hpp>

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

Zona mirarZona(const cv::Mat &bgr, int minimo)
{
    Zona zona;
    if (bgr.empty() || bgr.channels() != 3)
        return zona;

    // La zona es el tercio central: una hoja a 20-30 cm la llena.
    zona.caja = cv::Rect(bgr.cols / 3, bgr.rows / 3, bgr.cols / 3, bgr.rows / 3);
    cv::Mat hsv;
    cv::cvtColor(bgr(zona.caja), hsv, cv::COLOR_BGR2HSV);

    // HSV del píxel central. No se usa el promedio: el promedio del tono del rojo
    // (que vive cerca de 0 y cerca de 179) no significa nada.
    const cv::Vec3b centro = hsv.at<cv::Vec3b>(hsv.rows / 2, hsv.cols / 2);
    zona.h = centro[0];
    zona.s = centro[1];
    zona.v = centro[2];

    // El color que más píxeles tiene, si llena al menos `minimo` por ciento de la zona.
    const int total = zona.caja.area();
    int mayor = total * minimo / 100 - 1;
    for (Color color : {Color::Rojo, Color::Verde, Color::Azul, Color::Amarillo}) {
        const int pixeles = cv::countNonZero(mascaraDe(hsv, color));
        if (pixeles > mayor) {
            mayor = pixeles;
            zona.color = color;
            zona.porcentaje = pixeles * 100 / total;
        }
    }
    return zona;
}

void dibujarZona(cv::Mat &bgr, const Zona &zona, const char *arriba, const char *abajo)
{
    if (bgr.empty())
        return;
    const cv::Scalar blanco(255, 255, 255), negro(0, 0, 0);
    cv::rectangle(bgr, zona.caja, blanco, 3);
    const cv::Point centro(zona.caja.x + zona.caja.width / 2, zona.caja.y + zona.caja.height / 2);
    cv::drawMarker(bgr, centro, blanco, cv::MARKER_CROSS, 24, 2);          // el píxel que se mide
    cv::rectangle(bgr, cv::Rect(0, 0, bgr.cols, 40), negro, cv::FILLED);   // fondo del texto
    cv::rectangle(bgr, cv::Rect(0, bgr.rows - 40, bgr.cols, 40), negro, cv::FILLED);
    cv::putText(bgr, arriba, cv::Point(8, 28), cv::FONT_HERSHEY_SIMPLEX, 0.6, blanco, 2);
    cv::putText(bgr, abajo, cv::Point(8, bgr.rows - 12), cv::FONT_HERSHEY_SIMPLEX, 0.6, blanco, 2);
}
