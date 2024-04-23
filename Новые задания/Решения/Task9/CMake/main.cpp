#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <cstdint>
#include <cmath>

#pragma pack(2)


struct BITMAPFILEHEADER {
    unsigned short bfType;
    unsigned int bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
};

struct BITMAPINFOHEADER {
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


void rotate_nn(const std::string INPUT_PATH, const std::string OUTPUT_PATH, int angle) {
    std::ifstream input_file(INPUT_PATH, std::ios::binary);
    std::ofstream output_file(OUTPUT_PATH, std::ios::binary);

    if (!(input_file.is_open() && output_file.is_open())) {
        std::cout << "Can't open one of the files";
        input_file.close();
        output_file.close();
        exit(1);
    }

    BITMAPFILEHEADER fileHeader;
    input_file.read((char*)&fileHeader, sizeof(fileHeader));

    BITMAPINFOHEADER fileInfoHeader;
    input_file.read((char*)&fileInfoHeader, sizeof(fileInfoHeader));

    int pixelsOffset = fileHeader.bfOffBits;
    int colorsChannels = fileInfoHeader.biBitCount / 8;
    int width = fileInfoHeader.biWidth;
    int height = fileInfoHeader.biHeight;
    int row_size = std::floor((fileInfoHeader.biBitCount * width + 31) / 32) * 4;


    output_file.write((char*)&fileHeader, sizeof(fileHeader));
    output_file.write((char*)&fileInfoHeader, sizeof(fileInfoHeader));

    char* temp_info = new char[pixelsOffset - 54];
    input_file.read(temp_info, pixelsOffset - 54);
    output_file.write(temp_info, pixelsOffset - 54);
    delete[] temp_info;

    std::vector<char> row(row_size);
    std::vector<char> pixels(row_size * height + width * colorsChannels);
    input_file.read(pixels.data(), row_size * height + width * colorsChannels);

    std::vector<char> new_pixels(row_size * height + width * colorsChannels);

    auto startTime = std::chrono::high_resolution_clock::now();

    double cx = width / 2;
    double cy = height / 2;
    double radian = angle * 3.14159265 / 180.0;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            double x = (j - cx) * cos(radian) - (i - cy) * sin(radian) + cx;
            double y = (j - cx) * sin(radian) + (i - cy) * cos(radian) + cy;

            if (x >= 0 && x < width && y >= 0 && y < height) {
                for (int k = 0; k < colorsChannels; k++) {
                    new_pixels[i * row_size + j * colorsChannels + k] = pixels[(int)y * row_size + (int)x * colorsChannels + k];
                }
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = endTime - startTime;

    output_file.write(new_pixels.data(), row_size * height + width * colorsChannels);

    std::cout << "New image has been successfully created using rotate_nn for " << duration.count() << " seconds" << std::endl;

    input_file.close();
    output_file.close();
}


void rotate_bln(const std::string INPUT_PATH, const std::string OUTPUT_PATH, int angle) {
    std::ifstream input_file(INPUT_PATH, std::ios::binary);
    std::ofstream output_file(OUTPUT_PATH, std::ios::binary);

    if (!(input_file.is_open() && output_file.is_open())) {
        std::cout << "Can't open one of the files";
        input_file.close();
        output_file.close();
        exit(1);
    }

    BITMAPFILEHEADER fileHeader;
    input_file.read((char*)&fileHeader, sizeof(fileHeader));

    BITMAPINFOHEADER fileInfoHeader;
    input_file.read((char*)&fileInfoHeader, sizeof(fileInfoHeader));

    int pixelsOffset = fileHeader.bfOffBits;
    int colorsChannels = fileInfoHeader.biBitCount / 8;
    int width = fileInfoHeader.biWidth;
    int height = fileInfoHeader.biHeight;
    int row_size = std::floor((fileInfoHeader.biBitCount * width + 31) / 32) * 4;

    output_file.write((char*)&fileHeader, sizeof(fileHeader));
    output_file.write((char*)&fileInfoHeader, sizeof(fileInfoHeader));


    char* temp_info = new char[pixelsOffset - 54];
    input_file.read(temp_info, pixelsOffset - 54);
    output_file.write(temp_info, pixelsOffset - 54);
    delete[] temp_info;

    std::vector<char> pixels(row_size * height + width * colorsChannels);
    input_file.read(pixels.data(), row_size * height + width * colorsChannels);

    std::vector<char> new_pixels(row_size * height + width * colorsChannels);

    auto startTime = std::chrono::high_resolution_clock::now();

    double cx = width / 2.0;
    double cy = height / 2.0;
    double radian = angle * 3.14159265 / 180.0;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            double x = (j - cx) * std::cos(radian) - (i - cy) * std::sin(radian) + cx;
            double y = (j - cx) * std::sin(radian) + (i - cy) * std::cos(radian) + cy;

            int x0 = floor(x);
            int y0 = floor(y);
            int x1 = x0 + 1;
            int y1 = y0 + 1;

            double dx = x - x0;
            double dy = y - y0;
            double w00 = (1 - dx) * (1 - dy);
            double w01 = (1 - dx) * dy;
            double w10 = dx * (1 - dy);
            double w11 = dx * dy;

            if (x >= 0 && x < width && y >= 0 && y < height) {
                for (int k = 0; k < colorsChannels; k++) {
                    new_pixels[i * row_size + j * colorsChannels + k] = round(
                        w00 * pixels[y0 * row_size + x0 * colorsChannels + k] +
                        w01 * pixels[y0 * row_size + x0 * colorsChannels + k] +
                        w10 * pixels[y0 * row_size + x0 * colorsChannels + k] +
                        w11 * pixels[y0 * row_size + x0 * colorsChannels + k]
                    );
                }
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = endTime - startTime;

    output_file.write(new_pixels.data(), row_size * height + width * colorsChannels);

    std::cout << "New image has been successfully created using rotate_bln for " << duration.count() << " seconds" << std::endl;

    input_file.close();
    output_file.close();
}


int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Enter the PATH to the input file, to the output file and the rotate angle";
        return 1;
    }

    std::string INPUT_PATH = argv[1];
    std::string OUTPUT_PATH = argv[2];
    std::string OUTPUTPATH_FOR_SCALE_NN = "_rotate_nn_" + OUTPUT_PATH;
    std::string OUTPUTPATH_FOR_SCALE_BLN = "_rotate_bln_" + OUTPUT_PATH;
    std::string str_angle = argv[3];
    int angle = std::stoi(str_angle);

    rotate_nn(INPUT_PATH, OUTPUTPATH_FOR_SCALE_NN, angle);
    rotate_bln(INPUT_PATH, OUTPUTPATH_FOR_SCALE_BLN, angle);

    return 0;
}