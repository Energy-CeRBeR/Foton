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
std::vector<char> pixels;
std::vector<char> new_pixels;
int row_size;
int colorsChannels;
int width;
int height;
int outputWidth;
int outputHeight;
int new_row_size;
int pixelsOffset;
double radian;
double cx;
double cy;
int tile_size;


void rotate_thread(int thread_id, int num_threads, int cur_x, int cur_y) {
    
    //std::vector<char> current_pixels(new_row_size * tile_size + tile_size * colorsChannels);

    for (int i = cur_y + thread_id; i < std::min(cur_y + tile_size, outputHeight); i += num_threads) {
        for (int j = cur_x; j < std::min(cur_x + tile_size, outputWidth); j++) {
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

                std::vector<uint8_t> current_pixel(colorsChannels);
                input_file.seekg(pixelsOffset + (height > 0 ? (height - y0 - 1) : y0) * row_size + x0 * colorsChannels, std::ios::beg);
                input_file.read(reinterpret_cast<char*>(current_pixel.data()), colorsChannels);

                for (int k = 0; k < colorsChannels; k++) {
                    std::cout << (int)(unsigned char)current_pixel[k] << std::endl;
                    new_pixels[i * new_row_size + j * colorsChannels + k] = round(
                        w00 * current_pixel[k] +
                        w01 * current_pixel[k] +
                        w10 * current_pixel[k] +
                        w11 * current_pixel[k]
                    );
                }
            }
        }
    }
}


void rotate_bln(const std::string INPUT_PATH, const std::string OUTPUT_PATH, int angle, int tile_size) {
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

    output_file.write((char*)&fileHeader, sizeof(fileHeader));
    output_file.write((char*)&fileInfoHeader, sizeof(fileInfoHeader));

    char* temp_info = new char[pixelsOffset - 54];
    input_file.read(temp_info, pixelsOffset - 54);
    output_file.write(temp_info, pixelsOffset - 54);
    delete[] temp_info;

    //pixels.resize(row_size * height + width * colorsChannels);
    //input_file.read(pixels.data(), row_size * height + width * colorsChannels);

    new_pixels.resize(new_row_size * outputHeight + outputWidth * colorsChannels);

    cx = outputWidth / 2;
    cy = outputHeight / 2;

    auto startTime = std::chrono::high_resolution_clock::now();

    int num_threads = std::thread::hardware_concurrency();

    int width_tiles_counter = outputWidth / tile_size + 1; //+ (int)(outputWidth % tile_size != 0);
    int height_tiles_counter = outputHeight / tile_size + 1; //+ (int)(outputHeight % tile_size != 0);


    std::vector<std::thread> threads;
    for (int i = 0; i < height_tiles_counter; i++) {
        int current_y = i * tile_size;
        for (int j = 0; j < width_tiles_counter; j++) {
            int current_x = j * tile_size;
            for (int i = 0; i < num_threads; ++i) {
                threads.push_back(std::thread(rotate_thread, i, num_threads, current_x, current_y));
            }
        }
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