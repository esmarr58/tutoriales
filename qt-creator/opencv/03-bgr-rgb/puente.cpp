#include "puente.h"

#include <opencv2/imgproc.hpp>

cv::Mat aMat(const QImage &imagen)
{
    // 1) Pedimos a Qt los píxeles en un formato fijo: 3 bytes por píxel (R, G, B).
    QImage rgb = imagen.convertToFormat(QImage::Format_RGB888);

    // 2) Envolvemos esos bytes en un cv::Mat SIN copiarlos.
    //    bytesPerLine() es lo que ocupa cada fila en memoria (Qt la alinea a 4 bytes).
    cv::Mat envoltura(rgb.height(), rgb.width(), CV_8UC3,
                      const_cast<uchar *>(rgb.constBits()), rgb.bytesPerLine());

    // 3) Qt guarda R, G, B y OpenCV espera B, G, R: cvtColor cambia el orden.
    //    Además escribe en memoria nueva, así que ya no hace falta clone().
    cv::Mat bgr;
    cv::cvtColor(envoltura, bgr, cv::COLOR_RGB2BGR);
    return bgr;
}

QImage aQImage(const cv::Mat &mat)
{
    // De vuelta al orden de Qt: B, G, R -> R, G, B.
    cv::Mat rgb;
    cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
    QImage envoltura(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step),
                     QImage::Format_RGB888);
    return envoltura.copy();
}
