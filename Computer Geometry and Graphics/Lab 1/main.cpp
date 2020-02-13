#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <cmath>

using byte = unsigned char;

class PNMImage {
private:
    std::vector<byte> Buffer;
    std::vector<byte> ImageData;
    uint64_t Size, Width, Height, ColourDepth;
    uint8_t Type;
public:
    static std::vector<byte> ReadBinary (const char* path, uint64_t length){
        std::ifstream is(path, std::ios::binary);
        std::vector<byte> data(length);
        is.read(reinterpret_cast<char*>(data.data()), length);
        if (is.gcount() == 0) {
            // We have to check that reading from the stream actually worked.
            // If any of the stream operation above failed then `gcount()`
            // will return zero indicating that zero data was read from the stream
            std::cout << "Не удалось открыть файл" << std::endl;
            exit(0);
        }

        return data;
    }

    PNMImage (const char* path) {
        //GET FILE SIZE
        std::error_code ec{};
        Size = std::filesystem::file_size(path, ec);
        if (ec != std::error_code{})
            std::cout << "error when accessing test file, size is: " << Size << " message: " << ec.message() << '\n';

        //READ BUFFER
        Buffer = ReadBinary(path, Size);

        //PARSE BUFFER
        int flag = 0; // comment flag 0 - not in comment
                      // 1 - in comment
                      // 2 - endline and/or spaces after header
                      // 3 - in data
        char c_prev = '0';
        std::vector<byte> Header;
        std::vector<int> numbers;
        for (int i = 0; i < Buffer.size(); i++) {
            char c = Buffer[i];
            if (flag == 3) {
                ImageData.push_back(c);
                continue;
            }
            if (!(c == '\r' || c == '\n' || c == ' ' || c == '\0') && flag == 2) {
                flag = 3;
                ImageData.push_back(c);
                continue;
            }
            if (flag == 0) {
                if (c == '#' && numbers.size() < 3) {
                    flag = 1; // in comment
                    continue;
                }
                if (c == '\r' || c == '\n' || c == ' ' || c == '\0') {
                    if (numbers.size() > 3) {
                        flag = 2;
                        continue;
                    }
                    int32_t number = -1;
                    int it = i - 1;
                    while (it > 0) {
                        if ('0' <= Buffer[it] && Buffer[it] <= '9') {
                            if (number == -1) number++;
                            number += (Buffer[it] - '0') * (int)pow(10, i - it - 1);
                        } else {
                            break;
                        }
                        it--;
                    }
                    if (number != -1) numbers.push_back(number);

                    Header.push_back('-');
                    continue;
                }
                Header.push_back(c);
            } else if (flag == 1 && numbers.size() < 4) {
                if (c == '\r' || c == '\n') {
                    flag = 0;
                }
            }
        }

        //PARSE HEADER
        Type = numbers[0];
        Width = numbers[1];
        Height = numbers[2];
        ColourDepth = numbers[3];

        //TMP OUTPUT
        std::cout << "Type: P" << numbers[0] << std::endl
                  << "Size: "  << Size << " bytes" << std::endl
                  << "Width: " << Width << "px" << std::endl
                  << "Height: "<< Height<< "px" << std::endl
                  << "Colour Depth: "<< ColourDepth << "bits" << std::endl;

//        std::cout << std::endl;
//        for (auto c : ImageData) {
//            std::cout << c;
//        }


    }
};

int main(int argc, char** argv) {

    PNMImage image(argv[1]);

    return 0;
}
