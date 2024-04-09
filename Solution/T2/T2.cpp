#include <iostream>
#include <fstream>
#include <cmath>
#include <chrono>
#include <vector>

#pragma pack(2)
struct BitmapFileHeader {
    char header[2];
    unsigned int fileSize;
    unsigned int reserved;
    unsigned int dataOffset;
};

struct BitmapInfoHeader {
    unsigned int size;
    int width;
    int height;
    unsigned short planes;
    unsigned short bitCount;
    unsigned int compression;
    unsigned int imageSize;
    int xPixelsPerMeter;
    int yPixelsPerMeter;
    unsigned int colorsUsed;
    unsigned int colorsImportant;
};


int main() {
    std::string filename;
    std::cout << "Enter the BMP image filename: ";
    std::cin >> filename;

    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file) {
        std::cerr << "Could not open file." << std::endl;
        return 1;
    }

    BitmapFileHeader fileHeader;
    BitmapInfoHeader infoHeader;

    file.read(reinterpret_cast<char*>(&fileHeader), sizeof(BitmapFileHeader));
    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(BitmapInfoHeader));

    int width = infoHeader.width;
    int height = infoHeader.height;
    int pixelSize = infoHeader.bitCount / 8;
    int padding = (4 - ((width * pixelSize) % 4)) % 4;
    int row_size = std::floor((infoHeader.bitCount * width + 31) / 32) * 4;
    int colorsChannels = infoHeader.bitCount / 8;

    file.seekg(fileHeader.dataOffset, file.beg);
    char* pixels = new char[height * row_size + width * colorsChannels];
    file.read(pixels, height * row_size + width * colorsChannels);

    std::vector<double> mean(colorsChannels);
    std::vector<double> variance(colorsChannels);

    for (int k = 0; k < colorsChannels; k++) {
        mean[k] = 0;
        variance[k] = 0;
    }
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int cur_position = y * row_size + x * colorsChannels;
            for (int k = 0; k < colorsChannels; k++) {
                mean[k] += (int)(unsigned char)pixels[cur_position + k];
            }
        }

        file.seekg(padding, file.cur);
    }

    file.close();

    int totalPixels = width * height;
    for (int i = 0; i < colorsChannels; ++i) {
        mean[i] /= totalPixels;
    }

    std::ifstream fileAgain(filename, std::ios::in | std::ios::binary);
    fileAgain.read(reinterpret_cast<char*>(&fileHeader), sizeof(BitmapFileHeader));
    fileAgain.read(reinterpret_cast<char*>(&infoHeader), sizeof(BitmapInfoHeader));
    fileAgain.seekg(fileHeader.dataOffset, fileAgain.beg);

    auto start = std::chrono::steady_clock::now();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int cur_position = y * row_size + x * colorsChannels;

            for (int k = 0; k < colorsChannels; k++) {
                variance[k] += ((int)(unsigned char)pixels[cur_position + k] - mean[k]) * ((int)(unsigned char)pixels[cur_position + k] - mean[k]);
            }
        }
        fileAgain.seekg(padding, fileAgain.cur);
    }

    fileAgain.close();

    for (int i = 0; i < colorsChannels; ++i) {
        variance[i] /= totalPixels;
    }

    std::vector<double> stdDeviation(colorsChannels);
    for (int i = 0; i < colorsChannels; ++i) {
        stdDeviation[i] = std::sqrt(variance[i]);
    }

    auto end = std::chrono::steady_clock::now();
    double elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1e6;

    std::cout.setf(std::ios::fixed);
    std::cout.precision(6);

    for (int i = colorsChannels - 1; i >= 0; --i) {
        std::cout << "Mean of " << 3 - i << " channel: " << mean[i] << std::endl;
        std::cout << "Standard deviation of " << 3 - i << " channel: " << stdDeviation[i] << std::endl;
        std::cout << std::endl;
    }

    std::cout << "Time taken to calculate " << elapsedTime << " seconds" << std::endl;

    return 0;
}
