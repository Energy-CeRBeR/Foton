#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <memory>

#pragma pack(2)

struct BITMAPFILEHEADER
{
    unsigned short bfType;
    unsigned int bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
};

struct BITMAPINFOHEADER
{
    unsigned int biSize;
    int biWidth;
    int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
};

int pixelsOffset;
int width;
int height;
int new_width;
int new_height;
int colorsChannels;
int row_size;
int new_row_size;

// Функция для записи заголовка
void writeHeader(std::ifstream &input_file, std::ofstream &output_file, BITMAPFILEHEADER &fileHeader, BITMAPINFOHEADER &fileInfoHeader)
{
    output_file.write((char *)&fileHeader, sizeof(fileHeader));
    output_file.write((char *)&fileInfoHeader, sizeof(fileInfoHeader));

    char *temp_info = new char[pixelsOffset - 54];
    input_file.seekg(54);
    input_file.read(temp_info, pixelsOffset - 54);
    output_file.write(temp_info, pixelsOffset - 54);

    delete[] temp_info;
}

// Функция для чтения и записи вещественных данных из текстовых файлов для компонентов
std::vector<std::vector<double>> get_data_from_txt(std::ifstream &file)
{
    std::vector<std::vector<double>> data;
    std::string line;

    while (getline(file, line))
    {
        std::vector<double> row;
        std::stringstream ss(line);
        double value;
        while (ss >> value)
        {
            row.push_back(value);
        }
        data.push_back(row);
    }

    file.close();

    return data;
}

void write_inverse_haar_header(std::ifstream &input_file, std::ofstream &output_file)
{
    BITMAPFILEHEADER fileHeader;
    input_file.read((char *)&fileHeader, sizeof(fileHeader));

    BITMAPINFOHEADER fileInfoHeader;
    input_file.read((char *)&fileInfoHeader, sizeof(fileInfoHeader));

    pixelsOffset = fileHeader.bfOffBits;
    colorsChannels = fileInfoHeader.biBitCount / 8;
    width = fileInfoHeader.biWidth;
    height = fileInfoHeader.biHeight;
    new_row_size = std::floor((fileInfoHeader.biBitCount * 2 * width + 31) / 32) * 4;

    fileInfoHeader.biWidth *= 2;
    fileInfoHeader.biHeight *= 2;

    writeHeader(input_file, output_file, fileHeader, fileInfoHeader);
}

// Запись данных из текстовых файлов компонентов в соответствующие вектора
std::vector<std::vector<std::vector<std::vector<double>>>> read_txt_data(const std::string &TXT_DATA_DIR_PATH)
{
    std::vector<std::vector<std::vector<double>>> LL_data;
    std::vector<std::vector<std::vector<double>>> LH_data;
    std::vector<std::vector<std::vector<double>>> HL_data;
    std::vector<std::vector<std::vector<double>>> HH_data;

    // Чтение данных из текстовых файлов
    for (int k = 1; k < colorsChannels + 1; ++k)
    {
        // std::cout << "Getting " << k << "th image channel components: " << std::endl
        //           << std::endl;

        // std::cout << "Getting LL_component data ..." << std::endl;
        std::ifstream LL_txt_file(TXT_DATA_DIR_PATH + "LL_data_" + std::to_string(k) + ".txt");
        LL_data.push_back(get_data_from_txt(LL_txt_file));
        // std::cout << "LL_component data received!" << std::endl
        //           << std::endl;

        // std::cout << "Getting LH_component data ..." << std::endl;
        std::ifstream LH_txt_file(TXT_DATA_DIR_PATH + "LH_data_" + std::to_string(k) + ".txt");
        LH_data.push_back(get_data_from_txt(LH_txt_file));
        // std::cout << "LH_component data received!" << std::endl
        //           << std::endl;

        // std::cout << "Getting HL_component data ..." << std::endl;
        std::ifstream HL_txt_file(TXT_DATA_DIR_PATH + "HL_data_" + std::to_string(k) + ".txt");
        HL_data.push_back(get_data_from_txt(HL_txt_file));
        // std::cout << "HL_component data received!" << std::endl
        //           << std::endl;

        // std::cout << "Getting HH_component data ..." << std::endl;
        std::ifstream HH_txt_file(TXT_DATA_DIR_PATH + "HH_data_" + std::to_string(k) + ".txt");
        HH_data.push_back(get_data_from_txt(HH_txt_file));
        // std::cout << "HH_component data received!" << std::endl
        //           << std::endl;
    }

    // std::cout << "All data has been uploaded successfully!" << std::endl
    //           << std::endl;

    return {LL_data, LH_data, HL_data, HH_data};
}

// Обратное преобразование Хаара
void inverse_haar(const std::string &INPUT_PATH, const std::string &OUTPUT_PATH, const std::string &TXT_DATA_DIR_PATH)
{
    std::ifstream haar_component(INPUT_PATH, std::ios::binary);
    if (!(haar_component.is_open()))
    {
        std::cout << "Can't open input_file";
        haar_component.close();
        exit(1);
    }

    std::ofstream inverse_result(OUTPUT_PATH, std::ios::binary);

    write_inverse_haar_header(haar_component, inverse_result);

    std::vector<char> pixels(new_row_size * 2 * height + 2 * width * colorsChannels);

    std::vector<std::vector<std::vector<std::vector<double>>>> data_from_txt = read_txt_data(TXT_DATA_DIR_PATH);

    std::vector<std::vector<std::vector<double>>> LL_data = data_from_txt[0];
    std::vector<std::vector<std::vector<double>>> LH_data = data_from_txt[1];
    std::vector<std::vector<std::vector<double>>> HL_data = data_from_txt[2];
    std::vector<std::vector<std::vector<double>>> HH_data = data_from_txt[3];

    // Применение алгоритма
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            for (int k = 0; k < colorsChannels; k++)
            {
                double x = LL_data[k][i][j];
                double y = LH_data[k][i][j];
                double z = HL_data[k][i][j];
                double w = HH_data[k][i][j];

                double a = x - (y + z + w) / 4;
                double b = a + y;
                double c = a + z;
                double d = a + w;

                pixels[2 * i * new_row_size + 2 * j * colorsChannels + k] = static_cast<char>(round(a));
                pixels[2 * i * new_row_size + (2 * j + 1) * colorsChannels + k] = static_cast<char>(round(b));
                pixels[(2 * i + 1) * new_row_size + 2 * j * colorsChannels + k] = static_cast<char>(round(c));
                pixels[(2 * i + 1) * new_row_size + (2 * j + 1) * colorsChannels + k] = static_cast<char>(round(d));
            }
        }
    }

    inverse_result.write(pixels.data(), new_row_size * 2 * height + 2 * width * colorsChannels);
    inverse_result.close();
}

std::vector<char> write_haar_header(
    std::ifstream &input_file,
    std::ofstream &output_LL,
    std::ofstream &output_LH,
    std::ofstream &output_HL,
    std::ofstream &output_HH)
{
    BITMAPFILEHEADER fileHeader;
    input_file.read((char *)&fileHeader, sizeof(fileHeader));

    BITMAPINFOHEADER fileInfoHeader;
    input_file.read((char *)&fileInfoHeader, sizeof(fileInfoHeader));

    pixelsOffset = fileHeader.bfOffBits;
    colorsChannels = fileInfoHeader.biBitCount / 8;
    width = fileInfoHeader.biWidth;
    height = fileInfoHeader.biHeight;
    row_size = std::floor((fileInfoHeader.biBitCount * width + 31) / 32) * 4;
    new_row_size = std::floor((fileInfoHeader.biBitCount * (width / 2) + 31) / 32) * 4;

    fileInfoHeader.biWidth /= 2;
    fileInfoHeader.biHeight /= 2;

    writeHeader(input_file, output_LL, fileHeader, fileInfoHeader);
    writeHeader(input_file, output_LH, fileHeader, fileInfoHeader);
    writeHeader(input_file, output_HL, fileHeader, fileInfoHeader);
    writeHeader(input_file, output_HH, fileHeader, fileInfoHeader);

    std::vector<char> pixels(row_size * height + width * colorsChannels);
    input_file.read(pixels.data(), row_size * height + width * colorsChannels);

    input_file.close();

    return pixels;
}

// Прямое преобразование Хаара
void haar(const std::string &INPUT_PATH, const std::string &TXT_DIR_PATH, const std::string &COMPONENTS_DIR_PATH)
{

    std::ifstream input_file(INPUT_PATH, std::ios::binary);
    if (!(input_file.is_open()))
    {
        std::cout << "Can't open input_file";
        input_file.close();
        exit(1);
    }

    std::ofstream output_LL(COMPONENTS_DIR_PATH + "LL.bmp", std::ios::binary);
    std::ofstream output_LH(COMPONENTS_DIR_PATH + "LH.bmp", std::ios::binary);
    std::ofstream output_HL(COMPONENTS_DIR_PATH + "HL.bmp", std::ios::binary);
    std::ofstream output_HH(COMPONENTS_DIR_PATH + "HH.bmp", std::ios::binary);

    std::vector<char> pixels = write_haar_header(
        input_file,
        output_LL,
        output_LH,
        output_HL,
        output_HH);

    std::vector<std::shared_ptr<std::ofstream>> LL_data;
    std::vector<std::shared_ptr<std::ofstream>> LH_data;
    std::vector<std::shared_ptr<std::ofstream>> HL_data;
    std::vector<std::shared_ptr<std::ofstream>> HH_data;

    for (int k = 1; k < colorsChannels + 1; ++k)
    {
        auto LL_k = std::make_shared<std::ofstream>(TXT_DIR_PATH + "LL_data_" + std::to_string(k) + ".txt");
        auto LH_k = std::make_shared<std::ofstream>(TXT_DIR_PATH + "LH_data_" + std::to_string(k) + ".txt");
        auto HL_k = std::make_shared<std::ofstream>(TXT_DIR_PATH + "HL_data_" + std::to_string(k) + ".txt");
        auto HH_k = std::make_shared<std::ofstream>(TXT_DIR_PATH + "HH_data_" + std::to_string(k) + ".txt");

        LL_data.push_back(LL_k);
        LH_data.push_back(LH_k);
        HL_data.push_back(HL_k);
        HH_data.push_back(HH_k);
    }

    std::vector<char> LL_pixels(new_row_size * (height / 2) + (width / 2) * colorsChannels);
    std::vector<char> LH_pixels(new_row_size * (height / 2) + (width / 2) * colorsChannels);
    std::vector<char> HL_pixels(new_row_size * (height / 2) + (width / 2) * colorsChannels);
    std::vector<char> HH_pixels(new_row_size * (height / 2) + (width / 2) * colorsChannels);

    for (int i = 0; i < height; i += 2)
    {
        int count = 0;
        for (int j = 0; j < width; j += 2)
        {
            for (int k = 0; k < colorsChannels; k++)
            {
                int a = (int)(unsigned char)pixels[i * row_size + j * colorsChannels + k];
                int b = (int)(unsigned char)pixels[i * row_size + (j + 1) * colorsChannels + k];
                int c = (int)(unsigned char)pixels[(i + 1) * row_size + j * colorsChannels + k];
                int d = (int)(unsigned char)pixels[(i + 1) * row_size + (j + 1) * colorsChannels + k];

                double LL = (a + b + c + d) / 4.0;
                int LH = b - a;
                int HL = c - a;
                int HH = d - a;

                *LL_data[k] << LL << " ";
                *LH_data[k] << LH << " ";
                *HL_data[k] << HL << " ";
                *HH_data[k] << HH << " ";
                count++;

                LL_pixels[(i / 2) * new_row_size + (j / 2) * colorsChannels + k] = static_cast<char>(round(LL));
                LH_pixels[(i / 2) * new_row_size + (j / 2) * colorsChannels + k] = static_cast<char>(abs(LH));
                HL_pixels[(i / 2) * new_row_size + (j / 2) * colorsChannels + k] = static_cast<char>(abs(HL));
                HH_pixels[(i / 2) * new_row_size + (j / 2) * colorsChannels + k] = static_cast<char>(abs(HH));
            }
        }

        for (int k = 0; k < colorsChannels; ++k)
        {
            *LL_data[k] << "\n";
            *LH_data[k] << "\n";
            *HL_data[k] << "\n";
            *HH_data[k] << "\n";
        }
    }

    for (int k = 0; k < colorsChannels; ++k)
    {
        LL_data[k]->close();
        LH_data[k]->close();
        HL_data[k]->close();
        HH_data[k]->close();
    }

    output_LL.write(LL_pixels.data(), new_row_size * (height / 2) + (width / 2) * colorsChannels);
    output_LH.write(LH_pixels.data(), new_row_size * (height / 2) + (width / 2) * colorsChannels);
    output_HL.write(HL_pixels.data(), new_row_size * (height / 2) + (width / 2) * colorsChannels);
    output_HH.write(HH_pixels.data(), new_row_size * (height / 2) + (width / 2) * colorsChannels);

    output_LL.close();
    output_LH.close();
    output_HL.close();
    output_HH.close();
}

int main(int argc, char *argv[])
{

    std::cout << "Input - 1 to haar transform, input - 2 to inverse haar transform" << std::endl;
    int to_do;
    std::cin >> to_do;

    if (to_do == 1)
    {
        std::cout << "Write path to the bmp file: " << std::endl;
        std::string INPUT_PATH;
        std::cin >> INPUT_PATH;

        std::cout << "Write level transform: 1 or 2" << std::endl;
        int level_transform;
        std::cin >> level_transform;

        if (level_transform != 1 && level_transform != 2)
        {
            std::cout << "Invalid level transform!" << std::endl;
            return -1;
        }

        std::string TXT_DIR_PATH_1 = "txt_data_1/";
        std::string COMPONENTS_DIR_PATH_1 = "4_components_1/";
        haar(INPUT_PATH, TXT_DIR_PATH_1, COMPONENTS_DIR_PATH_1);

        std::cout << "The direct conversion for 1 level has been completed successfully!\n\n";

        if (level_transform == 2)
        {
            std::string TXT_BASE_DIR_PATH = "txt_data_2/";
            std::string COMPONENTS_BASE_DIR_PATH = "4_components_2/";

            std::vector<std::string> converter = {"LL", "LH", "HL", "HH"};

            for (int i = 1; i <= 4; ++i)
            {
                std::string TXT_DIR_PATH_2 = TXT_BASE_DIR_PATH + converter[i - 1] + "/";
                std::string COMPONENTS_DIR_PATH_2 = COMPONENTS_BASE_DIR_PATH + converter[i - 1] + "/";
                INPUT_PATH = COMPONENTS_DIR_PATH_1 + converter[i - 1] + ".bmp";

                haar(INPUT_PATH, TXT_DIR_PATH_2, COMPONENTS_DIR_PATH_2);
                std::cout << "The direct conversion for " + INPUT_PATH + " has been completed successfully!\n";
            }

            std::cout << "\nThe direct conversion for 2 level has been completed successfully!\n\n";
        }
    }

    else if (to_do == 2)
    {
        std::cout << "Write level transform" << std::endl;
        int level_transform;
        std::cin >> level_transform;

        if (level_transform != 1 && level_transform != 2)
        {
            std::cout << "Invalid level transform!" << std::endl;
            return -1;
        }

        std::string TXT_DIR_PATH_1 = "txt_data_1/";
        std::string COMPONENTS_DIR_PATH_1 = "4_components_1/";
        std::string INPUT_PATH = COMPONENTS_DIR_PATH_1 + "LL.bmp";
        std::string OUTPUT_PATH = "restored_base_file.bmp";

        inverse_haar(INPUT_PATH, OUTPUT_PATH, TXT_DIR_PATH_1);
        std::cout << "The reverse conversion for 1 level has been completed successfully!\n\n";

        if (level_transform == 2)
        {
            std::string TXT_BASE_DIR_PATH = "txt_data_2/";
            std::string COMPONENTS_BASE_DIR_PATH = "4_components_2/";

            std::vector<std::string> converter = {"LL", "LH", "HL", "HH"};
            for (int i = 1; i <= 4; ++i)
            {
                std::string TXT_DIR_PATH_2 = TXT_BASE_DIR_PATH + converter[i - 1] + "/";
                INPUT_PATH = COMPONENTS_BASE_DIR_PATH + converter[i - 1] + "/" + "LL.bmp";
                std::string OUTPUT_PATH = "restored_" + converter[i - 1] + ".bmp";

                inverse_haar(INPUT_PATH, OUTPUT_PATH, TXT_DIR_PATH_2);
                std::cout << "The reverse conversion for " + TXT_BASE_DIR_PATH + converter[i - 1] + " has been completed successfully!\n";
            }

            std::cout << "\nThe reverse conversion for 2 level has been completed successfully!\n\n";
        }
    }

    else
    {
        std::cout << "Unknown command" << std::endl;
    }

    return 0;
}