#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <cstdint>
#include <cmath>
#include <thread>
#include <algorithm>
#include <cstdint>

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


std::ifstream input_file;
std::ofstream output_file;
BITMAPFILEHEADER fileHeader;
BITMAPINFOHEADER fileInfoHeader;
std::vector<char> tile_pixels;
std::vector<char> current_pixels;
int row_size;
int colorsChannels;
int width;
int height;
int outputWidth;
int outputHeight;
int new_row_size;
int tile_row_size;
int pixelsOffset;
double radian;
double cx;
double cy;
int tile_size;


void rotate_thread(int thread_id, int num_threads, int cur_x, int cur_y) {
    for (int i = cur_y * tile_size + thread_id; i < std::min((cur_y + 1) * tile_size, outputHeight); i += num_threads) {
        for (int j = cur_x * tile_size; j < std::min((cur_x + 1) * tile_size, outputWidth); j++) {
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
                    current_pixels[(i - cur_y * tile_size) * new_row_size + (j - cur_x * tile_size) * colorsChannels + k] = round(
                        w00 * tile_pixels[(i - cur_y * tile_size) * new_row_size + (j - cur_x * tile_size) * colorsChannels + k] +
                        w01 * tile_pixels[(i - cur_y * tile_size) * new_row_size + (j - cur_x * tile_size) * colorsChannels + k] +
                        w10 * tile_pixels[(i - cur_y * tile_size) * new_row_size + (j - cur_x * tile_size) * colorsChannels + k] +
                        w11 * tile_pixels[(i - cur_y * tile_size) * new_row_size + (j - cur_x * tile_size) * colorsChannels + k]
                    );
                }
            }
        }
    }
}


void read_data(int cur_x, int cur_y) {
    tile_pixels.resize(tile_size * new_row_size + tile_size * colorsChannels);
    for (int i = cur_y * tile_size; i < std::min((cur_y + 1) * tile_size, outputHeight); i++) {
        for (int j = cur_x * tile_size; j < std::min((cur_x + 1) * tile_size, outputWidth); j++) {
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
                std::vector<char> pixel(colorsChannels);
                input_file.seekg(pixelsOffset + y0 * row_size + x0 * colorsChannels, std::ios::beg);
                input_file.read(pixel.data(), colorsChannels);
                for (int k = 0; k < colorsChannels; k++) {
                    tile_pixels[(i - cur_y * tile_size) * new_row_size + (j - cur_x * tile_size) * colorsChannels + k] = pixel[k];
                }
            }
            else {
                for (int k = 0; k < colorsChannels; k++) {
                    tile_pixels[(i - cur_y * tile_size) * new_row_size + (j - cur_x * tile_size) * colorsChannels + k] = 0;
                }
            }
        }
    }
}


void write_data(int i, int j) {
    for (int q = 0; q < tile_size; q++) {
        for (int p = 0; p < tile_size; p++) {
            if (i * tile_size + q < outputHeight && j * tile_size + p < outputWidth) {
                output_file.seekp(pixelsOffset + new_row_size * (i * tile_size + q) + (j * tile_size + p) * colorsChannels, std::ios::beg);
                std::vector<char> pixel(colorsChannels);
                for (int k = 0; k < colorsChannels; k++) {
                    pixel[k] = current_pixels[q * new_row_size + p * colorsChannels + k];
                    current_pixels[q * new_row_size + p * colorsChannels + k] = 0;
                }
                output_file.write(pixel.data(), colorsChannels);
            }
        }
    }
}


void rotate_bln(const std::string INPUT_PATH, const std::string OUTPUT_PATH, int angle, int tile_size) {
    input_file.open(INPUT_PATH, std::ios::binary);
    output_file.open(OUTPUT_PATH, std::ios::binary);

    if (!(input_file.is_open() && output_file.is_open())) {
        std::cout << "Can't open one of the files";
        input_file.close();
        output_file.close();
        exit(1);
    }

    input_file.read((char*)&fileHeader, sizeof(fileHeader));
    input_file.read((char*)&fileInfoHeader, sizeof(fileInfoHeader));

    pixelsOffset = fileHeader.bfOffBits;
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
    tile_row_size = std::floor((fileInfoHeader.biBitCount * tile_size + 31) / 32) * 4;

    output_file.write((char*)&fileHeader, sizeof(fileHeader));
    output_file.write((char*)&fileInfoHeader, sizeof(fileInfoHeader));

    char* temp_info = new char[pixelsOffset - 54];
    input_file.read(temp_info, pixelsOffset - 54);
    output_file.write(temp_info, pixelsOffset - 54);
    delete[] temp_info;

    cx = outputWidth / 2;
    cy = outputHeight / 2;

    int width_tiles_counter = outputWidth / tile_size + 1; 
    int height_tiles_counter = outputHeight / tile_size + 1;

    int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    current_pixels.resize(new_row_size * tile_size + tile_size * colorsChannels);

    double time_to_read = 0.0;
    double time_to_rotate = 0.0;
    double time_to_write = 0.0;
    for (int i = 0; i < height_tiles_counter; i++) {
        for (int j = 0; j < width_tiles_counter; j++) {
            auto startReadTime = std::chrono::high_resolution_clock::now();
            read_data(j, i);
            auto endReadTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> durationRead = endReadTime - startReadTime;
            time_to_read += durationRead.count();

            auto startRotateTime = std::chrono::high_resolution_clock::now();
            for (int k = 0; k < num_threads; ++k) {
                threads.push_back(std::thread(rotate_thread, k, num_threads, j, i));
            }
            for (auto& thread : threads) {
                thread.join();
            }
            threads.clear();
            auto endRotateTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> durationRotate = endRotateTime - startRotateTime;
            time_to_rotate += durationRotate.count();

            auto startWriteTime = std::chrono::high_resolution_clock::now();
            write_data(i, j);
            auto endWriteTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> durationWrite = endWriteTime - startWriteTime;
            time_to_write += durationWrite.count();
        }
    }

    std::cout << "New image has been successfully created!" << std::endl;
    std::cout << "Total data reading time: " << time_to_read << " seconds" << std::endl;
    std::cout << "Total image rotation time: " << time_to_rotate << " seconds" << std::endl;
    std::cout << "Total data writing time: " << time_to_write << " seconds" << std::endl;


    input_file.close();
    output_file.close();
}


int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cout << "Enter the PATH to the input file, to the output file, the rotate angle and size of tile";
        return 1;
    }

    std::string INPUT_PATH = argv[1];
    std::string OUTPUT_PATH = argv[2];
    std::string str_angle = argv[3];
    std::string str_tile_size = argv[4];

    int angle = std::stoi(str_angle);
    tile_size = std::stoi(str_tile_size);

    rotate_bln(INPUT_PATH, OUTPUT_PATH, angle, tile_size);


    return 0;
}