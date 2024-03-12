#include <iostream>
#include <fstream>
#include <vector>
#include <string>



void create_negative_file(const std::string INPUT_PATH, const std::string OUTPUT_PATH) {

    std::ifstream input_file(INPUT_PATH, std::ios::binary);
    std::ofstream output_file(OUTPUT_PATH, std::ios::binary);

    if (!(input_file.is_open() && output_file.is_open())) {
        std::cout << "Can't open one of the files";
        input_file.close();  
        output_file.close(); 
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

    char* temp_info = new char[pixelsOffset - 54];
    input_file.read(temp_info, pixelsOffset - 54);
    output_file.write(header, 54); 
    output_file.write(temp_info, pixelsOffset - 54); 
    delete[] temp_info;

    int row_size = std::floor((bitsPerPixel * width + 31) / 32) * 4;
    std::vector<char> row(row_size);
    
    std::vector<unsi

    for (int i = 0; i < height; i++) {
        input_file.read(row.data(), row_size); 
        for (int j = 0; j < width; j++) {
            for (int k = 0; k < colorsChannels; k++) {
                std::swap(row[j * colorsChannels + k]
            }
            
        }
        output_file.write(row.data(), row_size);
    }

    std::cout << "The negative image has been successfully created!" << std::endl;

    input_file.close();
    output_file.close();
}


int main(int argc, char* argv[]) {
    
    if (argc != 3) {
        std::cout << "Enter the PATH to the input file and to the output file";
        return 1;
    }

    std::string INPUT_PATH = argv[1];
    std::string OUTPUT_PATH = argv[2];
    create_negative_file(INPUT_PATH, OUTPUT_PATH);


    return 0;
}
