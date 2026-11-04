#include "puente.h"

cv::Mat aMat(const QImage &imagen)
{
    // 1) Pedimos a Qt los píxeles en un formato fijo: 3 bytes por píxel.
    QImage rgb = imagen.convertToFormat(QImage::Format_RGB888);

    // 2) Envolvemos esos bytes en un cv::Mat SIN copiarlos.
    //    bytesPerLine() es lo que ocupa cada fila en memoria (Qt la alinea a 4 bytes).
    cv::Mat envoltura(rgb.height(), rgb.width(), CV_8UC3,
                      const_cast<uchar *>(rgb.constBits()), rgb.bytesPerLine());

    // 3) clone() copia los datos: 'rgb' se destruye al salir de esta función.
    return envoltura.clone();
}

QImage aQImage(const cv::Mat &mat)
{
    // Envolvemos los bytes del Mat y copy() le da a la QImage sus propios píxeles.
    QImage envoltura(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step),
                     QImage::Format_RGB888);
    return envoltura.copy();
}
