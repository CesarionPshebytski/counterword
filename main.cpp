#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <map>

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

void print_if_present(std::map<char *, int> &m, char *el, int &i) {
    // Звертання m[...] заборонене!
    // Воно може модифікувати відображення.
//    auto itr = m.find(el);
    bool add = true;
    for (auto &it : m) {
        if(!strcmp(it.first, el)){
            add = false;
            break;
        }
    }
    if (!add) {
        auto itr = m.find(el);
        m[el]+=2;
        for (auto &it : m) {
            std::cout << it.first << " : " << it.second << std::endl;
        }
        std::cout<< "\n";
        std::cout << "Element " << el << " is present" << std::endl << "\n";
    } else {
        m[el]=0;
        for (auto &it : m) {
            std::cout << it.first << " : " << it.second << std::endl;
        }
        std::cout<< "\n";
//        auto it = m.begin();
//        m.insert(it, std::pair<char *, int>(el, ++i));
//        std::cout << "Element " << el << " is not present" << std::endl;
    }
}

int lines_to_dictionary(const std::vector<char *> lines, std::map<char *, int> &m) {
    int i = 0;
    for (char *line: lines) {
        line = strtok(line, "\n");
        while (line) {
            print_if_present(m, line, i);
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
    std::cout << "\n" << lines.size() << "\n";
    for (auto &it : m) {
        std::cout << it.first << " : " << it.second << std::endl;
    }


    return 0;
}