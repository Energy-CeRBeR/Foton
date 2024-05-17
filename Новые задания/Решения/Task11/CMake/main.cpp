#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <cstdint>
#include <cmath>
#include <thread>

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


BITMAPFILEHEADER fileHeader;
BITMAPINFOHEADER fileInfoHeader;
std::vector<char> pixels;
std::vector<char> new_pixels;
int row_size;
int colorsChannels;
int width;
int height;
int outputWidth;
int outputHeight;
int new_row_size;
double radian;
double cx;
double cy;


void rotate_thread(int thread_id, int num_threads) {
    for (int i = thread_id; i < outputHeight; i += num_threads) {
        for (int j = 0; j < outputWidth; ++j) {
            double x = (j - cx) * std::cos(radian) - (i - cy) * std::sin(radian) + width / 2;
            double y = (j - cx) * std::sin(radian) + (i - cy) * std::cos(radian) + height / 2;

            int x0 = floor(x);
            int y0 = floor(y);

            double dx = x - x0;
            double dy = y - y0;
            double w00 = (1 - dx) * (1 - dy);
            double w01 = (1 - dx) * dy;
            double w10 = dx * (1 - dy);
            double w11 = dx * dy;

            if (x >= 0 && x < width && y >= 0 && y < height) {
                for (int k = 0; k < colorsChannels; k++) {
                    new_pixels[i * new_row_size + j * colorsChannels + k] = round(
                        w00 * pixels[y0 * row_size + x0 * colorsChannels + k] +
                        w01 * pixels[y0 * row_size + x0 * colorsChannels + k] +
                        w10 * pixels[y0 * row_size + x0 * colorsChannels + k] +
                        w11 * pixels[y0 * row_size + x0 * colorsChannels + k]
                    );
                }
            }
        }
    }
}


void rotate_bln(const std::string INPUT_PATH, const std::string OUTPUT_PATH, int angle, int num_threads) {
    std::ifstream input_file(INPUT_PATH, std::ios::binary);
    std::ofstream output_file(OUTPUT_PATH, std::ios::binary);

    if (!(input_file.is_open() && output_file.is_open())) {
        std::cout << "Can't open one of the files";
        input_file.close();
        output_file.close();
        exit(1);
    }

    input_file.read((char*)&fileHeader, sizeof(fileHeader));
    input_file.read((char*)&fileInfoHeader, sizeof(fileInfoHeader));

    int pixelsOffset = fileHeader.bfOffBits;
    colorsChannels = fileInfoHeader.biBitCount / 8;
    width = fileInfoHeader.biWidth;
    height = fileInfoHeader.biHeight;
    row_size = std::floor((fileInfoHeader.biBitCount * width + 31) / 32) * 4;

    radian = angle * 3.14159265 / 180.0;

    outputWidth = abs(fileInfoHeader.biWidth * fabs(cos(radian)) + fileInfoHeader.biHeight * fabs(sin(radian)));
    outputHeight = abs(fileInfoHeader.biWidth * fabs(sin(radian)) + fileInfoHeader.biHeight * fabs(cos(radian)));

    fileInfoHeader.biWidth = outputWidth;
    fileInfoHeader.biHeight = outputHeight;

    new_row_size = std::floor((fileInfoHeader.biBitCount * outputWidth + 31) / 32) * 4;

    output_file.write((char*)&fileHeader, sizeof(fileHeader));
    output_file.write((char*)&fileInfoHeader, sizeof(fileInfoHeader));

    char* temp_info = new char[pixelsOffset - 54];
    input_file.read(temp_info, pixelsOffset - 54);
    output_file.write(temp_info, pixelsOffset - 54);
    delete[] temp_info;

    pixels.resize(row_size * height + width * colorsChannels);
    input_file.read(pixels.data(), row_size * height + width * colorsChannels);

    new_pixels.resize(new_row_size * outputHeight + outputWidth * colorsChannels);

    cx = outputWidth / 2;
    cy = outputHeight / 2;

    auto startTime = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.push_back(std::thread(rotate_thread, i, num_threads));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = endTime - startTime;

    output_file.write(new_pixels.data(), new_row_size * outputHeight + outputWidth * colorsChannels);

    std::cout << "New image has been successfully created using rotate_bln for " << duration.count() << " seconds" << std::endl;

    input_file.close();
    output_file.close();
}


int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cout << "Enter the PATH to the input file, to the output file, the rotate angle and number of threads";
        return 1;
    }

    std::string INPUT_PATH = argv[1];
    std::string OUTPUT_PATH = argv[2];
    std::string str_angle = argv[3];
    std::string str_threads = argv[4];
    int angle = std::stoi(str_angle);
    int num_threads = std::stoi(str_threads);

    rotate_bln(INPUT_PATH, OUTPUT_PATH, angle, num_threads);

    return 0;
}