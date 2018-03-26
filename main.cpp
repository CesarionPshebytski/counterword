#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <iostream>
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

int file_to_lines(const char *path, std::vector<char *> &lines) {
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
    buffer = strtok(buffer, " ");
    while (buffer) {
        lines.emplace_back(buffer);
        buffer = strtok(nullptr, " ");
    }

    /// free buffer and return true if everything ok;
    free(buffer);
    fclose(pFile);
    return 1;
}

void mapping(std::map<char *, int> &m, char *el) {
    bool is_in_map = false;
    for (auto &it: m) {
        if (!strcmp(it.first, el)) {
            it.second = it.second + 1;
            is_in_map = true;
            break;
        }
    }
    if (!is_in_map) {
        m[el] = 1;
    }
}

int lines_to_dictionary(const std::vector<char *> lines, std::map<char *, int> &m) {
    for (char *line: lines) {
        line = strtok(line, "\n");
        while (line) {
            words++;
            mapping(m, line);
            line = strtok(nullptr, " \n");
        }
    }
    return 1;
}

int main() {
    std::vector<char *> lines;
    const char *path = "/Users/cesarion_pshebytski/Downloads/DataSource/Loyko_Aeroport_RuLit_Me_421937.txt";
    path = "example.txt";
    std::map<char *, int> m;
    file_to_lines(path, lines);
    lines_to_dictionary(lines, m);
    std::cout << "words: " <<words;
    std::cout << "\n" << "different words: " << m.size() << "\n\n";
    for (auto &it : m) {
        std::cout << it.first << " : " << it.second << std::endl;
    }


    return 0;
}