#include <iostream>
#include <string>
#include "PNMImage.h"

using byte = unsigned char;

int main(int argc, char* argv[]) {
    if (argc != 11) {
        std::cerr << "Incorrect number of arguments" << std::endl;
        return 1;
    }

    char *inputFileName = nullptr, *outputFileName = nullptr;
    char *inputColorSpace = nullptr, *outputColorSpace = nullptr;
    int inputCount, outputCount;
    try {
        for (int i = 1; i < 11; ++i) {
            char *inputType = strdup(argv[i++]);
            if (inputType[0] != '-') {
                std::cerr << "Incorrect arguments" << std::endl;
                delete inputType;
                return 1;
            }
            switch (inputType[1]) {
                case 'f': {
                    inputColorSpace = strdup(argv[i]);
                    break;
                }
                case 't': {
                    outputColorSpace = strdup(argv[i]);
                    break;
                }
                case 'i': {
                    inputCount = std::stoi(argv[i++]);
                    inputFileName = strdup(argv[i]);
                    break;
                }
                case 'o': {
                    outputCount = std::stoi(argv[i++]);
                    outputFileName = strdup(argv[i]);
                    break;
                }
                default: {
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        delete inputColorSpace;
        delete outputColorSpace;
        delete inputFileName;
        delete outputFileName;
        return 1;
    }
    PNMImage* main = nullptr;
    if (inputCount == 3) {
        try {
            int l = strlen(inputFileName)-4;
            if (inputFileName[l]!='.' || inputFileName[l+1]!='p' || inputFileName[l+2]!='g' || inputFileName[l+3]!='m') {
                std::cerr << "Error, invalid input format";
                delete inputColorSpace;
                delete outputColorSpace;
                delete inputFileName;
                delete outputFileName;
                delete main;
                return 1;
            }
            std::string newIntputName(inputFileName);
            newIntputName = newIntputName.substr(0, newIntputName.find('.'));
            char *input1 = strdup(newIntputName.c_str());
            char *input2 = strdup(newIntputName.c_str());
            char *input3 = strdup(newIntputName.c_str());
            strcat(input1, "_1.pgm");
            strcat(input2, "_2.pgm");
            strcat(input3, "_3.pgm");
            PNMImage im1(input1);
            PNMImage im2(input2);
            PNMImage im3(input3);
            main = new PNMImage(PNMImage::mergeBytes(im1, im2, im3));
            delete input1;
            delete input2;
            delete input3;
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            delete inputColorSpace;
            delete outputColorSpace;
            delete inputFileName;
            delete outputFileName;
            delete main;
            return 1;
        }
    } else if (inputCount == 1){
        try {
            int l = strlen(inputFileName)-4;
            if (inputFileName[l]!='.' || inputFileName[l+1]!='p' || inputFileName[l+2]!='p' || inputFileName[l+3]!='m') {
                std::cerr << "Error, invalid input format";
                delete inputColorSpace;
                delete outputColorSpace;
                delete inputFileName;
                delete outputFileName;
                delete main;
                return 1;
            }
            main = new PNMImage(inputFileName);
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            delete inputColorSpace;
            delete outputColorSpace;
            delete inputFileName;
            delete outputFileName;
            delete main;
            return 1;
        }
    } else {
        delete inputColorSpace;
        delete outputColorSpace;
        delete inputFileName;
        delete outputFileName;
        delete main;
        std::cerr << "Error, invalid input count!" << std::endl;
        return 1;
    }

    try {
        main->convertColorSpace(inputColorSpace, outputColorSpace);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        delete inputColorSpace;
        delete outputColorSpace;
        delete inputFileName;
        delete outputFileName;
        delete main;
        return 1;
    }

    if (outputCount == 3) {
        try {
            PNMImage im1 = PNMImage::pull1stByte(*main);
            PNMImage im2 = PNMImage::pull2ndByte(*main);
            PNMImage im3 = PNMImage::pull3rdByte(*main);
            int l = strlen(outputFileName) - 4;
            if (outputFileName[l]!='.' || outputFileName[l+1]!='p' || outputFileName[l+2]!='g' || outputFileName[l+3]!='m') {
                std::cerr << "Error, invalid output format";
                delete inputColorSpace;
                delete outputColorSpace;
                delete inputFileName;
                delete outputFileName;
                delete main;
                return 1;
            }
            std::string newOutputName(outputFileName);
            newOutputName = newOutputName.substr(0, newOutputName.find('.'));
            char *output1 = strdup(newOutputName.c_str());
            char *output2 = strdup(newOutputName.c_str());
            char *output3 = strdup(newOutputName.c_str());
            strcat(output1, "_1.pgm");
            strcat(output2, "_2.pgm");
            strcat(output3, "_3.pgm");
            im1.Export(output1);
            im2.Export(output2);
            im3.Export(output3);
            delete output1;
            delete output2;
            delete output3;
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            delete inputColorSpace;
            delete outputColorSpace;
            delete inputFileName;
            delete outputFileName;
            delete main;
            return 1;
        }
    } else if (outputCount == 1) {
        try {
            int l = strlen(outputFileName) - 4;
            if (outputFileName[l]!='.' || outputFileName[l+1]!='p' || outputFileName[l+2]!='p' || outputFileName[l+3]!='m') {
                std::cerr << "Error, invalid output format";
                delete inputColorSpace;
                delete outputColorSpace;
                delete inputFileName;
                delete outputFileName;
                delete main;
                return 1;
            }
            main->Export(outputFileName);
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            delete inputColorSpace;
            delete outputColorSpace;
            delete inputFileName;
            delete outputFileName;
            delete main;
            return 1;
        }
    } else {
        std::cerr << "Error, invalid output count!" << std::endl;
        delete inputColorSpace;
        delete outputColorSpace;
        delete inputFileName;
        delete outputFileName;
        delete main;
        return 1;
    }

    return 0;
}