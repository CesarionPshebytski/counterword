#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

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

char *strtolower(char *s) {
    ///put string to lowercase
    for (int i = 0; i < strlen(s); ++i)
        s[i] = static_cast<char>(tolower(s[i]));
    return s;
}

void update_map(std::map<char *, int> &m, char *el) {
    /// check if element is already in map
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

int file_to_lines(const char *path, std::map<char *, int> &m, const char *delimiter) {
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

    /// split buffer into lines separated by whitespace:
    buffer = strtok(buffer, delimiter);
    while (buffer) {
        words++;
        update_map(m, strtolower(buffer));
        buffer = strtok(nullptr, delimiter);
    }

    /// free buffer and return true if everything ok;
    free(buffer);
    fclose(pFile);
    return 1;
}

int main() {
    const char *path = "/Users/cesarion_pshebytski/Downloads/DataSource/theBIG.txt";
//    path = "example.txt";
    std::vector<char *> lines;
    std::map<char *, int> m;
    auto stage1_start_time = get_current_time_fenced();
    file_to_lines(path, m, " .,:;!?()*«»<>-+/[]\"\'\n");

    for (auto &it : m) {
        std::cout << it.first << " : " << it.second << std::endl;
    }
    std::cout << "\n\nTime : " << to_us(get_current_time_fenced() - stage1_start_time) << std::endl;
    std::cout << "\n" << "words: " << words;
    std::cout << "\n" << "different words: " << m.size();

    return 0;
}