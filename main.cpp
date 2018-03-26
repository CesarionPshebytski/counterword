#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>

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

bool fileExists(const char *path, char *&buffer, std::vector<char *> &lines) {
    FILE *pFile;
    long lSize;
    size_t result;

    pFile = fopen(path, "r");
    if (pFile == nullptr) {
        fputs("File error", stderr);
        return false;
    }

    // obtain file size:
    fseek(pFile, 0, SEEK_END);
    lSize = ftell(pFile);
    rewind(pFile);

    // allocate memory to contain the whole file:
    buffer = (char *) malloc(sizeof(char) * lSize);
    if (buffer == nullptr) {
        fputs("Memory error", stderr);
        return false;
    }

    // copy the file into the buffer:
    result = fread(buffer, 1, lSize, pFile);
    if (result != lSize) {
        fputs("Reading error", stderr);
        return false;
    }

    /* the whole file is now loaded in the memory buffer. */

    // terminate
//    std::cout << buffer;
    buffer = strtok(buffer, " ");
    while(buffer){
        lines.emplace_back(buffer);
        buffer = strtok(nullptr, " ");
    }
    return fclose(pFile) != 0;
}

void xxx(const char *path, std::vector<std::string> &lines){
    std::string sLine;
    std::ifstream infile;
    infile.open(path);

    while (!infile.eof())
    {
        getline(infile, sLine);
        lines.push_back(sLine);
    }
    infile.close();
}

int main() {
    char *buffer;
//    std::vector<std::string> lines;
    std::vector<char *> arr;
    const char *path = "/Users/cesarion_pshebytski/Downloads/DataSource/Loyko_Aeroport_RuLit_Me_421937.txt";
//    auto stage1_start_time = get_current_time_fenced();
//    xxx(path, lines);
//    std::cout << "\nTime : " << to_us(get_current_time_fenced() - stage1_start_time) << std::endl;
//    std::cout<<"\n"<<lines.size();
//    ////
//    lines.clear();
    fileExists(path, buffer, arr);
    std::cout<<"\n"<<arr.size();

    free(buffer);
    return 0;
}