#include <iostream>
#include <string>
#include "PNMImage.h"

using byte = unsigned char;

int main(int argc, char* argv[]) {

    if (argc < 9 || argc > 10) {
        std::cerr << "Incorrect number of arguments" << std::endl;
        exit(1);
    }

    char* inputFileName = strdup(argv[1]);
    char* outputFileName = strdup(argv[2]);
    byte color = std::stoi(argv[3]);
    double thickness = std::stof(argv[4]);
    double x0 = std::stof(argv[5]);
    double y0 = std::stof(argv[6]);
    double x1 = std::stof(argv[7]);
    double y1 = std::stof(argv[8]);
    bool gammaDefined = argc == 10;
    double gamma = gammaDefined ? std::stof(argv[9]) : 2.2;

    PNMImage picture(inputFileName);

    if (gammaDefined)
        picture.drawThickLine(x0, y0, x1, y1, color, thickness, gamma);
    else
        picture.drawThickLine(x0, y0, x1, y1, color, thickness, 0);

    picture.Export(outputFileName);

    return 0;
}