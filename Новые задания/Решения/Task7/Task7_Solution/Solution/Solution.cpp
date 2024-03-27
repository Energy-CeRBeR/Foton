#include <iostream>
#include <fstream>
#include <vector>
#include <string>


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


void averaging_image(const std::string INPUT_PATH, const std::string OUTPUT_PATH, int n) {
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

    long pixelsOffset = 54 + fileHeader.bfOffBits - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER);
    int colorsChannels = fileInfoHeader.biBitCount / 8;
    int width = fileInfoHeader.biWidth;
    int height = fileInfoHeader.biHeight;
    int row_size = std::floor((fileInfoHeader.biBitCount * width + 31) / 32) * 4;

    int new_width = width / n;
    int new_height = height / n;
    int new_row_size = std::floor((fileInfoHeader.biBitCount * new_width + 31) / 32) * 4;

    fileInfoHeader.biWidth = new_width;
    fileInfoHeader.biHeight = new_height;
    fileInfoHeader.biSizeImage = new_width * colorsChannels + (new_height - 1) * new_row_size;
    
    output_file.write((char*)&fileHeader, sizeof(fileHeader));
    output_file.write((char*)&fileInfoHeader, sizeof(fileInfoHeader));

    char* temp_info = new char[pixelsOffset - 54];
    input_file.read(temp_info, pixelsOffset - 54);
    output_file.write(temp_info, pixelsOffset - 54);
    delete[] temp_info;

    std::vector<char> new_row(new_row_size);
    std::vector<char> row(row_size);
    std::cout << "YEEEEE" << std::endl;
    int x = 0;
    for (int i = n / 2; i < height; i += n) {
        x++;
        int y = 0;
        input_file.seekg(pixelsOffset + (height <= 0 ? (height - i - 1) : i) * row_size);
        input_file.read(new_row.data(), new_row_size);
        input_file.seekg(pixelsOffset + (height <= 0 ? (height - i - 1) : i) * row_size);
        input_file.read(row.data(), row_size);

        for (int j = n / 2; j < width; j += n) {
            y++;
            new_row[y * colorsChannels] = row[j * colorsChannels];
            // Скорее всего он не успеват записать все пиксели!!!

            /*char* pixel = new char[colorsChannels];
            input_file.read(pixel, colorsChannels);
            output_file.write(pixel, colorsChannels);
            delete[] pixel;*/

            //row[j * colorsChannels] = 255 - row[j * colorsChannels];
            //new_row[x * colorsChannels] = row[j * colorsChannels];
        }
        output_file.write(new_row.data(), new_row_size);
    }
    
   
    std::cout << "The negative image has been successfully created!" << std::endl;

    input_file.close();
    output_file.close();
}


int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Enter the PATH to the input file and to the output file";
        return 1;
    }

    std::string INPUT_PATH = argv[1];
    std::string OUTPUT_PATH = argv[2];
    std::string str_num = argv[3];
    int num = std::stoi(str_num);
    averaging_image(INPUT_PATH, OUTPUT_PATH, num);


    return 0;
}