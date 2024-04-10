#include <fstream>
#include <iomanip>
#include <cstring>
#include <random>

std::default_random_engine rng;
std::uniform_int_distribution<> distribution(0, 6000);

const std::string filename = "dust_sensor.csv";

int num_sensors = 1;
int sampling = 30;
int interval = 24;

void logError(std::string msg){
    std::ofstream logging("task1.log");
    logging << msg << "\n";
    logging.close();
    throw;
}
void args_parse(int argc, char *argv[]) {
    int parsedArgs = 0;

    std::vector<std::string> arg_flags = {"-n", "-st", "-si"};
    std::vector<int> arg_values = {1, 30, 24};

    for (int i = 1 ; i < argc ; i += 2)
    for (int j = 0 ; j < 3 ; ++j) if (strcmp(argv[i], arg_flags[j].c_str()) == 0) {
        arg_values[j] = std::stoi(argv[i + 1]);
        parsedArgs++;
        break;
    }
    num_sensors = arg_values[0];
    sampling = arg_values[1];
    interval = arg_values[2];

    if (2 * parsedArgs + 1 < argc)
        logError("Error 01: invalid command");
    if (num_sensors < 0 || sampling < 1 || interval < 1)
        logError("Error 02: invalid argument");
}

std::string get_time(time_t t, bool local = true) {
    auto tm = *std::localtime(&t);

    char string[80];
    strftime(string, 80, "%Y:%m:%d %H:%M:%S", local ? &tm : gmtime(&t));
    return string;
}

int main(int argc, char **argv) {
    args_parse(argc, argv);

    // create file output stream to write to file
    std::ofstream file(filename.c_str());

    if (!file.good())
        logError("Error 03: denied access dust_sensor.csv");

    file << std::fixed << std::setprecision(1);
    file << "id,time,values\n";

    long start = time(nullptr);
    long prev = start - sampling;

    time_t timeSince1900 = std::time(nullptr);

    for (time_t runtime = 0 ; runtime < interval * 3600 ; runtime += sampling ) {
        std::string Time = get_time(timeSince1900 + runtime);

        for (int id = 1 ; id <= num_sensors ; ++id) {
            file << id << ",";
            file << Time << ",";
            file << 0.1 * distribution(rng) << "\n";
        }
    }
    file.close();

    return 0;
}
