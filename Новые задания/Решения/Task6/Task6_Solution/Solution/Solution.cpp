#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cmath>
#include <algorithm>


struct Answer {
    std::vector<double> mean;
    std::vector<double> cko;
};


Answer calculate_MO_CKO(const std::string PATH) {
    std::ifstream file(PATH, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "The file cannot be opened using this path";
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
    int imageSize = width * height * colorsChannels;
    int row_size = std::floor((bitsPerPixel * width + 31) / 32) * 4;

    char* pixels = new char[imageSize];
    file.seekg(pixelsOffset);
    file.read(pixels, imageSize);
    file.close();

    std::vector<double> mean(colorsChannels);
    std::vector<double> square_mean(colorsChannels);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int offset = (x * colorsChannels) + (y * row_size);
            for (int k = 0; k < colorsChannels; k++) {
                mean[k] += (int)(unsigned char)pixels[offset + k];
                square_mean[k] += (int)(unsigned char)pixels[offset + k] * (int)(unsigned char)pixels[offset + k];
            }
        }
    }

    for (int k = colorsChannels - 1; k >= 0; k--) {
        mean[k] /= (double)(width * height);
        square_mean[k] /= double(width * height);
    }

    std::vector<double> standart_devation(colorsChannels);
    for (int k = 0; k < colorsChannels; k++) {
        standart_devation[k] = std::sqrt(square_mean[k] - mean[k] * mean[k]);
    }

    delete[] pixels;

    Answer result;
    result.mean = mean;
    result.cko = standart_devation;


    return result;
}


void print_result(const Answer& answer) {
    std::vector<double> mean = answer.mean;
    std::vector<double> cko = answer.cko;
    std::reverse(mean.begin(), mean.end());
    std::reverse(cko.begin(), cko.end());

    std::cout << "\t";
    for (int i = 0; i < mean.size(); i++) {
        std::cout << "Channel " << i + 1 << "\t";
    }
    std::cout << std::endl;

    std::cout << "MO: ";
    for (auto num : mean) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    std::cout << "CKO: ";
    for (auto num : cko) {
        std::cout << num << " ";
    }
}


int main(int argc, char* argv[]) {
    std::cout << std::fixed << std::setprecision(10);
    if (argc != 2) {
        std::cout << "You need to enter the path to the file";
        return 1;
    }

    std::string PATH = argv[1];
    Answer answer = calculate_MO_CKO(PATH);
    print_result(answer);

    return 0;
}