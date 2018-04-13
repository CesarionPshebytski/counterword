#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <fstream>
#include <mutex>
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

bool x(char i) { return (!isalnum(i)); }

char *filter_string(const char *s) {
    ///filter string from nonalpha symbols and return lowercase string
    std::string r = s;

//    std::remove_copy_if(s, s + strlen(s), std::back_inserter(r), x);
    r.erase(std::remove_if(r.begin(), r.end(), x), r.end());
    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
    auto *arr = static_cast<char *>(malloc(r.size()));
    strcpy(arr, r.c_str());
    //std::cout<<"^^^"<<arr<<" "<<strlen(arr)<<std::endl;
    return arr;
}

void update_map(std::unordered_map<char *, int> &m, char *el) {
    /// check if element is already in map
    //std::cout << "\n     el: " << el << "\n";
    if (strcmp(el, "") == 0) return;
    bool is_in_map = false;
    for (auto &it: m) {
        /// if is -- value is incremented
        if (strcmp(it.first, el) == 0) {
            it.second++;
            is_in_map = true;
            break;
        }
    }
    /// if not -- added with value 1
    if (!is_in_map) {
        //std::cout << "     el to map: " << el << "\n\n";
        auto iter = m.begin();
        m.insert(iter, std::pair<char *, int>(el, 1));
//        m[el] = 1;
        for (auto &it: m) {
            //std::cout << "     " << it.first << " : " << it.second << "\n";
        }
    }
}

std::vector<std::pair<int, int>> find_indexes(std::vector<std::string> words, int threads) {
    std::vector<std::pair<int, int>> vector;
    int step = (int) words.size() / threads;
    int start = 0;
    int end;
    for (size_t i = 0; i <= words.size(); i += step) {
        if ( ( words.size() - start+step < step  )) {
            end = words.size();
        }else {
            end = start+step;
        }
        vector.push_back(std::make_pair(start, end));
        start = end+1;
    }
    return vector;
}

std::vector<std::pair<int, int>> get_indexes(char* buffer, size_t result, int thread_count) {
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

void multiple_update(std::unordered_map<char *, int> &m, std::vector<std::string> words, int start, int end) {
    for (std::vector<int>::size_type i = start; i != end; i++) {
        update_map(m, filter_string(words[i].c_str()));
    }
}

int file_to_map(const char *path, std::unordered_map<char *, int> &m, const char *delimiter, int thread_number) {
    /// variables declaration
    FILE *pFile;
    long lSize;
    size_t result;
    char *buffer;

    /// open file from path for reading;
    pFile = fopen(path, "r");
    if (pFile == nullptr) {
        fputs("File error", stderr);
        return 0;
    }

    /// obtain file size:
    fseek(pFile, 0, SEEK_END);
    lSize = ftell(pFile);
    rewind(pFile);

    /// allocate memory to contain the whole file:
    buffer = (char *) malloc(sizeof(char) * lSize);
    if (buffer == nullptr) {
        fputs("Memory error", stderr);
        return 0;
    }

    /// copy the file into the buffer:
    auto loading_time = get_current_time_fenced();
    result = fread(buffer, 1, static_cast<size_t>(lSize), pFile);
    std::cout << "Reading time: " << to_us(get_current_time_fenced() - loading_time) << std::endl;
    if (result != lSize) {
        fputs("Reading error", stderr);
        return 0;
    }

    //std::cout<<"\n";
    //std::cout<<"@@@@@@@@@\n"<<buffer<<"\n@@@@@@@@@"<<std::endl;
    //std::cout<<"\n";

    /// split buffer into lines separated by whitespace:
    auto analyzing_time = get_current_time_fenced();
    std::string test = buffer;
    std::string word;
    std::vector<std::string> words;
    for (std::stringstream s(test); s >> word;) {
        words.push_back(word);
    }
    std::vector<std::thread> threads;

    std::vector<std::pair<int,int>> indexes = find_indexes(words,thread_number);

    for(std::pair<int,int> index_pair : indexes) {
        threads.emplace_back(std::thread(multiple_update, std::ref(m), words,index_pair.first, index_pair.second));
    }

    for (auto &thread : threads) thread.join();
    // old one, without threads
//    for(const std::string &single_word: words){
//        update_map(m, filter_string(const_cast<char *>(single_word.c_str())));
//    }
//    buffer = strtok(buffer, delimiter);
//    while (buffer!=NULL) {
//        //std::cout<<"###"<<qeqz<<" "<<strlen(qeqz)<<std::endl;
//        update_map(m, filter_string(buffer));
//        buffer = strtok(NULL, delimiter);
//    }
    std::cout << "Analyzing time: " << to_us(get_current_time_fenced() - analyzing_time) << std::endl;

    /// free buffer and return true if everything ok;
    free(buffer);
    fclose(pFile);
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

bool string_comparator(const std::pair<char *, int> &a, const std::pair<char *, int> &b) {
    ///comparator for string sorting
    return strcmp(a.first, b.first) < 0;
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
    std::unordered_map<char *, int> m;
    FILE *pFile;

    auto stage1_start_time = get_current_time_fenced();
    file_to_map(path, m, "\n \t", thread_number);

    std::vector<std::pair<char *, int>> sorted_elements(m.begin(), m.end());
    std::sort(sorted_elements.begin(), sorted_elements.end(), string_comparator);
    pFile = fopen(out_by_a, "w");
    for (auto &it : sorted_elements) {
        //std::cout<<"___"<<it.first<<" : "<<it.second<<"  "<<strlen(it.first)<<std::endl;
        auto *first = static_cast<char *>(malloc(strlen(it.first) + sizeof(it.second)));
        strcat(strcpy(first, it.first), (" : " + std::to_string(it.second) + "\n").c_str());
        fwrite(first, 1, strlen(first), pFile);
    }
    fclose(pFile);

    //std::cout<<"\n";

    std::multimap<int, char *> m1 = swapPairs(m);
    m.clear();
    pFile = fopen(out_by_n, "w");
    for (auto &it : m1) {
        auto *first = static_cast<char *>(malloc(strlen(it.second) + sizeof(it.first)));
        std::string second = it.second;
        strcat(strcpy(first, std::to_string(it.first).c_str()), (" : " + second + "\n").c_str());
        fwrite(first, 1, strlen(first), pFile);
    }
    m1.clear();
    fclose(pFile);

    std::cout << "Total time : " << to_us(get_current_time_fenced() - stage1_start_time);
    return 0;
}


//1 : depressedlookingporto
//1 : porto
//1 : rico
//1 : from
//1 : depressedlookingindividual
//2 : individual

//1 : depressedlookingporto
//1 : individual
//1 : from
//1 : porto
//1 : rico
//1 : depressedlookingeindividual

