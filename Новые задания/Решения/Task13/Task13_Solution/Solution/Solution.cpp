#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
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


int pixelsOffset;
int width;
int height;
int new_width;
int new_height;
int colorsChannels;
int row_size;
int new_row_size;
int k;


void write_header(std::ifstream &input_file, std::ofstream &output_file, BITMAPFILEHEADER &fileHeader, BITMAPINFOHEADER &fileInfoHeader) {
    output_file.write((char*)&fileHeader, sizeof(fileHeader));
    output_file.write((char*)&fileInfoHeader, sizeof(fileInfoHeader));

    char* temp_info = new char[pixelsOffset - 54];
    input_file.seekg(54);
    input_file.read(temp_info, pixelsOffset - 54);
    output_file.write(temp_info, pixelsOffset - 54);
    delete[] temp_info;
}


void filter_image(std::ofstream& output_file, const std::vector<char> &pixels) {
    std::vector<char> new_pixels(row_size * height + width * colorsChannels);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            std::vector<double> average(colorsChannels);
            for (int a = i - k / 2; a <= i + k / 2; a++) {
                for (int b = j - k / 2; b <= j + k / 2; b++) {
                    for (int c = 0; c < colorsChannels; c++) {
                        average[c] += (int)(unsigned char)pixels[(a + k / 2) * new_row_size + (b + k / 2) * colorsChannels + c];
                    }
                }
            }

            for (int c = 0; c < colorsChannels; c++) {
                average[c] /= (double)(k * k);
                new_pixels[i * row_size + j * colorsChannels + c] = average[c];
            }
        }
    }

    output_file.write(new_pixels.data(), row_size * height + width * colorsChannels);
    output_file.close();
}


std::vector<char> create_pixels_vector_with_null(const std::vector<char> &pixels) {
    std::vector<char> new_pixels(new_row_size * new_height + new_width * colorsChannels);
    for (int i = 0; i < height + k; i++) {
        for (int j = 0; j < width + k; j++) {
            if (i < k / 2 || j < k / 2 || i >= height + k / 2 || j >= width + k / 2) {
                for (int c = 0; c < colorsChannels; c++) {
                    new_pixels[i * new_row_size + j * colorsChannels + c] = 0;
                }
            }
            else {
                for (int c = 0; c < colorsChannels; c++) {
                    new_pixels[i * new_row_size + j * colorsChannels + c] = pixels[(i - k / 2) * row_size + (j - k / 2) * colorsChannels + c];
                }
            }
        }
    }

    return new_pixels;
}


std::vector<char> create_pixels_vector_with_add_str_col(const std::vector<char>& pixels) {
    std::vector<char> new_pixels(new_row_size * new_height + new_width * colorsChannels);
    for (int i = 0; i < height + k; i++) {
        for (int j = 0; j < width + k; j++) {
            int i_2 = i - k / 2;
            int j_2 = j - k / 2;

            if (i < k / 2) {
                i_2 = 0;
            }
            if (j < k / 2) {
                j_2 = 0;
            }
            if (i >= height + k / 2) {
                i_2 = height - 1;
            }
            if (j >= width + k / 2) {
                j_2 = width - 1;
            }

            for (int c = 0; c < colorsChannels; c++) {
                new_pixels[i * new_row_size + j * colorsChannels + c] = pixels[i_2 * row_size + j_2 * colorsChannels + c];
            }
        }
    }

    return new_pixels;
}


std::vector<char> create_pixels_vector_with_add_mirror_reflection(const std::vector<char>& pixels) {
    std::vector<char> new_pixels(new_row_size * new_height + new_width * colorsChannels);
    for (int i = 0; i < height + k; i++) {
        for (int j = 0; j < width + k; j++) {
            int i_2 = i - k / 2;
            int j_2 = j - k / 2;

            if (i < k / 2) {
                i_2 = (k / 2 - i);
            }
            if (j < k / 2) {
                j_2 = (k / 2 - j);
            }
            if (i >= height + k / 2) {
                i_2 = i - k - 1;
            }
            if (j >= width + k / 2) {
                j_2 = j - k - 1;
            }

            for (int c = 0; c < colorsChannels; c++) {
                new_pixels[i * new_row_size + j * colorsChannels + c] = pixels[i_2 * row_size + j_2 * colorsChannels + c];
            }
        }
    }

    return new_pixels;
}


void medianFilter(const std::string INPUT_PATH, const std::string OUTPUT_PATH) {
    std::ifstream input_file(INPUT_PATH, std::ios::binary);

    if (!(input_file.is_open())) {
        std::cout << "Can't open input_file";
        input_file.close();
        exit(1);
    }

    BITMAPFILEHEADER fileHeader;
    input_file.read((char*)&fileHeader, sizeof(fileHeader));

    BITMAPINFOHEADER fileInfoHeader;
    input_file.read((char*)&fileInfoHeader, sizeof(fileInfoHeader));

    pixelsOffset = fileHeader.bfOffBits;
    colorsChannels = fileInfoHeader.biBitCount / 8;
    width = fileInfoHeader.biWidth;
    height = fileInfoHeader.biHeight;
    new_width = width + k;
    new_height = height + k;
    row_size = std::floor((fileInfoHeader.biBitCount * width + 31) / 32) * 4;
    new_row_size = std::floor((fileInfoHeader.biBitCount * new_width + 31) / 32) * 4;

    std::string OUTPUTPATH_FOR_FILTER_WITH_ADD_NULL = "__add_null__" + OUTPUT_PATH;
    std::string OUTPUTPATH_FOR_FILTER_WITH_ADD_STR_COL = "__add_str_col__" + OUTPUT_PATH;
    std::string OUTPUTPATH_FOR_FILTER_WITH_ADD_MIRROR_REFLECTION = "__add_mirror_reflection__" + OUTPUT_PATH;

    std::ofstream output_file_1(OUTPUTPATH_FOR_FILTER_WITH_ADD_NULL, std::ios::binary);
    std::ofstream output_file_2(OUTPUTPATH_FOR_FILTER_WITH_ADD_STR_COL, std::ios::binary);
    std::ofstream output_file_3(OUTPUTPATH_FOR_FILTER_WITH_ADD_MIRROR_REFLECTION, std::ios::binary);

    if (!(output_file_1.is_open() && output_file_2.is_open() && output_file_3.is_open())) {
        std::cout << "Can't open one of output_files";
        input_file.close();
        output_file_1.close();
        output_file_2.close();
        output_file_3.close();
        exit(1);
    }

    write_header(input_file, output_file_1, fileHeader, fileInfoHeader);
    write_header(input_file, output_file_2, fileHeader, fileInfoHeader);
    write_header(input_file, output_file_3, fileHeader, fileInfoHeader);

    std::vector<char> pixels(row_size * height + width * colorsChannels);
    input_file.read(pixels.data(), row_size * height + width * colorsChannels);

    input_file.close();

    auto start_time_1 = std::chrono::high_resolution_clock::now();
    std::vector<char> special_pixels_1 = create_pixels_vector_with_null(pixels);
    filter_image(output_file_1, special_pixels_1);
    auto end_time_1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_1 = end_time_1 - start_time_1;
    std::cout << "The filter using add zero_pixels was successfully applied in " << duration_1.count() << " seconds" << std::endl;

    auto start_time_2 = std::chrono::high_resolution_clock::now();
    std::vector<char> special_pixels_2 = create_pixels_vector_with_add_str_col(pixels);
    filter_image(output_file_2, special_pixels_2);
    auto end_time_2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_2 = end_time_2 - start_time_2;
    std::cout << "The filter using add copy strings and columns was successfully applied in " << duration_2.count() << " seconds" << std::endl;

    auto start_time_3 = std::chrono::high_resolution_clock::now();
    std::vector<char> special_pixels_3 = create_pixels_vector_with_add_mirror_reflection(pixels);
    filter_image(output_file_3, special_pixels_3);
    auto end_time_3 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_3 = end_time_3 - start_time_3;
    std::cout << "The filter using add mirror reflection was successfully applied in " << duration_3.count() << " seconds" << std::endl;
}


int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Enter the PATH to the input file, to the output file, the rotate angle and size of tile";
        return 1;
    }

    std::string INPUT_PATH = argv[1];
    std::string OUTPUT_PATH = argv[2];
    std::string str_k = argv[3];
    k = std::stoi(str_k);
       
    medianFilter(INPUT_PATH, OUTPUT_PATH);

    return 0;
}