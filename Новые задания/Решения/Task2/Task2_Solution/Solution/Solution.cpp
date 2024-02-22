#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>


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

	std::string data_type;
	std::istringstream iss(str);
	iss >> data_type;

	char colon;
	iss >> colon;

	char sign;
	iss >> sign;

	double value;
	iss >> value;

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

    std::string line;
	while (getline(file, line)) {
		std::vector<std::pair<std::string, double>> current_data = splitString(line);
		std::string data_type = current_data[0].first;
		double value = current_data[0].second;
		
		if (data_type == "LINE_OFF") {
			rpc_data.LINE_OFF = value;
		}
		else if (data_type == "SAMP_OFF") {
			rpc_data.SAMP_OFF = value;
		}
		else if (data_type == "LAT_OFF") {
			rpc_data.LAT_OFF = value;
		}
		else if (data_type == "LONG_OFF") {
			rpc_data.LONG_OFF = value;
		}
		else if (data_type == "HEIGHT_OFF") {
			rpc_data.HEIGHT_OFF = value;
		}
		else if (data_type == "LINE_SCALE") {
			rpc_data.LINE_SCALE = value;
		}
		else if (data_type == "SAMP_SCALE") {
			rpc_data.SAMP_SCALE = value;
		}
		else if (data_type == "LAT_SCALE") {
			rpc_data.LAT_SCALE = value;
		}
		else if (data_type == "LONG_SCALE") {
			rpc_data.LONG_SCALE = value;
		}
		else if (data_type == "HEIGHT_SCALE") {
			rpc_data.HEIGHT_SCALE = value;
		}
	}

	file.close();
	return rpc_data;
}


PixelCoords calculatePixelCoords(const std::string& PATH, const GeodeticCoords& geodeticCoords, const RPCData& rpcData) {
	PixelCoords pixel_coords;
	double P, L, H;
	P = (geodeticCoords.latitude - rpcData.LAT_OFF) / rpcData.LAT_SCALE;
	L = (geodeticCoords.longitude - rpcData.LONG_OFF) / rpcData.LONG_SCALE;
	H = (geodeticCoords.height - rpcData.LAT_OFF) / rpcData.LAT_SCALE;

	std::ifstream file(PATH);

	if (!file) {
		std::cerr << "Failed to open RPC file: " << std::endl;
		file.close();
		exit(1);
	}

	std::vector<double> LINE_NUM_COEFF;
	std::vector<double> LINE_DEN_COEFF;
	std::vector<double> SAMP_NUM_COEFF;
	std::vector<double> SAMP_DEN_COEFF;

	std::string line;
	int k = 0;
	while (k <= 10) {
		k++;
		getline(file, line);
	}

	while (getline(file, line)) {
		std::vector<std::pair<std::string, double>> current_data = splitString(line);
		double value = current_data[0].second;

		if (k <= 30) {
			LINE_NUM_COEFF.push_back(value);
		}
		else if (k <= 50) {
			LINE_DEN_COEFF.push_back(value);
		}
		else if (k <= 70) {
			SAMP_NUM_COEFF.push_back(value);
		}
		else if (k <= 90) {
			SAMP_DEN_COEFF.push_back(value);
		}
	}

	double r = 0.0, c = 0.0;
	

	file.close();
	return pixel_coords;
}


int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "RU");

	if (argc != 5) {
		std::cerr << "Input format: <exe_file> <Latitude> <Longitude> <Height> <RPC_File>" << std::endl;
		return 1;
	}

	std::string PATH = argv[4];
	RPCData rpc_data = readRPC(PATH);

	GeodeticCoords geodetic_coords;
	geodetic_coords.latitude = std::stod(argv[1]);
	geodetic_coords.longitude = std::stod(argv[2]);
	geodetic_coords.height = std::stod(argv[3]);

	


	return 0;
}