#include <iostream>
#include <fstream>
#include <string>


typedef struct BITMAPFILEHEADER {
	unsigned short    bfType;
	unsigned long   bfSize;
	unsigned short    bfReserved1;
	unsigned short    bfReserved2;
	unsigned long   bfOffBits;
};




void error() {
	std::cout << "Ошибка при вводе данных. Повторите попытку" << std::endl;
}


int main() {
	setlocale(LC_ALL, "en_US.UTF8");

	std::ofstream file("C:\\Foton\\Task1\\Files\\file.bmp",
		std::ios::binary);

	if (file.is_open()) {
		std::cout << "OK" << std::endl;
	}
	else {
		std::cout << "ERROR" << std::endl;
	}

	std::string s;

	while (std::getline(std::cin, s) && !s.empty()) {
		std::string str_x = "";
		std::string str_y = "";
		bool flag = false;
		for (auto c : s) {
			if (c == ' ') {
				flag = true;
				continue;
			}

			if (not flag) {
				str_x = str_x + c;
			}
			else {
				str_y = str_y + c;
			}			
		}

		if (str_x.size() > 0 && str_y.size() > 0) {
			int x = std::stoi(str_x);
			int y = std::stoi(str_y);
			std::cout << x << " " << y << std::endl;
			// std::fseek(file, sizeof(BITMAPFILEHEADER), 0);
		}
		else { error(); }	
	}


		file.close();
		return 0;
	}


