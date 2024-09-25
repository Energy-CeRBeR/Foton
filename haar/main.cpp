#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>

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

    for (int i = 0; i < height - 2; i += 2)
    {
        for (int j = 0; j < width - 2; j += 2)
        {
            LL_data << "(";
            LH_data << "(";
            HL_data << "(";
            HH_data << "(";

            for (int k = 0; k < colorsChannels; k++)
            {
                int a = (int)(unsigned char)pixels[i * row_size + j * colorsChannels + k];
                int b = (int)(unsigned char)pixels[i * row_size + (j + 1) * colorsChannels + k];
                int c = (int)(unsigned char)pixels[(i + 1) * row_size + j * colorsChannels + k];
                int d = (int)(unsigned char)pixels[(i + 1) * row_size + (j + 1) * colorsChannels + k];

                double LL = (a + b + c + d) / 4.0;
                int LH = abs(b - a);
                int HL = abs(c - a);
                int HH = abs(d - a);

                LL_data << LL << " ";
                LH_data << LH << " ";
                HL_data << HL << " ";
                HH_data << HH << " ";

                LL_pixels[(i / 2) * new_row_size + (j / 2) * colorsChannels + k] = static_cast<char>(round(LL));
                LH_pixels[(i / 2) * new_row_size + (j / 2) * colorsChannels + k] = static_cast<char>(abs(LH));
                HL_pixels[(i / 2) * new_row_size + (j / 2) * colorsChannels + k] = static_cast<char>(abs(HL));
                HH_pixels[(i / 2) * new_row_size + (j / 2) * colorsChannels + k] = static_cast<char>(abs(HH));
            }

            LL_data << ") ";
            LH_data << ") ";
            HL_data << ") ";
            HH_data << ") ";
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
    if (argc < 2)
    {
        std::cout << "Enter the PATH to the input file";
        return 1;
    }

    std::string INPUT_PATH = argv[1];

    run_transform(INPUT_PATH);

    return 0;
}
