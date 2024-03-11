#include <iostream>
#include <fstream>
#include <vector>
#include <string>


struct PixelData {
    int B;
    int G;
    int R;
};



PixelData calculateMO(const std::string PATH) {
    std::ifstream input_file(PATH, std::ios::binary);
    if (!input_file.is_open()) {
        std::cout << "Can't open one of the files";
        input_file.close();
        exit(1);
    }

    char header[54];
    input_file.read(header, 54);
    if (header[0] != 'B' || header[1] != 'M') {
        std::cout << "The input file is not a bmp format file!" << std::endl;
        input_file.close();
        exit(1);
    }

    int pixelsOffset = *(int*)&header[10];
    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    int bitsPerPixel = *(int*)&header[28];
    int colorsChannels = bitsPerPixel / 8;
    int imageSize = width * height * colorsChannels;
    int row_size = std::floor((bitsPerPixel * width + 31) / 32) * 4;



    input_file.close();
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Enter the PATH to the input file and to the output file";
        return 1;
    }

    std::string PATH = argv[1];
    PixelData result = calculateMO(PATH);

    std::cout << result.R << std::endl;
    std::cout << result.G << std::endl;
    std::cout << result.B << std::endl;


    return 0;
}
