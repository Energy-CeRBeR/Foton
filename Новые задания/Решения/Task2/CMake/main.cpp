#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>


struct RPCData {
	double LINE_OFF;
	double SAMP_OFF;
	double LAT_OFF;
	double LONG_OFF;
	double HEIGHT_OFF;
	double LINE_SCALE;
	double SAMP_SCALE;
	double LAT_SCALE;
	double LONG_SCALE;
	double HEIGHT_SCALE;
	std::vector<double> LINE_NUM_COEFF;
	std::vector<double> LINE_DEN_COEFF;
	std::vector<double> SAMP_NUM_COEFF;
	std::vector<double> SAMP_DEN_COEFF;
};


struct GeodeticCoords {
	double latitude;
	double longitude;
	double height;
};


struct PixelCoords {
	double column;
	double row;
};


std::vector<std::pair<std::string, double>> splitString(const std::string& str) {
	std::vector<std::pair<std::string, double>> result;
	std::istringstream iss(str);

	std::string data_type;
	iss >> data_type;

	char sign;
	iss >> sign;

	std::string str_value;
	iss >> str_value;
	double value;
	try {
		value = std::stod(str_value);
	}
	catch (...) {
		std::cout << "Incorrect data format in the file!";
		exit(1);
	}

	if (sign == '-') {
		value = -value;
	}

	result.push_back(std::make_pair(data_type, value));
	return result;
}


RPCData readRPC(const std::string& PATH) {
	RPCData rpc_data;

	std::ifstream file(PATH);
	if (!file) {
		std::cerr << "Failed to open RPC file: " << std::endl;
		file.close();
		exit(1);
	}

	std::vector<double> LINE_NUM_COEFF(20);
	std::vector<double> LINE_DEN_COEFF(20);
	std::vector<double> SAMP_NUM_COEFF(20);
	std::vector<double> SAMP_DEN_COEFF(20);

	std::string line;
	while (getline(file, line)) {
		std::vector<std::pair<std::string, double>> current_data = splitString(line);
		std::string data_type = current_data[0].first;
		double value = current_data[0].second;
		
		if (std::isdigit(line.back())) {
			int index_pos = data_type.rfind("_");
			int colon_pos = data_type.rfind(":");
			std::string index_str = data_type.substr(index_pos + 1, colon_pos - index_pos - 1);
			int index;
			try {
				index = std::stoi(index_str);
			}
			catch (...) {
				std::cout << "Incorrect data format in the file!";
				exit(1);
			}
			
			std::string name = data_type.substr(0, index_pos);

			if (name == "LINE_NUM_COEFF") {
				LINE_NUM_COEFF[index - 1] = value;
			}
			else if (name == "LINE_DEN_COEFF") {
				LINE_DEN_COEFF[index - 1] = value;
			}
			else if (name == "SAMP_NUM_COEFF") {
				SAMP_NUM_COEFF[index - 1] = value;
			}
			else if (name == "SAMP_DEN_COEFF") {
				SAMP_DEN_COEFF[index - 1] = value;
			}
		}

		else {
			if (data_type == "LINE_OFF:") {
				rpc_data.LINE_OFF = value;
			}
			else if (data_type == "SAMP_OFF:") {
				rpc_data.SAMP_OFF = value;
			}
			else if (data_type == "LAT_OFF:") {
				rpc_data.LAT_OFF = value;
			}
			else if (data_type == "LONG_OFF:") {
				rpc_data.LONG_OFF = value;
			}
			else if (data_type == "HEIGHT_OFF:") {
				rpc_data.HEIGHT_OFF = value;
			}
			else if (data_type == "LINE_SCALE:") {
				rpc_data.LINE_SCALE = value;
			}
			else if (data_type == "SAMP_SCALE:") {
				rpc_data.SAMP_SCALE = value;
			}
			else if (data_type == "LAT_SCALE:") {
				rpc_data.LAT_SCALE = value;
			}
			else if (data_type == "LONG_SCALE:") {
				rpc_data.LONG_SCALE = value;
			}
			else if (data_type == "HEIGHT_SCALE:") {
				rpc_data.HEIGHT_SCALE = value;
			}
		}	
	}

	rpc_data.LINE_NUM_COEFF = LINE_NUM_COEFF;
	rpc_data.LINE_DEN_COEFF = LINE_DEN_COEFF;
	rpc_data.SAMP_NUM_COEFF = SAMP_NUM_COEFF;
	rpc_data.SAMP_DEN_COEFF = SAMP_DEN_COEFF;

	file.close();
	return rpc_data;
}


double get_PLHsum(const std::vector<double>& LINE, double P, double L, double H) {
	double result;
	result = (
		LINE[0] + LINE[1] * L + LINE[2] * P + LINE[3] * H +
		LINE[4] * L * P + LINE[5] * L * H + LINE[6] * P * H + LINE[7] * L * L + 
		LINE[8] * P * P + LINE[9] * H * H +  LINE[10] * P * L * H + LINE[11] * L * L * L + 
		LINE[12] * L * P * P + LINE[13] * L * H * H + LINE[14] * L * L * P + LINE[15] * P * P * P + 
		LINE[16] * P * H * H + LINE[17] * L * L * H + LINE[18] * P * P * H + LINE[19] * H * H * H
		);

	return result;
}


PixelCoords calculatePixelCoords(const std::string& PATH, const GeodeticCoords& geodeticCoords, const RPCData& rpc_data) {
	std::ifstream file(PATH);
	if (!file) {
		std::cerr << "Failed to open RPC file: " << std::endl;
		file.close();
		exit(1);
	}

	PixelCoords pixel_coords;
	double P, L, H;
	P = (geodeticCoords.latitude - rpc_data.LAT_OFF) / rpc_data.LAT_SCALE;
	L = (geodeticCoords.longitude - rpc_data.LONG_OFF) / rpc_data.LONG_SCALE;
	H = (geodeticCoords.height - rpc_data.HEIGHT_OFF) / rpc_data.HEIGHT_SCALE;

	std::vector<double> LINE_NUM_COEFF = rpc_data.LINE_NUM_COEFF;
	std::vector<double> LINE_DEN_COEFF = rpc_data.LINE_DEN_COEFF;
	std::vector<double> SAMP_NUM_COEFF = rpc_data.SAMP_NUM_COEFF;
	std::vector<double> SAMP_DEN_COEFF = rpc_data.SAMP_DEN_COEFF;

	double s1, s2, s3, s4;
	s1 = get_PLHsum(LINE_NUM_COEFF, P, L, H);
	s2 = get_PLHsum(LINE_DEN_COEFF, P, L, H);
	s3 = get_PLHsum(SAMP_NUM_COEFF, P, L, H);
	s4 = get_PLHsum(SAMP_DEN_COEFF, P, L, H);

	double r_n, c_n;
	r_n = s1 / s2;
	c_n = s3 / s4;

	double row, column;
	row = r_n * rpc_data.LINE_SCALE + rpc_data.LINE_OFF;
	column = c_n * rpc_data.SAMP_SCALE + rpc_data.SAMP_OFF;

	pixel_coords.row = row;
	pixel_coords.column = column;
	
	file.close();
	return pixel_coords;
}


void printRPC(const RPCData& rpc_data) {
	std::cout << "RPC" << std::endl;
	std::cout << "LINE_OFF: " << rpc_data.LINE_OFF<< std::endl;
	std::cout << "SAMP_OFF: " << rpc_data.SAMP_OFF<< std::endl;
	std::cout << "LAT_OFF: " << rpc_data.LAT_OFF<< std::endl;
	std::cout << "LONG_OFF: " << rpc_data.LONG_OFF<< std::endl;
	std::cout << "HEIGHT_OFF: " << rpc_data.HEIGHT_OFF<< std::endl;
	std::cout << "LINE_SCALE: " << rpc_data.LINE_SCALE<< std::endl;
	std::cout << "SAMP_SCALE: " << rpc_data.SAMP_SCALE<< std::endl;
	std::cout << "LAT_SCALE: " << rpc_data.LAT_SCALE<< std::endl;
	std::cout << "LONG_SCALE: " << rpc_data.LONG_SCALE<< std::endl;
	std::cout << "HEIGHT_SCALE: " << rpc_data.HEIGHT_SCALE<< std::endl;
	std::cout << std::endl;
}


void printCoords(const PixelCoords& pixel_coords) {
	std::cout << "Pixel Coords" << std::endl;
	std::cout << "Column: " << pixel_coords.column << std::endl;
	std::cout << "Row: " << pixel_coords.row << std::endl;
}


int main(int argc, char* argv[]) {
	std::cout << std::fixed << std::setprecision(10);

	if (argc != 5) {
		std::cerr << "Input format: <exe_file> <Latitude> <Longitude> <Height> <RPC_File>" << std::endl;
		return 1;
	}

	std::string PATH = argv[4];
	RPCData rpc_data = readRPC(PATH);

	GeodeticCoords geodetic_coords;
	try {
		geodetic_coords.latitude = std::stod(argv[1]);
		geodetic_coords.longitude = std::stod(argv[2]);
		geodetic_coords.height = std::stod(argv[3]);
	}
	catch (...) {
		std::cout << "Incorrect data format in the file!";
		return 1;
	}

	PixelCoords pixel_coords;
	pixel_coords = calculatePixelCoords(PATH, geodetic_coords, rpc_data);

	printRPC(rpc_data);
	printCoords(pixel_coords);

	return 0;
}