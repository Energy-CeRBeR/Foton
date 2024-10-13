#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <chrono>
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
int k;

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

void medianFilterSort(std::ofstream &output_file, const std::string &COMPONENT_NAME, const std::vector<char> &pixels)
{
    std::vector<std::shared_ptr<std::ofstream>> txt_data;
    for (int c = 1; c <= colorsChannels; ++c)
        txt_data.push_back(std::make_shared<std::ofstream>(COMPONENT_NAME + "_data_" + std::to_string(c) + ".txt"));

    std::vector<char> new_pixels(row_size * height + width * colorsChannels);
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            std::vector<std::vector<char>> neighborhood(colorsChannels);
            for (int a = i - k / 2; a <= i + k / 2; a++)
                for (int b = j - k / 2; b <= j + k / 2; b++)
                    for (int c = 0; c < colorsChannels; c++)
                        neighborhood[c].push_back(pixels[(a + k / 2) * new_row_size + (b + k / 2) * colorsChannels + c]);

            for (int c = 0; c < colorsChannels; c++)
            {
                std::sort(neighborhood[c].begin(), neighborhood[c].end());
                new_pixels[i * row_size + j * colorsChannels + c] = neighborhood[c][neighborhood[c].size() / 2];
                *txt_data[c] << (int)(unsigned char)neighborhood[c][neighborhood[c].size() / 2] << " ";
            }
        }

        for (int c = 0; c < colorsChannels; ++c)
            *txt_data[c] << "\n";
    }

    for (int c = 0; c < colorsChannels; ++c)
        txt_data[c]->close();

    output_file.write(new_pixels.data(), row_size * height + width * colorsChannels);
    output_file.close();
}

std::vector<char> createPixelsVector(const std::vector<char> &pixels)
{
    std::vector<char> new_pixels(new_row_size * new_height + new_width * colorsChannels);
    for (int i = 0; i < height + k; i++)
    {
        for (int j = 0; j < width + k; j++)
        {
            int i_2 = i - k / 2;
            int j_2 = j - k / 2;

            if (i < k / 2)
            {
                i_2 = (k / 2 - i);
            }
            if (j < k / 2)
            {
                j_2 = (k / 2 - j);
            }
            if (i >= height + k / 2)
            {
                i_2 = i - k - 1;
            }
            if (j >= width + k / 2)
            {
                j_2 = j - k - 1;
            }

            for (int c = 0; c < colorsChannels; c++)
            {
                new_pixels[i * new_row_size + j * colorsChannels + c] = pixels[i_2 * row_size + j_2 * colorsChannels + c];
            }
        }
    }

    return new_pixels;
}

void medianFilter(const std::string INPUT_PATH, const std::string OUTPUT_PATH, const std::string &COMPONENT_NAME)
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
    new_width = width + k;
    new_height = height + k;
    row_size = std::floor((fileInfoHeader.biBitCount * width + 31) / 32) * 4;
    new_row_size = std::floor((fileInfoHeader.biBitCount * new_width + 31) / 32) * 4;

    std::ofstream output_file(OUTPUT_PATH, std::ios::binary);
    if (!(output_file.is_open()))
    {
        std::cout << "Can't open output_file";
        input_file.close();
        output_file.close();
        exit(1);
    }

    writeHeader(input_file, output_file, fileHeader, fileInfoHeader);

    std::vector<char> pixels(row_size * height + width * colorsChannels);
    input_file.read(pixels.data(), row_size * height + width * colorsChannels);
    input_file.close();

    std::vector<char> special_pixels = createPixelsVector(pixels);
    medianFilterSort(output_file, COMPONENT_NAME, special_pixels);

    std::cout << "Median filter with std::sort was successfully applied!\n";
}

int main(int argc, char *argv[])
{
    if (argc < 5)
    {
        std::cout << "Enter the PATH to the .bmp component, to the updated component path, component name (LL, LH, HL or HH) and the scanning window size k";
        return 1;
    }

    std::string INPUT_PATH = argv[1];
    std::string OUTPUT_PATH = argv[2];
    std::string COMPONENT_NAME = argv[3];
    std::string str_k = argv[4];
    k = std::stoi(str_k);

    medianFilter(INPUT_PATH, OUTPUT_PATH, COMPONENT_NAME);

    return 0;
}