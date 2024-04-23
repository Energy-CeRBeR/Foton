#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>

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
    std::vector<char> pixels(row_size * height + width * colorsChannels);
    input_file.read(pixels.data(), row_size * height + width * colorsChannels);

    std::vector<char> new_pixels(new_row_size * new_height + new_width * colorsChannels);

    auto startTime = std::chrono::high_resolution_clock::now();
    
    double scal_x = (double)new_width / width;
    double scal_y = (double)new_height / height;
    int pre_i, pre_j, after_i, after_j; 
    for (int i = 0; i < new_height; i++) {
        for (int j = 0; j < new_width; j++) {
            for (int k = 0; k < colorsChannels; k++) {
                pre_i = (int)(i / scal_y);
                pre_j = (int)(j / scal_x);
                if (pre_i >= 0 && pre_i < height && pre_j >= 0 && pre_j < width) {
                    new_pixels[i * new_row_size + j * colorsChannels + k] = pixels[pre_i * row_size + pre_j * colorsChannels + k];
                }
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = endTime - startTime;

    output_file.write(new_pixels.data(), new_row_size * new_height + new_width * colorsChannels);

    std::cout << "New image has been successfully created using scale_nn for " << duration.count() << " seconds" << std::endl;

    input_file.close();
    output_file.close();
}



void scale_bln(const std::string INPUT_PATH, const std::string OUTPUT_PATH, int new_width, int new_height) {
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
    std::vector<char> pixels(row_size * height + width * colorsChannels);
    input_file.read(pixels.data(), row_size * height + width * colorsChannels);

    std::vector<char> new_pixels(new_row_size * new_height + new_width * colorsChannels);

    auto startTime = std::chrono::high_resolution_clock::now();

    double scal_x = (double)new_width / width;
    double scal_y = (double)new_height / height;
    for (int i = 0; i < new_height; i++) {
        for (int j = 0; j < new_width; j++) {
            double x = (double)j / scal_x;
            double y = (double)i / scal_y;

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

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = endTime - startTime;

    output_file.write(new_pixels.data(), new_row_size * new_height + new_width * colorsChannels);
    std::cout << "New image has been successfully created using scale_bln for " << duration.count() << " seconds" <<  std::endl;

    input_file.close();
    output_file.close();
}


int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cout << "Enter the PATH to the input file, to the output file, new width and new height";
        return 1;
    }

    std::string INPUT_PATH = argv[1];
    std::string OUTPUT_PATH = argv[2];
    std::string OUTPUTPATH_FOR_SCALE_NN = "_scale_nn_" + OUTPUT_PATH;
    std::string OUTPUTPATH_FOR_SCALE_BLN = "_scale_bln_" + OUTPUT_PATH;
    std::string str_width = argv[3];
    std::string str_height = argv[4];
    int new_width = std::stoi(str_width);
    int new_height = std::stoi(str_height);

    scale_nn(INPUT_PATH, OUTPUTPATH_FOR_SCALE_NN, new_width, new_height);
    scale_bln(INPUT_PATH, OUTPUTPATH_FOR_SCALE_BLN, new_width, new_height);


    return 0;
}