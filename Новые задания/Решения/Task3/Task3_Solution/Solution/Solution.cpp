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

    int pixelsOffset = *(int*)&header[10];
    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    int bitsPerPixel = *(int*)&header[28];
    int colorsChannels = bitsPerPixel / 8;

    bmp_data.width = width;
    bmp_data.height = height;
    bmp_data.pixelsOffset = pixelsOffset;
    bmp_data.bitsPerPixel = bitsPerPixel;
    bmp_data.colorsChannels = colorsChannels;

    file.close();
    return bmp_data;
}


PixelsData get_pixels_data(const BMP_data& bmp_data, const std::string PATH) {
    PixelsData pixels_data;
    int row_size = ((((bmp_data.width * 24) + 31) & ~31) >> 3);
    std::ifstream file(PATH, std::ios::binary);

    std::vector<uint8_t> rgb_1(3);
    file.seekg(bmp_data.pixelsOffset + (bmp_data.height > 0 ? (bmp_data.width - 1) : 0) * row_size, std::ios::beg);
    file.read(reinterpret_cast<char*>(rgb_1.data()), 3);
    pixels_data.upper_left = rgb_1;

    std::vector<uint8_t> rgb_2(3);
    file.seekg(bmp_data.pixelsOffset + (bmp_data.height > 0 ? (bmp_data.width - 1) : 0) * row_size + (bmp_data.width * 3), std::ios::beg);
    file.read(reinterpret_cast<char*>(rgb_2.data()), 3);
    pixels_data.upper_right = rgb_2;

    std::vector<uint8_t> rgb_3(3);
    file.seekg(bmp_data.pixelsOffset + (bmp_data.height > 0 ?
        (bmp_data.width - std::floor(bmp_data.height / 2) - 1) : std::floor(bmp_data.height / 2)) * row_size +
        std::floor(bmp_data.width / 2) * 3, std::ios::beg);
    file.read(reinterpret_cast<char*>(rgb_3.data()), 3);
    pixels_data.central = rgb_3;

    std::vector<uint8_t> rgb_4(3);
    file.seekg(bmp_data.pixelsOffset + (bmp_data.height > 0 ?
        (bmp_data.width - (bmp_data.height - 1) - 1) : (bmp_data.height - 1)) * row_size, std::ios::beg);
    file.read(reinterpret_cast<char*>(rgb_4.data()), 3);
    pixels_data.lower_left = rgb_4;
    file.seekg(0);

    std::vector<uint8_t> rgb_5(3);
    file.seekg(bmp_data.pixelsOffset + (bmp_data.height > 0 ?
        (bmp_data.width - (bmp_data.height - 1) - 1) : (bmp_data.height - 1)) * row_size +
        (bmp_data.width - 1) * 3, std::ios::beg);
    file.read(reinterpret_cast<char*>(rgb_5.data()), 3);
    pixels_data.lower_right = rgb_5;

    std::cout << (int)pixels_data.upper_left[0] << std::endl;
    std::cout << (int)pixels_data.upper_right[0] << std::endl;
    std::cout << (int)pixels_data.central[0] << std::endl;
    std::cout << (int)pixels_data.lower_left[0] << std::endl;
    std::cout << (int)pixels_data.lower_right[0] << std::endl;

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


void print_pixels_data(const PixelsData& pixels_data) {
    std::cout << "Pixels info:" << std::endl;

    std::vector<uint8_t> upper_left = pixels_data.upper_left;
    std::cout << "Coordinates of the upper-left pixel: " << (int)upper_left[2] << " " << (int)upper_left[0] << " " << (int)upper_left[1] << std::endl;

    std::vector<uint8_t> upper_right = pixels_data.upper_right;
    std::cout << "Coordinates of the upper-right pixel: " << (int)upper_right[2] << " " << (int)upper_right[0] << " " << (int)upper_right[1] << std::endl;

    std::vector<uint8_t> central = pixels_data.central;
    std::cout << "Coordinates of the central pixel: " << (int)central[2] << " " << (int)central[0] << " " << (int)central[1] << std::endl;

    std::vector<uint8_t> lower_left = pixels_data.lower_left;
    std::cout << "Coordinates of the lower-left pixel: " << (int)lower_left[2] << " " << (int)lower_left[0] << " " << (int)lower_left[1] << std::endl;

    std::vector<uint8_t> lower_right = pixels_data.lower_right;
    std::cout << "Coordinates of the lower-right pixel: " << (int)lower_right[2] << " " << (int)lower_right[0] << " " << (int)lower_right[1] << std::endl;
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
    print_pixels_data(pixels_data);

    return 0;
}
