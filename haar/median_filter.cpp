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

int width;
int height;
int colorsChannels;
int k;

std::vector<std::vector<std::vector<double>>> createPixelsVector(const std::vector<std::vector<std::vector<double>>> &pixels)
{
    std::vector<std::vector<std::vector<double>>> new_pixels(colorsChannels,
                                                             std::vector<std::vector<double>>(height + k,
                                                                                              std::vector<double>(width + k, 0)));

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
                new_pixels[c][i][j] = pixels[c][i_2][j_2];
            }
        }
    }

    return new_pixels;
}

std::vector<std::vector<double>> get_data_from_txt(const std::string PATH)
{
    std::ifstream file(PATH);
    if (!file.is_open())
    {
        std::cout << "Can't open file!\n";
        exit(1);
    }

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

void medianFilterTXT(std::string &DIR_PATH, const std::string &COMPONENT_NAME, const std::string &INPUT_PATH, const std::string &OUTPUT_PATH)
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

    int pixelsOffset = fileHeader.bfOffBits;
    int colorsChannels = fileInfoHeader.biBitCount / 8;
    int width2 = fileInfoHeader.biWidth;
    int height2 = fileInfoHeader.biHeight;
    int row_size = std::floor((fileInfoHeader.biBitCount * width2 + 31) / 32) * 4;

    char *temp_info = new char[pixelsOffset - 54];
    input_file.seekg(54);
    input_file.read(temp_info, pixelsOffset - 54);
    input_file.close();

    std::ofstream output_file(OUTPUT_PATH, std::ios::binary);
    output_file.write((char *)&fileHeader, sizeof(fileHeader));
    output_file.write((char *)&fileInfoHeader, sizeof(fileInfoHeader));

    output_file.write(temp_info, pixelsOffset - 54);
    delete[] temp_info;

    std::vector<char> output_pixels(row_size * height2 + width2 * colorsChannels);

    std::vector<std::vector<std::vector<double>>> input_data;
    std::vector<std::shared_ptr<std::ofstream>> output_data;
    for (int c = 1; c <= colorsChannels; ++c)
    {
        input_data.push_back(get_data_from_txt(DIR_PATH + COMPONENT_NAME + "_data_" + std::to_string(c) + ".txt"));
        output_data.push_back(std::make_shared<std::ofstream>(COMPONENT_NAME + "_data_" + std::to_string(c) + ".txt"));
    }

    height = input_data[0].size();
    width = input_data[0][0].size();

    std::vector<std::vector<std::vector<double>>> pixels = createPixelsVector(input_data);
    std::vector<std::vector<std::vector<double>>> new_pixels(colorsChannels,
                                                             std::vector<std::vector<double>>(height,
                                                                                              std::vector<double>(width, 0)));

    std::cout << "Start median filter!\n";

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            std::vector<std::vector<double>> neighborhood(colorsChannels);
            for (int a = i - k / 2; a <= i + k / 2; a++)
                for (int b = j - k / 2; b <= j + k / 2; b++)
                    for (int c = 0; c < colorsChannels; c++)
                        neighborhood[c].push_back(pixels[c][a + k / 2][b + k / 2]);

            for (int c = 0; c < colorsChannels; c++)
            {
                std::sort(neighborhood[c].begin(), neighborhood[c].end());
                new_pixels[c][i][j] = neighborhood[c][neighborhood[c].size() / 2];
                output_pixels[i * row_size + j * colorsChannels + c] = abs(neighborhood[c][neighborhood[c].size() / 2]);
                *output_data[c] << neighborhood[c][neighborhood[c].size() / 2] << " ";
            }
        }

        for (int c = 0; c < colorsChannels; ++c)
            *output_data[c] << "\n";
    }

    for (int c = 0; c < colorsChannels; ++c)
        output_data[c]->close();

    output_file.write(output_pixels.data(), row_size * height + width * colorsChannels);
    output_file.close();

    std::cout << "The application of the median filter to the text data was successful!\n";
}

int main(int argc, char *argv[])
{
    if (argc < 7)
    {
        std::cout << "Enter the PATH to dir with .txt file, component name (LL, LH, HL or HH), num of colors channels and the scanning window size k";
        return 1;
    }

    std::string DIR_PATH = argv[1];
    std::string COMPONENT_NAME = argv[2];
    std::string str_colors_channels = argv[3];
    std::string str_k = argv[4];
    std::string INPUT_PATH = argv[5];
    std::string OUTPUT_PATH = argv[6];
    k = std::stoi(str_k);
    colorsChannels = std::stoi(str_colors_channels);

    medianFilterTXT(DIR_PATH, COMPONENT_NAME, INPUT_PATH, OUTPUT_PATH);

    return 0;
}