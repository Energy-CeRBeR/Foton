#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
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


struct Data {
    int k;
    double S;
    double M;
    double O;
    std::vector<std::vector<double>> matrix;
};


int pixelsOffset;
int width;
int height;
int new_width;
int new_height;
int colorsChannels;
int row_size;
int new_row_size;
Data data;


void generateGaussianMatrix(int k, double S) {
    int matrix_size = k;
    std::vector<std::vector<double>> gaussian_matrix(matrix_size, std::vector<double>(matrix_size));
    int center = k / 2;
    for (int i = 0; i < matrix_size; ++i) {
        for (int j = 0; j < matrix_size; ++j) {
            gaussian_matrix[i][j] = 1.0 / (2 * 3.14159 * S * S) * exp(-((i - center) * (i - center) + (j - center) * (j - center)) / (2 * S * S));
        }
    }
    data.matrix = gaussian_matrix;
}


void printMatrix() {
    for (const auto& row : data.matrix) {
        for (double element : row) {
            std::cout << element << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}


void normalizeGaussianMatrix() {
    double sum = 0.0;
    for (const auto& row : data.matrix) {
        for (double element : row) {
            sum += element;
        }
    }

    if (data.k < 15) {
        std::cout << "The sum of the elements of an unnormalized Gaussian matrix: " << sum << std::endl << std::endl;

    }

    for (auto& row : data.matrix) {
        for (double& element : row) {
            element /= sum;
        }
    }
}


void saveDataToFile() {
    std::ostringstream filename;
    filename << "data_" << data.k << "_" << data.S << "_" << ".txt";
    std::ofstream file(filename.str());

    file << "k = " << data.k << std::endl;
    file << "S = " << data.S << std::endl;
    file << "M = " << data.M << std::endl;
    file << "O = " << data.O << std::endl;
    file << std::endl;

    file << "Коэффициенты матрицы A размерностью kxk:" << std::endl;
    for (const auto& row : data.matrix) {
        for (double element : row) {
            file << element << " ";
        }
        file << std::endl;
    }
    file << std::endl;

    file << "# ИНФОРМАЦИЯ ПО РАБОТЕ С ФАЙЛОМ #" << std::endl;
    file << "Разрешается менять только значения коэффициентов k, M, O и значения коэффициентов матрицы A размерностью kxk." << std::endl;
    file << "При нарушении структуры файла / некорректном вводе значений программа перестанет корректно работать" << std::endl;

    std::cout << "The data has been successfully saved to a file!" << std::endl;

    file.close();
}


void writeHeader(std::ifstream& input_file, std::ofstream& output_file, BITMAPFILEHEADER& fileHeader, BITMAPINFOHEADER& fileInfoHeader) {
    output_file.write((char*)&fileHeader, sizeof(fileHeader));
    output_file.write((char*)&fileInfoHeader, sizeof(fileInfoHeader));

    char* temp_info = new char[pixelsOffset - 54];
    input_file.seekg(54);
    input_file.read(temp_info, pixelsOffset - 54);
    output_file.write(temp_info, pixelsOffset - 54);
    delete[] temp_info;
}


void filterImage(std::ofstream& output_file, const std::vector<char>& pixels) {
    std::vector<char> new_pixels(row_size * height + width * colorsChannels);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            std::vector<double> average(colorsChannels);
            for (int a = i - data.k / 2; a <= i + data.k / 2; a++) {
                for (int b = j - data.k / 2; b <= j + data.k / 2; b++) {
                    for (int c = 0; c < colorsChannels; c++) {
                        average[c] += data.matrix[a - i + data.k / 2][b - j + data.k / 2] * (int)(unsigned char)pixels[(a + data.k / 2) * new_row_size + (b + data.k / 2) * colorsChannels + c];
                    }
                }
            }

            for (int c = 0; c < colorsChannels; c++) {
                average[c] = data.M * average[c] + data.O;
                average[c] = std::min(std::max(average[c], 0.0), 255.0);
                new_pixels[i * row_size + j * colorsChannels + c] = static_cast<char>(round(average[c]));
            }
        }
    }

    output_file.write(new_pixels.data(), row_size * height + width * colorsChannels);
    output_file.close();
}


std::vector<char> createPixelsVector(const std::vector<char>& pixels) {
    std::vector<char> new_pixels(new_row_size * new_height + new_width * colorsChannels);
    for (int i = 0; i < height + data.k; i++) {
        for (int j = 0; j < width + data.k; j++) {
            int i_2 = i - data.k / 2;
            int j_2 = j - data.k / 2;

            if (i < data.k / 2) {
                i_2 = (data.k / 2 - i);
            }
            if (j < data.k / 2) {
                j_2 = (data.k / 2 - j);
            }
            if (i >= height + data.k / 2) {
                i_2 = i - data.k - 1;
            }
            if (j >= width + data.k / 2) {
                j_2 = j - data.k - 1;
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
    new_width = width + data.k;
    new_height = height + data.k;
    row_size = std::floor((fileInfoHeader.biBitCount * width + 31) / 32) * 4;
    new_row_size = std::floor((fileInfoHeader.biBitCount * new_width + 31) / 32) * 4;

    std::ofstream output_file(OUTPUT_PATH, std::ios::binary);
    if (!(output_file.is_open())) {
        std::cout << "Can't open one of output_files";
        input_file.close();
        output_file.close();
        exit(1);
    }

    writeHeader(input_file, output_file, fileHeader, fileInfoHeader);

    std::vector<char> pixels(row_size * height + width * colorsChannels);
    input_file.read(pixels.data(), row_size * height + width * colorsChannels);

    input_file.close();

    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<char> special_pixels = createPixelsVector(pixels);
    filterImage(output_file, special_pixels);
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;
    std::cout << "The filter was successfully applied in " << duration.count() << " seconds" << std::endl;
}


int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cout << "Enter the PATH to the input file, to the output file, the filter size k and tke CKO S";
        return 1;
    }

    std::string INPUT_PATH = argv[1];
    std::string OUTPUT_PATH = argv[2];
    std::string FILE_PATH = argv[3];
    std::string str_k = argv[3];
    std::string str_S = argv[4];

    data.k = std::stoi(str_k);
    data.S = std::stoi(str_S);
    data.M = 1;
    data.O = 0;

    generateGaussianMatrix(data.k, data.S);
    if (data.k < 15) {
        std::cout << "Gaussian matrix:" << std::endl;
        printMatrix();
    }
    normalizeGaussianMatrix();
    if (data.k < 15) {
        std::cout << "Normalize Gaussian matrix:" << std::endl;
        printMatrix();
    }

    medianFilter(INPUT_PATH, OUTPUT_PATH);
    saveDataToFile();

    return 0;
}