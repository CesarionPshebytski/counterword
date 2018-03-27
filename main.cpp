#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <unordered_map>
#include <regex>

int words = 0;

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

char *filter_string(char *s) {
    ///filter string from nonalpha symbols and put string to lowercase
    std::string r;
    std::remove_copy_if(s, s + strlen(s), std::back_inserter(r),
                        std::not1(std::ptr_fun<int, int>(&std::isalpha)));
    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
    auto *arr = static_cast<char *>(malloc(sizeof(char) * r.size()));
    strcpy(arr, r.c_str());
    return arr;
}




void update_map(std::unordered_map<char *, int> &m, char *el) {
    /// check if element is already in map
    if(strcmp(el, "") == 0) return;
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
        m[el] = 1;
    }
}

std::vector<std::pair<int, int>>  get_indexes(char* buffer, size_t result, int thread_count) {
    std::vector<std::pair<int,int>> vector;
    int step = (int) result / thread_count;
    int start = 0,end;
    for(size_t i = 0; i <= result; i+=step) {

        if ( isspace(buffer[start])) {
            while( start <= result) {

                if(isspace(buffer[start])) {
                    start++;
                }else {

                    break;
                }
            }
        }
        end = start+step;
        if ( isspace(buffer[end])) {
            while( end <= result) {

                if(isspace(buffer[end])) {
                    end++;
                }else {
                    break;
                }
            }
        }
        std::cout<< "start " << start << " end " << end << std::endl;
        vector.push_back(std::make_pair(start,end));
        start = end;

    }
    return vector;
};

int file_to_map(const char *path, std::unordered_map<char *, int> &m, const char *delimiter) {
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
    result = fread(buffer, 1, lSize, pFile);
    if (result != lSize) {
        fputs("Reading error", stderr);
        return 0;
    }
    std::vector<std::pair<int,int>> vector = get_indexes(buffer, result, 3);
    std::cout<< vector.at(0).first << std::endl;
    /// split buffer into lines separated by whitespace:
    buffer = strtok(buffer, delimiter);
    while (buffer) {
        words++;
        update_map(m, filter_string(buffer));
        buffer = strtok(nullptr, delimiter);
    }

    /// free buffer and return true if everything ok;
    free(buffer);
    fclose(pFile);
    return 1;
}

template<class T1, class T2>
std::multimap<T2, T1> swapPairs(std::unordered_map<T1, T2> m) {
    std::multimap<T2, T1> m1;

    for (auto &&item : m) {
        m1.emplace(item.second, item.first);
    }

    return m1;
};

bool string_comparator(const std::pair<char *, int> &a, const std::pair<char *, int> &b) {
    return strcmp(a.first, b.first) < 0;
}

int main() {
    const char *path = "/Users/cesarion_pshebytski/Downloads/DataSource/theBIG.txt";
    path = "example.txt";
    std::unordered_map<char *, int> m;
    auto stage1_start_time = get_current_time_fenced();
    file_to_map(path, m, " \n");

    std::vector<std::pair<char *, int>> sorted_elements(m.begin(), m.end());
    std::sort(sorted_elements.begin(), sorted_elements.end(), string_comparator);
    for (auto &it : sorted_elements) {
        std::cout << it.first << " : " << it.second << std::endl;
    }

    std::cout << "\n";
    const std::multimap<int, char *> &m1 = swapPairs(m);
    for (auto i : m1) {
        std::cout << i.first << " : " << i.second << std::endl;
    }

    std::cout << "\n\nTime : " << to_us(get_current_time_fenced() - stage1_start_time) << std::endl;
    std::cout << "\n" << "words: " << words;
    std::cout << "\n" << "different words: " << m.size();
    return 0;
}