#include <iostream>
#include <string>
#include "PNMImage.h"

using byte = unsigned char;

int main(int argc, char* argv[]) {
    if (argc != 7) {
        std::cerr << "Incorrect number of arguments" << std::endl;
        return 1;
    }

    char *inputFileName, *outputFileName;
    byte bit;
    int ditheringType;
    bool gradient;
    double gamma;

    try {
        inputFileName = strdup(argv[1]);
        outputFileName = strdup(argv[2]);
        gradient = std::stoi(argv[3]) == 1;
        ditheringType = std::stoi(argv[4]);
        bit = std::stoi(argv[5]);
        gamma = std::stof(argv[6]);
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
        if (gradient) picture->fillGradient(gamma);
        switch (ditheringType) {
            case 0: {
                picture->ditherNone(bit, gamma);
                break;
            }
            case 1: {
                picture->ditherOrdered(bit, gamma);
                break;
            }
            case 2: {
                picture->ditherRandom(bit, gamma);
                break;
            }
            case 3: {
                picture->ditherFloydSteinberg(bit, gamma);
                break;
            }
            case 4: {
                picture->ditherJJN(bit, gamma);
                break;
            }
            case 5: {
                picture->ditherSierra(bit, gamma);
                break;
            }
            case 6: {
                picture->ditherAtkinson(bit, gamma);
                break;
            }
            case 7: {
                picture->ditherHalftone(bit, gamma);
                break;
            }
            default: {

            }
        }
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
    try {
        delete inputFileName; // fix 4 ?
        delete outputFileName;
        delete picture;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}