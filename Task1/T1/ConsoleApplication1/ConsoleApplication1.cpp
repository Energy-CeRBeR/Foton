#include <iostream>
#include <fstream>
#include <string>


void error() {
	std::cout << "Введите корректные значения x и y!" << std::endl;
}


bool isValid(const std::string& str) {
	if (str.size() == 0) {
		return false;
	}
	for (char c : str) {
		if (!std::isdigit(c)) {	// Валидация строки
			return false;
		}
	}
	return true;
}


int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "RU");

	if (argc != 2) {
		std::cout << "Введите путь к файлу";	// Если введённый путь некорректен / отсутствует
		return 1;
	}

	FILE* file = fopen(argv[1], "rb");

	if (file == NULL) {
		std::cout << "По данному пути файл не найден"; // Если не удаётся получить доступ к файлу 
		fclose(file);									// или указан неверный путь
		return 1;
	}

	if (fgetc(file) != 'B' || fgetc(file) != 'M') {
		std::cout << "Файл не является изображением формата BMP!" << std::endl;
		fclose(file);  // Проверка расширения файла
		return 1;
	}

	// Чтение служебной информации из файла
	unsigned char header[52];
	unsigned char colors[3];
	std::string s;

	fread(header, sizeof(unsigned char), 52, file);
	int width = *(int*)&header[16];
	int height = *(int*)&header[20];

	std::cout << "Размер изображения: " << width << " x " << abs(height) << std::endl;

	std::cout << "Введите значения x, y: ";
	std::string str_x = "";
	std::string str_y = "";
	while (std::getline(std::cin, s) && !s.empty()) {
		std::string str_x = "";
		std::string str_y = "";
		bool flag = false;
		for (auto c : s) {
			if (c == ' ') {
				flag = true;
				continue;     // Получение значений x и y из введённой строки
			}

			if (not flag) {
				str_x = str_x + c;
			}
			else {
				str_y = str_y + c;
			}
		}

		// Если введены корректные данные, начало работы с ними,
		// в противном случае вывод ошибки
		if (isValid(str_x) && isValid(str_y)) {
			int x = std::stoi(str_x);
			int y = std::stoi(str_y);

			if (x < 0 || y < 0) {
				std::cout << "Значение x и y должно быть неотрицательным" << std::endl;
				error();
			}

			if (x >= width || y >= height) {
				std::cout << "Введённые значения превосходят ширину или высоту изображения." 
					<< std::endl;
				error();
			}

			else {

				// Перемещение курсора в нужное положение с учётом того, 
				// что служебная информация занимает 54 байта 
				// Далее идут цвета пикселей, каждый пиксель занимает 3 байта
				// После установки курсора считываем данные в colors и выводим в обратном цикле,
				// т.к данные хранятся в формате BGR, а не RGB

				fseek(file, 54 + ((height > 0 ? (height - y - 1) : y) * width + x) * 3, 0);
				fread(colors, sizeof(unsigned char), 3, file);

				for (int i = 2; i >= 0; --i) { std::cout << (int)colors[i] << " "; }

				std::cout << std::endl;
			}
		}
		else { error(); }

		std::cout << "Введите значения x, y: ";
	}

	std::cout << "Была введена пустая строка. Окончание работы программы" << std::endl;
	fclose(file);

	return 0;
}