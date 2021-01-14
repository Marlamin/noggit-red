#include "gaussianblur.h"

double GaussianBlur::GetNumberOnNormalDistribution(int i, int j, const int center, double sigma) const
{
    return (1.0 / (2 * M_PI * pow(sigma, 2)) * exp ( - (pow(i - center, 2) + pow(j - center, 2)) / (2 * pow(sigma, 2))));
}


void GaussianBlur::GetPixelMatrix(const QPoint &center, const QImage &image, QRgb** matrix)
{
    QSize image_size = image.size();
    for (int i = 0; i < size_; i++)
        for (int j = 0; j < size_; j++)
            matrix[i][j] = image.pixel(GetCoordinate(QPoint(i, j) + center, image_size));
}


QRgb GaussianBlur::GetNewPixelValue(QRgb **matrix)
{
    QRgb result = 0;
    double red = 0;
    double blue = 0;
    double green = 0;
    for (int i = 0; i < size_; i++)
        for (int j = 0; j < size_; j++) {
            red += qRed(matrix[i][j]) * matrix_[i][j];
            blue += qBlue(matrix[i][j]) * matrix_[i][j];
            green += qGreen(matrix[i][j]) * matrix_[i][j];
        }
    result = qRgb(red, green, blue);
    return result;
}


QPoint GaussianBlur::GetCoordinate(const QPoint &point, const QSize &image_size) {
    QPoint result(point);
    result.setX(result.x() - radius_);
    result.setY(result.y() - radius_);
    if (result.x() < 0)
        result.setX(0);
    if (result.y() < 0)
        result.setY(0);
    if (result.x() >= image_size.width())
        result.setX(image_size.width() - 1);
    if (result.y() >= image_size.height())
        result.setY(image_size.height() - 1);
    return result;
}


void GaussianBlur::NormilizeElementsBySum() {
    double sum = GetSumElements();
    for (int i = 0; i < size_; i++)
        for (int j = 0; j < size_; j++)
            matrix_[i][j] /= sum;
}


GaussianBlur::GaussianBlur(int radius, double sigma) :
            radius_ (radius),
            diviation_ (sigma),
            values_is_set_ (false)
{
    if (true == (radius_ % 2))
    {
        size_   = 2 * radius_ + 1;
        matrix_ = new double* [size_];
        for (int i = 0; i < size_; i++)
            matrix_[i] = new double [size_];
        for (int i = 0; i < size_; i++)
            for (int j = 0; j < size_; j++)
                matrix_[i][j] = GetNumberOnNormalDistribution(i, j, radius_, diviation_);
        NormilizeElementsBySum();
        values_is_set_ = true;
    }
}


GaussianBlur::GaussianBlur(const GaussianBlur &original):
            radius_(original.radius_),
            diviation_(original.diviation_)
{
    size_ = 2 * radius_ + 1;
    matrix_ = new double* [size_];

    for (int i = 0; i < size_; i++)
        matrix_[i] = new double [size_];
    for (int i = 0; i < size_; i++)
        for (int j = 0; j < size_; j++)
            matrix_[i][j] = original.matrix_[i][j];
}


GaussianBlur::~GaussianBlur()
{
    for (int i = 0; i < size_; i++)
        delete[] matrix_[i];

    delete[] matrix_;
}


double GaussianBlur::GetSumElements() const
{
    double result = 0;

    for (int i = 0; i < size_; i++)
        for (int j = 0; j < size_; j++)
            result += matrix_[i][j];

    return result;
}


QImage GaussianBlur::ApplyGaussianFilterToImage(const QImage& input)
{
    if (true == values_is_set_)
    {
        QImage output(input);
        QRgb **temp = new QRgb* [size_];
        for (int i = 0; i < size_; i++)
            temp[i] = new QRgb [size_];

        for (int i = 0; i < output.width(); i++)
            for (int j = 0; j < output.height(); j++)
            {
                GetPixelMatrix(QPoint(i, j), output, temp);
                output.setPixel(QPoint(i,j), GetNewPixelValue(temp));
            }

        for (int i = 0; i < size_; i++)
            delete[] temp[i];
        delete[] temp;

        return output;
    } else
        return input;
}


const GaussianBlur& GaussianBlur::operator = (const GaussianBlur &blur)
{
    if (this != &blur)
    {
        for (int i = 0; i < size_; i ++)
            delete[] matrix_[i];
        delete[] matrix_;

        radius_ = blur.radius_;
        diviation_ = blur.diviation_;
        size_ = blur.size_;
        matrix_ = new double* [size_];

        for (int i = 0; i < size_; i++)
            matrix_[i] = new double [size_];
        for (int i = 0; i < size_; i++)
            for (int j = 0; j < size_; j++)
                matrix_[i][j] = blur.matrix_[i][j];
    }
    return *this;
}


QTextStream &operator << (QTextStream &stream, const GaussianBlur &blur) {
    stream.setRealNumberPrecision(6);
    for (int i = 0; i < blur.size_; i++) {
        for (int j = 0; j < blur.size_; j++)
            stream << blur.matrix_[i][j] << " ";
        stream << endl;
    }
    return stream;
}
