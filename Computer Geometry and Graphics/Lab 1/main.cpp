#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <cmath>

using byte = unsigned char;

class PNMImage {
private:
    std::vector<byte> Buffer;

    uint64_t Size, Width, Height, ColourDepth;
    uint8_t Type;
public:
    std::vector<byte> ImageData;
    static std::vector<byte> ReadBinary(const char* path, uint64_t length) {
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

    static void WriteBinary(const char* path, const std::vector<byte>& vector) {
        std::ofstream os(path, std::ios::binary);
        os.write(reinterpret_cast<const char*>(&vector[0]), vector.size());
    }

    PNMImage(const char* path) {
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
                if (numbers.size() > 3) {
                    flag = 3;
                    ImageData.push_back(c);
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

        //OUTPUT
        std::cout << "Type: P" << numbers[0] << std::endl
                  << "Size: "  << Size << " bytes" << std::endl
                  << "Width: " << Width << "px" << std::endl
                  << "Height: "<< Height<< "px" << std::endl
                  << "Colour Depth: "<< ColourDepth << "bits" << std::endl;
    }

    void Export(const char* path) {
        std::cout << "Exporting..." << std::endl;
        Buffer.clear();
        Buffer.push_back('P');
        Buffer.push_back(char(Type + '0'));
        Buffer.push_back('\n');

        std::string Width__ = std::to_string(Width);
        std::string Height__ = std::to_string(Height);
        std::string ColourDepth__ = std::to_string(ColourDepth);

        for (auto c : Width__) {
            Buffer.push_back(c);
        }
        Buffer.push_back(' ');
        for (auto c : Height__) {
            Buffer.push_back(c);
        }
        Buffer.push_back('\n');
        for (auto c : ColourDepth__) {
            Buffer.push_back(c);
        }
        Buffer.push_back(' ');
        for (auto c : ImageData) {
            Buffer.push_back(c);
        }

        WriteBinary(path, Buffer);

        std::cout << "Export Successful!" << std::endl;
    }

    void Invert() {
        for (auto & i : ImageData) {
            i = ~i;
        }
        std::cout << "Inverting..." << std::endl;
    }

    void Mirror(int direction) {
        // 0 - horizontal
        // 1 - vertical
        if (direction == 0) {
            if (Type == 5) {
                uint64_t i,j;
                for (i = 0; i < Height; i++) {
                    for (j = 0; j < Width/2; j++) {
                        std::swap(ImageData[i*Width + j], ImageData[i*Width + Width - 1 - j]);
                    }
                }
                return;
            }
            if (Type == 6) {
                uint64_t i,j;
                for (i = 0; i < Height; i++) {
                    for (j = 0; j < Width*3/2; j+=3) {
                        std::swap(ImageData[i*Width*3 + j], ImageData[i*Width*3 + Width*3 - j - 3]);
                        std::swap(ImageData[i*Width*3 + j + 1], ImageData[i*Width*3 + Width*3 - j - 2]);
                        std::swap(ImageData[i*Width*3 + j + 2], ImageData[i*Width*3 + Width*3 - j - 1]);
                    }
                }
                return;
            }
            return;
        }
        if (direction == 1) {
            if (Type == 5) {
                uint64_t i,j;
                for (i = 0; i < Width; i++) {
                    for (j = 0; j < Height/2; j++) {
                        std::swap(ImageData[j * Width + i], ImageData[(Height - 1 - j) * Width + i]);
                        /*
                         * 00 01 02 03 04 05 06
                        0*  0  0  0  0  1  1  1
                         * 07 08 09 10 11 12 13
                        1*  1  0  1  0  0  1  0
                         * 14 15 16 17 18 19 20
                        2*  0  0  1  1  0  1  0
                         *  */
                    }
                }
                return;
            }
            if (Type == 6) {
                uint64_t i,j;
                for (i = 0; i < Width*3; i++) {
                    for (j = 0; j < Height/2; j++) {
                        std::swap(ImageData[j * Width*3 + i], ImageData[(Height - 1 - j) * Width*3 + i]);
                        /*
                         *    00 01 02  03 04 05  06 07 08  09 10 11  12 13 14   width = 5
                         * 0   0  3  3   7  8  9   3  1  2   1  5  4   0  f  5
                         *    15 16 17  18 19 20  21 22 23  24 25 26  27 28 29
                         * 1   0  3  6   4  7  3   2  8  2   9  e  e   3  f  f
                         *    30 31 32  33 34 35  36 37 38  39 40 41  42 43 44
                         * 2   6  3  5   f  f  f   e  c  a   f  e  7    a  b  3
                         *
                         * height = 3
                         * */
                    }
                }
                return;
            }
            return;
        }

    }
    void Rotate(int direction) {
        // 0 - clockwise
        // 1 - counterclockwise
        if (direction == 0) {
            if (Type == 5) {
                std::vector<byte> NewImageData;
                uint64_t NewWidth, NewHeight;
                NewHeight = Width;
                NewWidth = Height;
                NewImageData.resize(Width * Height);
                for (uint64_t i = 0; i < Height; i++) {
                    for (uint64_t j = 0; j < Width; j++) {
                        NewImageData[(NewWidth - i - 1) + j*NewWidth] = ImageData[i*Width + j];
                    }
                }
                Width = NewWidth;
                Height = NewHeight;
                ImageData = NewImageData;
                return;
            }
            if (Type == 6) {
                std::vector<byte> NewImageData;
                uint64_t NewWidth, NewHeight;
                NewHeight = Width;
                NewWidth = Height;
                NewImageData.resize(Width * Height * 9);
                for (uint64_t i = 0; i < Height; i++) {
                    for (uint64_t j = 0; j < Width; j++) {
                        NewImageData[j*3 + i*NewWidth*3] = ImageData[(Height - 1 - j)*Width*3 + i*3];
                        NewImageData[j*3 + 1 + i*NewWidth*3] = ImageData[(Height - 1 - j)*Width*3 + i*3 + 1];
                        NewImageData[j*3 + 2 + i*NewWidth*3] = ImageData[(Height - 1 - j)*Width*3 + i*3 + 2];
                    }
                }

                Width = NewWidth;
                Height = NewHeight;
                ImageData = NewImageData;
                /*
                         *    00 01 02  03 04 05  06 07 08  09 10 11  12 13 14   width = 5
                         * 0   0  3  3   7  8  9   3  1  2   1  5  4   0  f  5
                         *    15 16 17  18 19 20  21 22 23  24 25 26  27 28 29
                         * 1   0  3  6   4  7  3   2  8  2   9  e  e   3  f  f
                         *    30 31 32  33 34 35  36 37 38  39 40 41  42 43 44
                         * 2   6  3  5   f  f  f   e  c  a   f  e  7    a  b  3
                         *
                         * height = 3
                         *
                         *    00 01 02  03 04 05  06 07 08
                         *     6  3  5   0  3  6   0  3  3
                         *    09 10 11  12 13 14  15 16 17
                         *     f  f  f   4  7  3   7  8  9
                         *    18 19 20  21 22 23  24 25 26
                         *     e  c  a   2  8  2   3  1  2
                         *    27 28 29  30 31 32  33 34 35
                         *     f  e  7   9  e  e   1  5  4
                         *    36 37 38  39 40 41  42 43 44
                         *     a  b  3   3  f  f   0  f  5
                         * */
                return;
            }
            return;
        }
        if (direction == 1) {
            if (Type == 5) {
                std::vector<byte> NewImageData;
                uint64_t NewWidth, NewHeight;
                NewHeight = Width;
                NewWidth = Height;
                NewImageData.resize(Width * Height);
                for (uint64_t i = 0; i < Height; i++) {
                    for (uint64_t j = 0; j < Width; j++) {
                        NewImageData[j*NewWidth + i] = ImageData[(i + 1)*Width - 1 - j];
                    }
                }
                Width = NewWidth;
                Height = NewHeight;
                ImageData = NewImageData;
                /*
                 * 00 01 02 03 04 05 06  width = 7
                0*  0  0  0  0  1  1  1
                 * 07 08 09 10 11 12 13
                1*  1  0  1  0  0  1  0
                 * 14 15 16 17 18 19 20
                2*  0  0  1  1  0  1  0
                 *
                 * height = 3
                 *
                 * 00 01 02
                 *  1  0  0
                 * 03 04 05
                 *  1  1  1
                 * 06 07 08
                 *  1  0  0
                 * 09 10 11
                 *  0  0  1
                 * 12 13 14
                 *  0  1  1
                 * 15 16 17
                 *  0  0  0
                 * 18 19 20
                 *  0  1  0
                 * */

                return;
            }
            if (Type == 6) {
                std::vector<byte> NewImageData;
                uint64_t NewWidth, NewHeight;
                NewHeight = Width;
                NewWidth = Height;
                NewImageData.resize(Width * Height * 9);
                for (uint64_t i = 0; i < Height; i++) {
                    for (uint64_t j = 0; j < Width; j++) {
                        NewImageData[(NewHeight - 1 - j)*NewWidth*3 + i*3] = ImageData[i*Width*3 + j*3];
                        NewImageData[(NewHeight - 1 - j)*NewWidth*3 + i*3 + 1] = ImageData[i*Width*3 + j*3 + 1];
                        NewImageData[(NewHeight - 1 - j)*NewWidth*3 + i*3 + 2] = ImageData[i*Width*3 + j*3 + 2];
                    }
                }

                Width = NewWidth;
                Height = NewHeight;
                ImageData = NewImageData;
                /*
                         *    00 01 02  03 04 05  06 07 08  09 10 11  12 13 14   width = 5
                         * 0   0  3  3   7  8  9   3  1  2   1  5  4   0  f  5
                         *    15 16 17  18 19 20  21 22 23  24 25 26  27 28 29
                         * 1   0  3  6   4  7  3   2  8  2   9  e  e   3  f  f
                         *    30 31 32  33 34 35  36 37 38  39 40 41  42 43 44
                         * 2   6  3  5   f  f  f   e  c  a   f  e  7    a  b  3
                         *
                         * height = 3
                         *
                         *    00 01 02  03 04 05  06 07 08
                         *     0  f  5   3  f  f   a  b  3
                         *    09 10 11  12 13 14  15 16 17
                         *     1  5  4   9  e  e   f  e  7
                         *    18 19 20  21 22 23  24 25 26
                         *     3  1  2   2  8  2   e  c  a
                         *    27 28 29  30 31 32  33 34 35
                         *     7  8  9   4  7  3   f  f  f
                         *    36 37 38  39 40 41  42 43 44
                         *     0  3  3   0  3  6   6  3  5
                         * */
                return;
            }
            return;
        }
    }
};

int main(int argc, char** argv) {

    if (argc < 3) {
        std::cout << "Error: Not enough arguments!" << std::endl;
        return 0;
    }

    PNMImage image(argv[1]);


//    for (auto c : image.ImageData) {
//        std::cout << c;
//    }
//    std::cout << std::endl;

    switch (argv[3][0]) {
        case '0':
            image.Invert();
            break;
        case '1':
            image.Mirror(0);
            break;
        case '2':
            image.Mirror(1);
            break;
        case '3':
            image.Rotate(0);
            break;
        case '4':
            image.Rotate(1);
            break;
        default:
            std::cout << "Ошибка, невозможно выполнить преобразование!"<< std::endl;
    }

//    for (auto c : image.ImageData) {
//        std::cout << c;
//    }

    image.Export(argv[2]);
//
//    unsigned char c = 0x08;
//    printf("\n%d\n", c);
//    c = ~c;
//    printf("%d\n", c);
    return 0;
}
