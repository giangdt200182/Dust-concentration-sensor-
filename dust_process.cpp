#include <fstream>
#include <iomanip>
#include <string>
#include <random>

#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <cassert>

using namespace std;

const string task1 = "dust_outlier.csv";
const string task2 = "dust_aqi.csv";
const string task3 = "dust_summary.csv";
const string task4 = "dust_statistics.csv";
const int HOUR = 3600;

const vector<float> MOCKS_C = {0, 12, 35.5, 55.5, 150.5, 250.5, 350.5, 550.5};
const vector<int> MOCKS_AQI = {0, 50, 100, 150, 200, 300, 400, 500};
const vector<string> MOCKS_POLLUTION = {"Good", "Moderate", "Slightly unhealthy", "Unhealthy", "Very unhealthy", "Hazardous", "Extremely hazardous"};

void logError(string msg) {
    std::ofstream logging("task2.log");
    logging << msg << "\n";
    logging.close();
    throw;
}

long get_epoch_time(string T) {
    struct tm t = {0};
    size_t pos = 0;

    t.tm_year = stoi(T.substr(0, 4)) - 1900;
    t.tm_mon  = stoi(T.substr(5, 2)) - 1;
    t.tm_mday = stoi(T.substr(8, 2));
    t.tm_hour = stoi(T.substr(11, 2)) + 1;
    t.tm_min  = stoi(T.substr(14, 2));
    t.tm_sec  = stoi(T.substr(17, 2));

    // Get time since 1900 epoch
    // t.tm_year -= 1900;

    time_t timeSince1900 = mktime(&t);
    return long(timeSince1900);
}
string get_std_time(long epochtime) {
    time_t t = epochtime + 6 * 3600;
    char string[80];
    strftime(string, 80, "%Y:%m:%d %H:%M:%S", gmtime(&t));
    return string;
}

class DataPoint {
public:
    DataPoint(int id, string time, float val){
        id_ = id;
        time_ = time;
        val_ = val;
    }
    ~DataPoint() = default;

    friend ostream & operator << (ostream &os, const DataPoint &dt) {
        os << dt.id_ << ",";
        os << dt.time_ << ",";
        os << dt.val_;
        return os;
    }
    int id() const { return id_; }
    float val() const { return val_; }
    long time() const { return get_epoch_time(time_); }
private:
    int id_;
    string time_;
    float val_;
};

class DataFrame {
public:
    DataFrame(string dt_filename) {
        ifstream csv(dt_filename.c_str()); //Open file by ifstream
        string line; // save
        int line_idx = 1;

        if (!csv.good())
            logError("Error 01: file not found or cannot be accessed");

        getline(csv, line); // first line is about headers

        if (line != "id,time,values") {
            logError("Error 02: invalid csv file");
            throw;
        }

        while (getline(csv, line)) {
            try {
                size_t comma1 = line.find(",");
                size_t comma2 = line.find(",", comma1 + 1);

                string indexStr(line.begin(), line.begin() + comma1);
                string valueStr(line.begin() + comma2 + 1, line.end());
                string timeStr(line.begin() + comma1 + 1, line.begin() + comma2);

                indice.push_back(stoi(indexStr));   assert(indice.back() > 0);
                values.push_back(stof(valueStr));
                times.push_back(timeStr);
            } catch(...) {
                logError("Error 03: data is missing at line " + to_string(line_idx));
                throw;
            }
            line_idx++;
        }
    }
    size_t num_sensors() const {
        vector<int> aux = indice;
        sort(aux.begin(), aux.end());
        return unique(aux.begin(), aux.end()) - aux.begin();
    }
    size_t size() {
        return indice.size();
    }
    DataPoint operator [](int i) {
        return DataPoint(indice[i], times[i], values[i]);
    }
private:
    vector<float> values;
    vector<int> indice;
    vector<string> times;
};

class DataSummary {
public:
    void add(const DataPoint& dt) {
        if (index < 0) {
            index = dt.id();
            arg_time_min = start_time = dt.time();
            arg_time_max = end_time = dt.time();
            min_ = max_ = sum_ = dt.val();
            n = 1;
            return;
        }
        sum_ += dt.val();   n++;
        end_time = dt.time();

        if (min_ > dt.val()) { min_ = dt.val(); arg_time_min = dt.time(); }
        if (max_ < dt.val()) { max_ = dt.val(); arg_time_max = dt.time(); }
    }
    friend ostream & operator << (ostream &os, const DataSummary &dt) {
        long interval = dt.end_time - dt.start_time;

        os << dt.index << ",max," << get_std_time(dt.arg_time_max) << "," << dt.max_ << "\n";
        os << dt.index << ",min," << get_std_time(dt.arg_time_min) << "," << dt.min_ << "\n";
        os << dt.index << ",mean," << get_std_time(interval).substr(11, 8) <<  "," << dt.sum_ / dt.n;

        return os; 
    }
protected:
    long arg_time_min;
    long arg_time_max;
    long start_time;
    long end_time;
    
    float min_, max_;
    float sum_;
    int n, index = -1;
};

class HourAverageMeter {
public:
    bool add(const DataPoint &dt) {
        if (index < 0) {
            index = dt.id();
            start_time = dt.time();
            sum = n = 0;
        }
        if (dt.time() / HOUR != start_time / HOUR)
            return false;
        
        sum += dt.val();
        n++;

        return true;
    }
    int aqi() const {
        float x = sum / n;
        for (int i = 0 ; i < MOCKS_C.size() - 1 ; ++i)
            if (i == MOCKS_C.size() - 1 || x < MOCKS_C[i + 1]) {
                x -= MOCKS_C[i];
                x /= (MOCKS_C[i + 1] - MOCKS_C[i]);
                x *= (MOCKS_AQI[i + 1] - MOCKS_AQI[i]);
                x += MOCKS_AQI[i] + 0.5;
                return x;
            }
        throw std::invalid_argument("passing outlier data");
    }
    string pollution() const {
        float x = sum / n;
        for (int i = 0 ; i < MOCKS_C.size() - 1 ; ++i)
            if (i == MOCKS_C.size() - 1 || x < MOCKS_C[i + 1])
                return MOCKS_POLLUTION[i];
        throw std::invalid_argument("passing outlier data");
    }
    friend ostream & operator << (ostream &os, const HourAverageMeter &dt) {
        long time = (dt.start_time / 3600 + 1) * 3600;
        float mean = dt.sum / dt.n;

        os << DataPoint(dt.index, get_std_time(time), mean) << ",";
        os << dt.aqi() << ",";
        os << dt.pollution();

        return os;
    }
private:
    long start_time;
    float sum;
    int n, index = -1;
};

bool is_outlier(float x) {
    if (x < 5)      return true;
    if (x > 550.5)  return true;

    return false;
}

int main(int argc, char **argv) {
    string dt_filename = "dust_sensor.csv";
    if (argc > 1)
        dt_filename = argv[1];

    DataFrame data(dt_filename);

    { // task 2.1
        vector<int> outlier_indice;
        ofstream file(task1.c_str());

        for (int i = 0 ; i < data.size() ; ++i)
            if (is_outlier(data[i].val()))
                outlier_indice.push_back(i);

        file << "Number of outliers: " << outlier_indice.size() << "\n";
        file << "id,time,values\n";

        for (int i : outlier_indice)
            file << data[i] << "\n";
        
        file.close();
    } // done task 2.1
    { // task 2.2 + 2.4
        int num_sensors = data.num_sensors();
        
        vector<HourAverageMeter> meters[num_sensors];

        ofstream file2(task2.c_str());
        ofstream file4(task4.c_str());

        file2 << "id,time,values,aqi,pollution\n";
        file2 << fixed << setprecision(1);

        for (int i = 0 ; i < num_sensors ; ++ i)
            meters[i].push_back(HourAverageMeter());
        
        for (int i = 0 ; i < data.size() ; ++i) if (!is_outlier(data[i].val())) {
            int id = data[i].id() - 1;

            bool success = meters[id].back().add(data[i]);
            if (!success) {
                file2 << meters[id].back() << "\n";
                meters[id].push_back(HourAverageMeter());
                meters[id].back().add(data[i]);
            }
        }
        for (int i = 0 ; i < num_sensors ; ++i)
            file2 << meters[i].back() << "\n";
        
        for (int i = 0 ; i < num_sensors ; ++i) {
            map <string, int> cnt_state;

            for (auto meter : meters[i])
                cnt_state[meter.pollution()]++;
            
            for (string pollution_level : MOCKS_POLLUTION) {
                file4 << i + 1 << ",";
                file4 << pollution_level << ",";
                file4 << cnt_state[pollution_level] << "\n";
            }
        }
        file4.close();
    } // done task 2.2 + 2.4
    { // task 2.3
        ofstream file(task3.c_str());
        int num_sensors = data.num_sensors();
        
        DataSummary brief[num_sensors];

        file << "id,parameters,time,values \n";
        file << fixed << setprecision(1);

        for (int i = 0 ; i < data.size() ; ++i) if (!is_outlier(data[i].val())) {
            int id = data[i].id();
            brief[id - 1].add(data[i]);
        }
        for (int i = 0 ; i < num_sensors ; ++i)
            file << brief[i] << "\n";
        
        file.close();
    } // done task 2.3
    return 0;
}