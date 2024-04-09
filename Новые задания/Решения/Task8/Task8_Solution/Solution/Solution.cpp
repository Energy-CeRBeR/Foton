#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
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


void scale_nn(const std::string INPUT_PATH, const std::string OUTPUT_PATH, int new_width, int new_height) {
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

    int new_row_size = std::floor((fileInfoHeader.biBitCount * new_width + 31) / 32) * 4;

    fileInfoHeader.biWidth = new_width;
    fileInfoHeader.biHeight = new_height;

    output_file.write((char*)&fileHeader, sizeof(fileHeader));
    output_file.write((char*)&fileInfoHeader, sizeof(fileInfoHeader));

    char* temp_info = new char[pixelsOffset - 54];
    input_file.read(temp_info, pixelsOffset - 54);
    output_file.write(temp_info, pixelsOffset - 54);
    delete[] temp_info;


    std::vector<char> row(row_size);
    std::vector<char> new_pixels(new_row_size * new_height + (new_width + 1) * colorsChannels);







    std::cout << "New image has been successfully created using thinning for " << std::endl;

    input_file.close();
    output_file.close();
}


void scale_bln(const std::string INPUT_PATH, const std::string OUTPUT_PATH, int n) {
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

    int new_width = width / n;
    int new_height = height / n;
    int new_row_size = std::floor((fileInfoHeader.biBitCount * new_width + 31) / 32) * 4;

    fileInfoHeader.biWidth = new_width;
    fileInfoHeader.biHeight = new_height;

    output_file.write((char*)&fileHeader, sizeof(fileHeader));
    output_file.write((char*)&fileInfoHeader, sizeof(fileInfoHeader));

    char* temp_info = new char[pixelsOffset - 54];
    input_file.read(temp_info, pixelsOffset - 54);
    output_file.write(temp_info, pixelsOffset - 54);
    delete[] temp_info;

    auto startTime = std::chrono::high_resolution_clock::now();

    std::vector<char> row(row_size);
    std::vector<char> new_pixels(new_row_size * new_height + new_width * colorsChannels);
    std::vector<char> pixels(row_size * height + width * colorsChannels);
    input_file.read(pixels.data(), row_size * height + width * colorsChannels);








    std::cout << "New image has been successfully created using averaging for " << std::endl;

    input_file.close();
    output_file.close();
}


int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cout << "Enter the PATH to the input file, to the output file and <n>";
        return 1;
    }

    std::string INPUT_PATH = argv[1];
    std::string OUTPUT_PATH = argv[2];
    std::string OUTPUTPATH_FOR_THINNING = "thinning_" + OUTPUT_PATH;
    std::string OUTPUTPATH_FOR_AVERAGING = "averaging_" + OUTPUT_PATH;
    std::string str_width = argv[3];
    std::string str_height = argv[4];
    int new_width = std::stoi(str_width);
    int new_height = std::stoi(str_height);

    scale_nn(INPUT_PATH, OUTPUTPATH_FOR_THINNING, new_width, new_height);
    //scale_bln(INPUT_PATH, OUTPUTPATH_FOR_AVERAGING, new_width, new_height);


    return 0;
}