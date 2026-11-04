#ifndef PUENTE_H
#define PUENTE_H

#include <QImage>
#include <opencv2/core.hpp>

// Puente entre Qt y OpenCV. Las dos funciones COPIAN los píxeles:
// el resultado no depende de que la imagen de origen siga viva.
cv::Mat aMat(const QImage &imagen);   // QImage  -> cv::Mat de 3 canales
QImage aQImage(const cv::Mat &mat);   // cv::Mat de 1 o 3 canales -> QImage

#endif // PUENTE_H
