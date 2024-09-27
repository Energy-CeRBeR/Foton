#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <sstream>

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

// std::vector<std::vector<char>> read_four_components(
//     std::ifstream &LL_component,
//     std::ifstream &LH_component,
//     std::ifstream &HL_component,
//     std::ifstream &HH_component,
//     std::ofstream &inverse_result)
// {

//     std::vector<char> LL_pixels(row_size * height + width * colorsChannels);
//     LL_component.read(LL_pixels.data(), row_size * height + width * colorsChannels);
//     LL_component.close();

//     std::vector<char> LH_pixels(row_size * height + width * colorsChannels);
//     LH_component.read(LH_pixels.data(), row_size * height + width * colorsChannels);
//     LH_component.close();

//     std::vector<char> HL_pixels(row_size * height + width * colorsChannels);
//     HL_component.read(HL_pixels.data(), row_size * height + width * colorsChannels);
//     HL_component.close();

//     std::vector<char> HH_pixels(row_size * height + width * colorsChannels);
//     HH_component.read(HH_pixels.data(), row_size * height + width * colorsChannels);
//     HH_component.close();
// }

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

void inverse_haar(std::string INPUT_PATH)
{
    std::ifstream haar_component(INPUT_PATH, std::ios::binary);
    if (!(haar_component.is_open()))
    {
        std::cout << "Can't open input_file";
        haar_component.close();
        exit(1);
    }

    BITMAPFILEHEADER fileHeader;
    haar_component.read((char *)&fileHeader, sizeof(fileHeader));

    BITMAPINFOHEADER fileInfoHeader;
    haar_component.read((char *)&fileInfoHeader, sizeof(fileInfoHeader));

    pixelsOffset = fileHeader.bfOffBits;
    colorsChannels = fileInfoHeader.biBitCount / 8;
    width = fileInfoHeader.biWidth;
    height = fileInfoHeader.biHeight;
    // row_size = std::floor((fileInfoHeader.biBitCount * width + 31) / 32) * 4;
    new_row_size = std::floor((fileInfoHeader.biBitCount * 2 * width + 31) / 32) * 4;

    fileInfoHeader.biWidth *= 2;
    fileInfoHeader.biHeight *= 2;

    std::ofstream inverse_result("inverse_result.bmp", std::ios::binary);
    writeHeader(haar_component, inverse_result, fileHeader, fileInfoHeader);
    std::vector<char> pixels(new_row_size * 2 * height + 2 * width * colorsChannels);

    std::cout << "Getting LL_component data ..." << std::endl;
    std::ifstream LL_txt_file("LL_data.txt");
    std::vector<std::vector<double>> LL_data = get_data_from_txt(LL_txt_file);
    std::cout << "LL_component data received!" << std::endl
              << std::endl;

    std::cout << "Getting LH_component data ..." << std::endl;
    std::ifstream LH_txt_file("LH_data.txt");
    std::vector<std::vector<double>> LH_data = get_data_from_txt(LH_txt_file);
    std::cout << "LH_component data received!" << std::endl
              << std::endl;

    std::cout << "Getting HL_component data ..." << std::endl;
    std::ifstream HL_txt_file("HL_data.txt");
    std::vector<std::vector<double>> HL_data = get_data_from_txt(HL_txt_file);
    std::cout << "HL_component data received!" << std::endl
              << std::endl;

    std::cout << "Getting HH_component data ..." << std::endl;
    std::ifstream HH_txt_file("HH_data.txt");
    std::vector<std::vector<double>> HH_data = get_data_from_txt(HH_txt_file);
    std::cout << "HH_component data received!" << std::endl
              << std::endl;

    std::cout << "All data has been uploaded successfully!" << std::endl
              << std::endl;

    for (int i = 0; i < height - 1; i++)
    {
        for (int j = 0; j < width; j++)
        {
            for (int k = 0; k < colorsChannels; k++)
            {
                double x = LL_data[i][j];
                double y = LH_data[i][j];
                double z = HL_data[i][j];
                double w = HH_data[i][j];

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

    std::cout << "SUCCESS!!!" << std::endl;

    inverse_result.close();
}

void haar(
    std::ofstream &output_LL,
    std::ofstream &output_LH,
    std::ofstream &output_HL,
    std::ofstream &output_HH,
    const std::vector<char> &pixels)
{

    std::ofstream LL_data("LL_data.txt");
    std::ofstream LH_data("LH_data.txt");
    std::ofstream HL_data("HL_data.txt");
    std::ofstream HH_data("HH_data.txt");

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

                LL_data << LL << " ";
                LH_data << LH << " ";
                HL_data << HL << " ";
                HH_data << HH << " ";
                count++;

                LL_pixels[(i / 2) * new_row_size + (j / 2) * colorsChannels + k] = static_cast<char>(round(LL));
                LH_pixels[(i / 2) * new_row_size + (j / 2) * colorsChannels + k] = static_cast<char>(abs(LH));
                HL_pixels[(i / 2) * new_row_size + (j / 2) * colorsChannels + k] = static_cast<char>(abs(HL));
                HH_pixels[(i / 2) * new_row_size + (j / 2) * colorsChannels + k] = static_cast<char>(abs(HH));
            }
        }

        LL_data << "\n";
        LH_data << "\n";
        HL_data << "\n";
        HH_data << "\n";
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

void run_transform(const std::string INPUT_PATH)
{
    std::ifstream input_file(INPUT_PATH, std::ios::binary);
    if (!(input_file.is_open()))
    {
        std::cout << "Can't open input_file";
        input_file.close();
        exit(1);
    }

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

    std::ofstream output_LL("LL.bmp", std::ios::binary);
    std::ofstream output_LH("LH.bmp", std::ios::binary);
    std::ofstream output_HL("HL.bmp", std::ios::binary);
    std::ofstream output_HH("HH.bmp", std::ios::binary);

    writeHeader(input_file, output_LL, fileHeader, fileInfoHeader);
    writeHeader(input_file, output_LH, fileHeader, fileInfoHeader);
    writeHeader(input_file, output_HL, fileHeader, fileInfoHeader);
    writeHeader(input_file, output_HH, fileHeader, fileInfoHeader);

    std::vector<char> pixels(row_size * height + width * colorsChannels);
    input_file.read(pixels.data(), row_size * height + width * colorsChannels);

    input_file.close();

    haar(output_LL, output_LH, output_HL, output_HH, pixels);

    std::cout << "SUCCESS!!!" << std::endl;
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
        run_transform(INPUT_PATH);
    }

    else if (to_do == 2)
    {
        std::cout << "Write path to the one of the haar transform components file" << std::endl;
        std::string INPUT_PATH;
        std::cin >> INPUT_PATH;
        inverse_haar(INPUT_PATH);
    }

    else
    {
        std::cout << "Unknown command" << std::endl;
    }

    return 0;
}
