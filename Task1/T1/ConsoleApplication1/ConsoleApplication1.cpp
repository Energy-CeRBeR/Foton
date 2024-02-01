#include <iostream>
#include <fstream>
#include <vector>

// Функция для чтения изображения из файла
std::vector<std::vector<float>> readImage(const std::string& filename, int& width, int& height)
{
    std::ifstream file(filename, std::ios::binary);
    std::vector<std::vector<float>> image;

    if (file)
    {
        // Читаем заголовок BMP файла
        char header[54];
        file.read(header, 54);

        // Получаем ширину и высоту изображения
        width = *(int*)&header[18];
        height = *(int*)&header[22];

        // Размер изображения в пикселях
        int imageSize = width * height;

        // Создаем буфер для хранения пикселей
        image.resize(height);
        for (int i = 0; i < height; ++i)
            image[i].resize(width);

        // Читаем пиксели изображения
        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
            {
                unsigned char pixel[3];
                file.read((char*)pixel, 3);

                // Преобразуем цвета пикселей в диапазон от 0 до 1
                image[i][j] = pixel[0] / 255.0;
            }
        }

        file.close();
    }
    else
    {
        std::cout << "Failed to open image file: " << filename << std::endl;
    }

    return image;
}

// Функция для записи изображения в файл
void saveImage(const std::string& filename, const std::vector<std::vector<float>>& image, int width, int height)
{
    std::ofstream file(filename, std::ios::binary);

    if (file)
    {
        // Заголовок BMP файла
        char header[54] = {
            'B', 'M',         // Magic number
            0, 0, 0, 0,       // File size
            0, 0,            // Reserved
            0, 0,            // Reserved
            54, 0, 0, 0,     // Offset to pixel data
            40, 0, 0, 0,     // Header size
            0, 0, 0, 0,      // Image width
            0, 0, 0, 0,      // Image height
            1, 0,            // Planes
            24, 0,           // Bits per pixel
            0, 0, 0, 0,      // Compression
            0, 0, 0, 0,      // Image size
            0, 0, 0, 0,      // X pixels per meter
            0, 0, 0, 0,      // Y pixels per meter
            0, 0, 0, 0,      // Number of colors
            0, 0, 0, 0       // Important colors
        };

        // Заполняем заголовок BMP файла
        *(int*)&header[2] = 54 + width * height * 3;  // File size
        *(int*)&header[18] = width;                   // Image width
        *(int*)&header[22] = height;                  // Image height
        *(int*)&header[34] = width * height * 3;      // Image size

        // Записываем заголовок в файл
        file.write(header, 54);

        // Записываем пиксели изображения
        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
            {
                // Преобразуем значения пикселей обратно в диапазон от 0 до 255
                unsigned char pixel[3] = {
                    static_cast<unsigned char>(image[i][j] * 255),
                    static_cast<unsigned char>(image[i][j] * 255),
                    static_cast