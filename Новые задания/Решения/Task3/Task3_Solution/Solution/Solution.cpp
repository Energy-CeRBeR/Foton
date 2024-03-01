#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>


struct BMP_data {
    int pixelsOffset;
    int width;
    int height;
    int bitsPerPixel;
    int colorsChannels;
};


struct PixelsData {
    std::vector<uint8_t> upper_left;
    std::vector<uint8_t> upper_right;
    std::vector<uint8_t> central;
    std::vector<uint8_t> lower_left;
    std::vector<uint8_t> lower_right;
};


BMP_data readBMP(const std::string PATH) {
    BMP_data bmp_data;

    std::ifstream file(PATH, std::ios::binary);
    if (!file) {
        std::cout << "Can't open this file";
        file.close();
        exit(1);
    }

    char header[54];
    file.read(header, 54);
    if (header[0] != 'B' || header[1] != 'M') {
        std::cout << "The file is not a bmp format file!" << std::endl;
        file.close();
        exit(1);
    }

    bmp_data.pixelsOffset = *(int*)&header[10];
    bmp_data.width = *(int*)&header[18];
    bmp_data.height = *(int*)&header[22];
    bmp_data.bitsPerPixel = *(int*)&header[28];
    bmp_data.colorsChannels = bmp_data.bitsPerPixel / 8;

    file.close();
    return bmp_data;
}


PixelsData get_pixels_data(const BMP_data& bmp_data, const std::string PATH) {
    PixelsData pixels_data;
    int row_size = floor((bmp_data.bitsPerPixel * bmp_data.width + 31) / 32) * 4;
    std::ifstream file(PATH, std::ios::binary);
    std::vector<uint8_t> rgb(bmp_data.colorsChannels);

    file.seekg(bmp_data.pixelsOffset + (bmp_data.height > 0 ? (bmp_data.height - 1) : 0) * row_size, std::ios::beg);
    file.read(reinterpret_cast<char*>(rgb.data()), bmp_data.colorsChannels);
    pixels_data.upper_left = rgb;

    file.seekg(bmp_data.pixelsOffset + (bmp_data.height > 0 ? (bmp_data.height - 1) : 0) * row_size + (bmp_data.width - 1) * bmp_data.colorsChannels, std::ios::beg);
    file.read(reinterpret_cast<char*>(rgb.data()), bmp_data.colorsChannels);
    pixels_data.upper_right = rgb;

    file.seekg(bmp_data.pixelsOffset + (bmp_data.height > 0 ?
        (bmp_data.height - std::floor(bmp_data.height / 2) - 1) : std::floor(bmp_data.height / 2)) * row_size +
        std::floor(bmp_data.width / 2) * bmp_data.colorsChannels, std::ios::beg);
    file.read(reinterpret_cast<char*>(rgb.data()), bmp_data.colorsChannels);
    pixels_data.central = rgb;

    file.seekg(bmp_data.pixelsOffset + (bmp_data.height > 0 ?
        (bmp_data.height - (bmp_data.height - 1) - 1) : (bmp_data.height - 1)) * row_size, std::ios::beg);
    file.read(reinterpret_cast<char*>(rgb.data()), bmp_data.colorsChannels);
    pixels_data.lower_left = rgb;

    file.seekg(bmp_data.pixelsOffset + (bmp_data.height > 0 ?
        (bmp_data.height - (bmp_data.height - 1) - 1) : (bmp_data.height - 1)) * row_size +
        (bmp_data.width - 1) * bmp_data.colorsChannels, std::ios::beg);
    file.read(reinterpret_cast<char*>(rgb.data()), bmp_data.colorsChannels);
    pixels_data.lower_right = rgb;

    file.close();
    return pixels_data;
}


void print_bmp_data(const BMP_data& bmp_data) {
    std::cout << "BMP info:" << std::endl;
    std::cout << "Image width: " << bmp_data.width << std::endl;
    std::cout << "Image height: " << bmp_data.height << std::endl;
    std::cout << "Bits per pixel: " << bmp_data.bitsPerPixel << std::endl;
    std::cout << "Colors channel: " << bmp_data.colorsChannels << std::endl;
}


void _print_colors(const std::vector<uint8_t>& position, int colorsChannels) {
    if (colorsChannels > 1) {
        for (int i = colorsChannels - 1; i >= 0; i--) {
            std::cout << (int)position[i] << " ";
        }
    }
    else {
        for (int i = 0; i < 3; i++) {
            std::cout << (int)position[0] << " ";
        }
    }
    std::cout << std::endl;
}


void print_pixels_data(const PixelsData& pixels_data, int colorsChannels) {
    std::cout << "Pixels info:" << std::endl;

    std::cout << "Coordinates of the upper-left pixel: ";
    _print_colors(pixels_data.upper_left, colorsChannels);

    std::cout << "Coordinates of the upper-right pixel: ";
    _print_colors(pixels_data.upper_right, colorsChannels);

    std::cout << "Coordinates of the central pixel:";
    _print_colors(pixels_data.central, colorsChannels);

    std::cout << "Coordinates of the lower-left pixel: ";
    _print_colors(pixels_data.lower_left, colorsChannels);

    std::cout << "Coordinates of the lower_right pixel: ";
    _print_colors(pixels_data.lower_right, colorsChannels);
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Enter the path to the file";
        return 1;
    }

    std::string PATH = argv[1];
    BMP_data bmp_data = readBMP(PATH);
    PixelsData pixels_data = get_pixels_data(bmp_data, PATH);

    print_bmp_data(bmp_data);
    std::cout << std::endl;
    print_pixels_data(pixels_data, bmp_data.colorsChannels);

    return 0;
}
