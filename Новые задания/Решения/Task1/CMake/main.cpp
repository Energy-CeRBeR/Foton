#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <iomanip>


struct Data {
	std::string name;
	double percentage;
};


std::string generateName() {
	std::string name;
	int size = rand() % 10 + 1;
	for (int i = 0; i < size; i++) {
		char c = 'a' + rand() % 26;
		name.push_back(c);
	}

	return name;
}


double generatePercentage() {
	static std::mt19937 randomGenerator(std::random_device{}());
	double percentage =  std::uniform_real_distribution<>(0.0, 1.0)(randomGenerator);

	return percentage;
}


void fillarr(std::vector<Data>& arr) {
	for (size_t i = 0; i < arr.size(); ++i) {
		arr[i].name = generateName();
		arr[i].percentage = generatePercentage();
	}
}


bool sortParams(const Data arr1, const Data arr2) {
	return arr1.percentage < arr2.percentage;
}


void sortarr(std::vector<Data>& arr) {
	std::sort(arr.begin(), arr.end(), sortParams);
}


double sortingTimer(std::vector<Data>& arr, int seed) {
	std::mt19937 randomGenerator(seed);
	fillarr(arr);

	auto startTime = std::chrono::high_resolution_clock::now();
	sortarr(arr);
	auto endTime = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double> duration = endTime - startTime;
	return duration.count();
}


int main(int argc, char* argv[]) {
	std::vector<int> seeds = { 123, 456, 789 };
	std::vector<int> arr_sizes = { 5000, 10000, 15000, 20000, 25000, 30000, 35000 };

	std::cout << "\t\t\tSorting time, sec\t\t\t\t\t" << std::endl;
	std::cout << "---------+------------------------------------------------+------------+\n";
	std::cout << "Size" << "\t |\t\t\t" << "Seed" << "\t\t\t  |   Average  |" << std::endl;
	std::cout << "---------+------------------------------------------------+------------+\n";

	for (auto size : arr_sizes) {
		std::vector<Data> arr(size);
		double totalTime = 0.0;
		std::cout << size << "\t |";

		for (auto seed : seeds) {
			double sorting_time = sortingTimer(arr, seed);
			std::cout << "\t" << std::setprecision(2) << std::fixed << sorting_time << "\t  |";
			totalTime += sorting_time;
		}

		double average_time = totalTime / 3;
		std::cout << "    " << std::setprecision(2) << std::fixed << average_time << "    |" << std::endl;
		std::cout << "---------+----------------+---------------+---------------+------------+\n";
	}

	return 0;
}