#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <algorithm>
#include <cstring>

using namespace std;

const unsigned char START_BYTE = 0;
const unsigned char STOP_BYTE = 255;

void logError(string msg) {
    std::ofstream logging("task3.log");
    logging << msg << "\n";
    logging.close();
    throw;
}

long get_epoch_time(string T) {
    if (T.size() != 19 || T[4] != ':' && T[7] != ':' && T[10] != ' ' && T[13] != ':' && T[16] != ':')
        throw std::invalid_argument("incorrect time string format");
    struct tm t = {0};

    t.tm_year = stoi(T.substr(0, 4)) - 1900;
    t.tm_mon  = stoi(T.substr(5, 2)) - 1;
    t.tm_mday = stoi(T.substr(8, 2));
    t.tm_hour = stoi(T.substr(11, 2)) + 1;
    t.tm_min  = stoi(T.substr(14, 2));
    t.tm_sec  = stoi(T.substr(17, 2));

    time_t timeSince1900 = mktime(&t);
    return long(timeSince1900);
}

int main(int argc, char **argv) {
    const string INPUT_FILE = argv[1];
    const string OUTPUT_FILE = argv[2];
    // done parse args
    
    ifstream csv(INPUT_FILE);
    ofstream hex(OUTPUT_FILE);
    // done parse args

    if (!csv.good()) logError("Error 01: file not found or cannot be accessed");
    if (!hex.good()) logError("Error 04: cannot override " + OUTPUT_FILE);

    // Read each line of the CSV file
    int line_idx = 1;
    string line;
    getline(csv, line);

    if (line != "id,time,values,aqi,pollution")
        logError("Error 02: invalid csv file");


    while (getline(csv, line)) {
        int idx, aqi;
        long time;
        float mean;

        try {
            stringstream ss(line);
            string component;
            getline(ss, component, ','); idx = stoi(component);
            getline(ss, component, ','); time = get_epoch_time(component);
            getline(ss, component, ','); mean = stof(component);
            getline(ss, component, ','); aqi = stoi(component);
        } catch (...) {
            logError("Error 03: data is missing at line " + to_string(line_idx));
        }
        line_idx++;

        unsigned char hex_data[15];

        memcpy(hex_data + 2, &aqi, 2);
        memcpy(hex_data + 4, &mean, 4);
        memcpy(hex_data + 8, &time, 4);
        memcpy(hex_data + 12, &idx, 1);

        reverse(hex_data, hex_data + 15);

        hex_data[1] = 15;   // package length
        hex_data[0] = START_BYTE;
        hex_data[14] = STOP_BYTE;
        hex_data[13] = 255 - std::accumulate(hex_data + 1, hex_data + 13, 0) - hex_data[14]; // check sum

        for (size_t i = 0 ; i < 15 ; ++i) {
            hex << std::hex;        // print the hex representation;
            hex << std::uppercase;  // the symbol a, b, c, d, e, f should be upper case
            hex << std::setw(2) << std::setfill('0');   // hex string format
            hex << static_cast<int>(hex_data[i]) << " ";
        }
        hex << "\n";
    }
    // Close the input and output files
    csv.close();
    hex.close();

    return 0;
}
