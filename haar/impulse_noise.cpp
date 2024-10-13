#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>

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
int colorsChannels;
int row_size;

void impulse_noise(std::vector<char> pixels, std::ofstream &output_file, int N)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);

    std::vector<std::pair<int, int>> data;
    for (int i = 0; i < width; ++i)
        for (int j = 0; j < height; ++j)
            data.push_back({i, j});
    std::shuffle(data.begin(), data.end(), gen);

    std::cout << "Generate impulse noise...\n";

    for (int k = 0; k < N; ++k)
    {
        std::pair<int, int> pos = data.back();
        data.pop_back();

        int x = pos.first;
        int y = pos.second;
        for (int c = 0; c < colorsChannels; ++c)
            pixels[y * row_size + x * colorsChannels + c] = distrib(gen);
    }

    std::cout << "Impulse noise has been successfully added!\n";

    output_file.write(pixels.data(), row_size * height + width * colorsChannels);
    output_file.close();
}

std::vector<char> write_header(std::ifstream &input_file, std::ofstream &output_file)
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

    output_file.write((char *)&fileHeader, sizeof(fileHeader));
    output_file.write((char *)&fileInfoHeader, sizeof(fileInfoHeader));

    char *temp_info = new char[pixelsOffset - 54];
    input_file.seekg(54);
    input_file.read(temp_info, pixelsOffset - 54);
    output_file.write(temp_info, pixelsOffset - 54);
    delete[] temp_info;

    std::vector<char> pixels(row_size * height + width * colorsChannels);
    input_file.read(pixels.data(), row_size * height + width * colorsChannels);
    input_file.close();

    std::cout << "Header data successful write to the output file!\n";

    return pixels;
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cout << "You need to enter the path to the input file, path to the output file and noise count!\n";
        return 1;
    }

    std::string INPUT_PATH = argv[1];
    std::string OUTPUT_PATH = argv[2];

    std::ifstream input_file(INPUT_PATH, std::ios::binary);
    if (!(input_file.is_open()))
    {
        std::cout << "Can't open input file";
        input_file.close();
        exit(1);
    }

    std::ofstream output_file(OUTPUT_PATH, std::ios::binary);
    if (!output_file.is_open())
    {
        std::cout << "Can't open one of output_files";
        input_file.close();
        output_file.close();
        exit(1);
    }

    int N = std::stoi(argv[3]);

    impulse_noise(write_header(input_file, output_file), output_file, N);

    return 0;
}
