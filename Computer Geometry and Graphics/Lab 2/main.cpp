#include <iostream>
#include <string>
#include "PNMImage.h"

using byte = unsigned char;

int main(int argc, char* argv[]) {
    if (argc < 9 || argc > 10) {
        std::cerr << "Incorrect number of arguments" << std::endl;
        return 1;
    }

    char *inputFileName, *outputFileName;
    byte color;
    bool gammaDefined;
    double thickness, x0, y0, x1, y1, gamma;

    try {
        inputFileName = strdup(argv[1]);
        outputFileName = strdup(argv[2]);
        color = std::stoi(argv[3]);
        thickness = std::stod(argv[4]);
        x0 = std::stod(argv[5]);
        y0 = std::stod(argv[6]);
        x1 = std::stod(argv[7]);
        y1 = std::stod(argv[8]);
        gammaDefined = argc == 10;
        gamma = gammaDefined ? std::stof(argv[9]) : 2.2;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    PNMImage *picture = nullptr;

    try {
        picture = new PNMImage(inputFileName);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    try {
        if (gammaDefined)
            picture->drawThickLine(x0, y0, x1, y1, color, thickness, gamma);
        else
            picture->drawThickLine(x0, y0, x1, y1, color, thickness, 0);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    try {
        picture->Export(outputFileName);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}