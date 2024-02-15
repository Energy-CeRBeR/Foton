#include <iostream>
#include <fstream>
#include <vector>


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Enter the path to the file";  // Если введённый путь некорректен/отсутствует
        return 1;
    }

    std::ifstream input_file(argv[1], std::ios::binary);
    std::ofstream output_file(argv[2], std::ios::binary);

    if (!(input_file.is_open() && output_file.is_open())) {
        std::cout << "Can't open one of the files";
        input_file.close();  // Если не удаётся получить доступ к файлу 
        output_file.close();  // или указан неверный путь
        return 1;
    }

    char header[54];
    input_file.read(header, 54); // Чтение служебной информации из исходного файла

    if (header[0] != 'B' || header[1] != 'M') {
        std::cout << "The source file is not a bmp format file!" << std::endl;
        input_file.close();  // Проверка расширения исходного файла
        output_file.close();
        return 1;
    }

    int width = *(int*)&header[18]; // Получение значений длины и ширины файлов
    int height = *(int*)&header[22];
    int pixels_offset = *(int*)&header[10]; // Получение данных о смещении
    int row_size = ((((width * 24) + 31) & ~31) >> 3); // Вычисление шага
    std::vector<char> row(row_size);
    char* temp_info = new char[pixels_offset - 54];
    input_file.read(temp_info, pixels_offset - 54);
    output_file.write(header, 54); // Запись служебной информации заголовка в новый файл
    output_file.write(temp_info, pixels_offset - 54); // Запись информации с учётом смещения пикселей
    delete[] temp_info;

    for (int i = 0; i <= height; i++) {
        input_file.read(row.data(), row_size); // Чтение данных из исходного файла
        for (int j = 0; j <= width / 2; j++) {
            for (int k = 0; k < 3; k++) {
                std::swap(row[j * 3 + k], row[(width - 1 - j) * 3 + k]); // Переворачиваем изображение
            }
        }
        output_file.write(row.data(), row_size); // Запись пикселей в новый файл
    }

    std::cout << "Vertical reflection has been successfully completed!" << std::endl;
    input_file.close();
    output_file.close();

    return 0;
}
