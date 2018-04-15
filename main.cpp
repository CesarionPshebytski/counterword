#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <fstream>
#include <map>
#include <vector>
#include <unordered_map>
#include <regex>
#include <string>
#include <atomic>
#include <sstream>

inline std::chrono::high_resolution_clock::time_point get_current_time_fenced() {
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto res_time = std::chrono::high_resolution_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    return res_time;
}

template<class D>
inline long long to_us(const D &d) {
    return std::chrono::duration_cast<std::chrono::microseconds>(d).count();
}

std::string filter_string(std::string s) {
    std::string r{s};

    r.erase(std::remove_if(r.begin(), r.end(), [](auto c) { return !isalnum(c); }), r.end());
    std::transform(r.begin(), r.end(), r.begin(), ::tolower);

    return r;
}


std::vector<std::pair<int, int>> find_indexes(std::vector<std::string> words, int threads) {
    std::vector<std::pair<int, int>> vector;
    int step = (int) words.size() / threads;
    int start = 0;
    int end;

    for(int i = 0; i< threads; i++) {
        if(words.size() - start < step) {
            end = words.size();
        }else {
            end = start+step;
        }
        vector.push_back(std::make_pair(start, end));
        start = end;
    }

    return vector;
}

std::vector<std::pair<int, int>> get_indexes(char *buffer, size_t result, int thread_count) {
    std::vector<std::pair<int, int>> vector;
    int step = (int) result / thread_count;
    int start = 0, end;
    for (size_t i = 0; i <= result; i += step) {

        if (isspace(buffer[start])) {
            while (start <= result) {

                if (isspace(buffer[start])) {
                    start++;
                } else {

                    break;
                }
            }
        }
        end = start + step;
        if (isspace(buffer[end])) {
            while (end <= result) {

                if (isspace(buffer[end])) {
                    end++;
                } else {
                    break;
                }
            }
        }
        std::cout << "start " << start << " end " << end << std::endl;
        vector.push_back(std::make_pair(start, end));
        start = end;

    }
    return vector;
};

void multiple_update(std::unordered_map<std::string, int> &m, const std::vector<std::string> &words, size_t start,
                     size_t end) {
    std::string word;
    for (auto i = start; i != end; ++i) {
        word = filter_string(words[i]);
        if(word == "") {
            continue;
        }
            ++m[word];

    }
}

int file_to_map(const char *path, std::unordered_map<std::string, int> &m, const char *delimiter, int thread_number) {


    /// split buffer into lines separated by whitespace:
    auto loading_time = get_current_time_fenced();
    std::string word;
    std::vector<std::string> words;
    std::ifstream file;
    file.open(path);
    while(file >> word) {
        words.push_back(word);
    }
    std::cout << "Reading time: " << to_us(get_current_time_fenced() - loading_time) << std::endl;

    // analyze time
    std::vector<std::pair<int, int>> indexes = find_indexes(words, thread_number);
    auto analyzing_time = get_current_time_fenced();
//    std::string test = buffer;

    std::vector<std::thread> threads;


    for (std::pair<int, int> index_pair : indexes) {
        threads.emplace_back(std::thread(multiple_update, std::ref(m), words, index_pair.first, index_pair.second));
    }

    for (auto &thread : threads) thread.join();
//     old one, without threads
//    for(const std::string &single_word: words){
//        ++m[filter_string(single_word)];
////        update_map(m, filter_string(const_cast<char *>(single_word.c_str())));
//    }

    std::cout << "Analyzing time: " << to_us(get_current_time_fenced() - analyzing_time) << std::endl;



    return 1;
}

template<class T1, class T2>
std::multimap<T2, T1> swapPairs(std::unordered_map<T1, T2> m) {
    ///swap key and value
    std::multimap<T2, T1> m1;
    for (auto &&item : m) {
        m1.emplace(item.second, item.first);
    }
    return m1;
};

bool string_comparator(const std::pair<std::string, int> &a, const std::pair<std::string, int> &b) {
    ///comparator for string sorting
    return a.first.compare(b.first) < 0;
}

struct configuration_t {
    ///configuration structure
    int thread_number;
    std::string infile, out_by_a, out_by_n;
};

configuration_t read_configuration(const std::string &filename) {
    ///opening configuration file
    std::ifstream config_stream(filename);
    if (!config_stream.is_open()) {
        std::cerr << "Failed to open configuration file " << filename << std::endl;
    }
    try {
        ///reading configuration file
        std::ios::fmtflags flags(config_stream.flags());
        config_stream.exceptions(std::ifstream::failbit);
        configuration_t res;
        std::string temp;
        std::string temporary_temp;
        try {
            ///setting values of structure from file
            getline(config_stream, temp, '=');
            config_stream >> res.infile;
            getline(config_stream, temp, '=');
            config_stream >> res.out_by_a;
            getline(config_stream, temp, '=');
            config_stream >> res.out_by_n;
            getline(config_stream, temp, '=');
            config_stream >> res.thread_number;
            getline(config_stream, temp, '=');
        } catch (std::ios_base::failure &fail) {
            config_stream.flags(flags);
            throw;
        }
        config_stream.flags(flags);
        if (res.thread_number < 0) {
            throw std::runtime_error("thread number should be > 0");
        }
        return res;
    } catch (std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
    return configuration_t();
}

int main() {
    std::string filename("config.txt");
    configuration_t config = read_configuration(filename);
    const char *path = std::regex_replace(config.infile, std::regex("\""), "").c_str();
    const char *out_by_a = std::regex_replace(config.out_by_a, std::regex("\""), "").c_str();
    const char *out_by_n = std::regex_replace(config.out_by_n, std::regex("\""), "").c_str();
    int thread_number = config.thread_number;
    std::unordered_map<std::string, int> m;
    FILE *pFile;

    auto stage1_start_time = get_current_time_fenced();
    file_to_map(path, m, "\n \t", thread_number);

    std::vector<std::pair<std::string, int>> sorted_elements(m.begin(), m.end());
    std::sort(sorted_elements.begin(), sorted_elements.end(), string_comparator);
    pFile = fopen(out_by_a, "w");
    for (auto &it : sorted_elements) {
        //std::cout<<"___"<<it.first<<" : "<<it.second<<"  "<<strlen(it.first)<<std::endl;
        std::string first;
        first = it.first + " : " + std::to_string(it.second) + "\n";

        fwrite(first.c_str(), 1, first.length(), pFile);
    }
    fclose(pFile);

    //std::cout<<"\n";

    std::multimap<int, std::string> m1 = swapPairs(m);
    m.clear();
    pFile = fopen(out_by_n, "w");
    for (auto &it : m1) {
        std::string first;
        std::string second = it.second;
        first = std::to_string(it.first) + " : " + second + "\n";

        fwrite(first.c_str(), 1, first.length(), pFile);
    }
    m1.clear();
    fclose(pFile);

    std::cout << "Total time : " << to_us(get_current_time_fenced() - stage1_start_time);
    return 0;
}


