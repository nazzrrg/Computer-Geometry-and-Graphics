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
        if (!is) {
            std::cout << "Error when opening file." << std::endl;
            exit(1);
        }
        std::vector<byte> data;
        try {
            data.resize(length);
        } catch (std::exception& e) {
            std::cout << "Buffer error, file too large!" << std::endl;
            std::cerr << "Error: " << e.what( ) << std::endl;
            std::cerr << "Type " << typeid( e ).name( ) << std::endl;
            exit (1);
        }

        try {
            is.read(reinterpret_cast<char*>(data.data()), length);
        } catch (std::exception& e) {
            std::cout << "Reading error, file could not be read properly!" << std::endl;
            std::cerr << "Error: " << e.what( ) << std::endl;
            std::cerr << "Type " << typeid( e ).name( ) << std::endl;
            exit (1);
        }

        is.close();
        return data;
    }

    static void WriteBinary(const char* path, const std::vector<byte>& vector) {
        std::ofstream os(path, std::ios::binary);
        if (!os) {
            std::cout << "Error creating output file!" << std::endl;
            exit(1);
        }
        try {
            os.write(reinterpret_cast<const char*>(&vector[0]), vector.size());
        } catch (std::exception& e) {
            std::cout << "Writing error, file could not be written properly!" << std::endl;
            std::cerr << "Error: " << e.what( ) << std::endl;
            std::cerr << "Type " << typeid( e ).name( ) << std::endl;
            exit (1);
        }
    }

    explicit PNMImage(const char* path) {
        //GET FILE SIZE
        std::error_code ec{};
        Size = std::filesystem::file_size(path, ec);
        if (ec != std::error_code{}) {
            std::cout << "Error when accessing file. Message: " << ec.message() << '\n';
            exit(1);
        }
        //READ BUFFER
        Buffer = ReadBinary(path, Size);

        //PARSE BUFFER
        int flag = 0; // comment flag 0 - not in comment
                      // 1 - in comment
                      // 2 - in data
        std::vector<byte> Header;
        std::vector<int> numbers;
        for (int i = 0; i < Buffer.size(); i++) {
            char c = Buffer[i];
            if (flag == 2) {
                try {
                    ImageData.push_back(c);
                } catch (std::exception& e) {
                    std::cout << "Buffer error!" << std::endl;
                    std::cerr << "Error: " << e.what( ) << std::endl;
                    std::cerr << "Type " << typeid( e ).name( ) << std::endl;
                    exit (1);
                }
            }
            if (flag == 0) {
                if (c == '#' && numbers.size() < 4) {
                    flag = 1; // in comment
                    continue;
                }
                if (c == '\r' || c == '\n' || c == ' ' || c == '\0') {
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
                    if (number != -1)
                        try {
                            numbers.push_back(number);
                        } catch (std::exception& e) {
                            std::cout << "Buffer error!" << std::endl;
                            std::cerr << "Error: " << e.what() << std::endl;
                            std::cerr << "Type " << typeid(e).name() << std::endl;
                            exit(1);
                        }
                    if (numbers.size() == 4) {
                        flag = 2;
                    }
                    try {
                        Header.push_back('-');
                    } catch (std::exception& e) {
                        std::cout << "Buffer error!" << std::endl;
                        std::cerr << "Error: " << e.what() << std::endl;
                        std::cerr << "Type " << typeid(e).name() << std::endl;
                        exit(1);
                    }
                    continue;
                }
                try {
                    Header.push_back(c);
                } catch (std::exception& e) {
                    std::cout << "Buffer error!" << std::endl;
                    std::cerr << "Error: " << e.what() << std::endl;
                    std::cerr << "Type " << typeid(e).name() << std::endl;
                    exit(1);
                }
            }
            if (flag == 1 && numbers.size() < 4) {
                if (c == '\r' || c == '\n') {
                    flag = 0;
                }
            }
        }

        //PARSE HEADER
        Type = numbers[0];
         if (Type != 5 && Type != 6) {
             std::cout << "Error: unable to read this file format!" << std::endl;
             exit(1);
         }
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

        try {
            Buffer.push_back('P');
            Buffer.push_back(char(Type + '0'));
            Buffer.push_back('\n');
        } catch (std::exception& e) {
            std::cout << "Buffer error during output!" << std::endl;
            std::cerr << "Error: " << e.what( ) << std::endl;
            std::cerr << "Type " << typeid( e ).name( ) << std::endl;
            exit (1);
        }


        std::string Width__ = std::to_string(Width);
        std::string Height__ = std::to_string(Height);
        std::string ColourDepth__ = std::to_string(ColourDepth);

        for (auto c : Width__) {
            try{
                Buffer.push_back(c);
            } catch (std::exception& e) {
                std::cout << "Buffer error during output!" << std::endl;
                std::cerr << "Error: " << e.what( ) << std::endl;
                std::cerr << "Type " << typeid( e ).name( ) << std::endl;
                exit (1);
            }
        }
        try{
            Buffer.push_back(' ');
        } catch (std::exception& e) {
            std::cout << "Buffer error during output!" << std::endl;
            std::cerr << "Error: " << e.what( ) << std::endl;
            std::cerr << "Type " << typeid( e ).name( ) << std::endl;
            exit (1);
        }
        for (auto c : Height__) {
            try {
                Buffer.push_back(c);
            } catch (std::exception& e) {
                std::cout << "Buffer error during output!" << std::endl;
                std::cerr << "Error: " << e.what( ) << std::endl;
                std::cerr << "Type " << typeid( e ).name( ) << std::endl;
                exit (1);
            }
        }
        try {
            Buffer.push_back('\n');
        } catch (std::exception& e) {
            std::cout << "Buffer error during output!" << std::endl;
            std::cerr << "Error: " << e.what( ) << std::endl;
            std::cerr << "Type " << typeid( e ).name( ) << std::endl;
            exit (1);
        }
        for (auto c : ColourDepth__) {
            try{
                Buffer.push_back(c);
            } catch (std::exception& e) {
                std::cout << "Buffer error during output!" << std::endl;
                std::cerr << "Error: " << e.what( ) << std::endl;
                std::cerr << "Type " << typeid( e ).name( ) << std::endl;
                exit (1);
            }
        }
        try {
            Buffer.push_back(' ');
        } catch (std::exception& e) {
            std::cout << "Buffer error during output!" << std::endl;
            std::cerr << "Error: " << e.what( ) << std::endl;
            std::cerr << "Type " << typeid( e ).name( ) << std::endl;
            exit (1);
        }
        for (auto c : ImageData) {
            try {
                Buffer.push_back(c);
            } catch (std::exception& e) {
                std::cout << "Buffer error during output!" << std::endl;
                std::cerr << "Error: " << e.what( ) << std::endl;
                std::cerr << "Type " << typeid( e ).name( ) << std::endl;
                exit (1);
            }
        }

        WriteBinary(path, Buffer);

        std::cout << "Export Successful!" << std::endl;
    }
    void Invert() {
        std::cout << "Inverting..." << std::endl;
        for (auto & i : ImageData) {
            i = ~i;
        }
        std::cout << "Inverting finished!" << std::endl;
    }
    void Mirror(int direction) {
        // 0 - horizontal
        // 1 - vertical
        if (direction == 0) {
            std::cout << "Mirroring horizontally..." << std::endl;
            if (Type == 5) {
                uint64_t i,j;
                for (i = 0; i < Height; i++) {
                    for (j = 0; j < Width/2; j++) {
                        std::swap(ImageData[i*Width + j], ImageData[i*Width + Width - 1 - j]);
                    }
                }
            } else {
                uint64_t i,j;
                for (i = 0; i < Height; i++) {
                    for (j = 0; j < Width*3/2; j+=3) {
                        std::swap(ImageData[i*Width*3 + j], ImageData[i*Width*3 + Width*3 - j - 3]);
                        std::swap(ImageData[i*Width*3 + j + 1], ImageData[i*Width*3 + Width*3 - j - 2]);
                        std::swap(ImageData[i*Width*3 + j + 2], ImageData[i*Width*3 + Width*3 - j - 1]);
                    }
                }
            }
            std::cout << "Mirroring finished!" << std::endl;
        }
        if (direction == 1) {
            std::cout << "Mirroring vertically..." << std::endl;
            if (Type == 5) {
                uint64_t i,j;
                for (i = 0; i < Width; i++) {
                    for (j = 0; j < Height/2; j++) {
                        std::swap(ImageData[j * Width + i], ImageData[(Height - 1 - j) * Width + i]);
                    }
                }
            } else {
                uint64_t i,j;
                for (i = 0; i < Width*3; i++) {
                    for (j = 0; j < Height/2; j++) {
                        std::swap(ImageData[j * Width*3 + i], ImageData[(Height - 1 - j) * Width*3 + i]);
                    }
                }
            }
            std::cout << "Mirroring finished!" << std::endl;
        }
    }
    void Rotate(int direction) {
        // 0 - clockwise
        // 1 - counterclockwise
        if (direction == 0) {
            std::cout << "Rotating clockwise..." << std::endl;
            if (Type == 5) {
                std::vector<byte> NewImageData;
                uint64_t NewWidth, NewHeight;
                NewHeight = Width;
                NewWidth = Height;
                try {
                    NewImageData.resize(Width * Height);
                } catch (std::exception& e) {
                    std::cout << "Memory error during rotation!" << std::endl;
                    std::cerr << "Error: " << e.what( ) << std::endl;
                    std::cerr << "Type " << typeid( e ).name( ) << std::endl;
                    exit (1);
                }
                for (uint64_t i = 0; i < Height; i++) {
                    for (uint64_t j = 0; j < Width; j++) {
                        NewImageData[(NewWidth - i - 1) + j*NewWidth] = ImageData[i*Width + j];
                    }
                }
                Width = NewWidth;
                Height = NewHeight;
                ImageData = NewImageData;
            } else {
                std::vector<byte> NewImageData;
                uint64_t NewWidth, NewHeight;
                NewHeight = Width;
                NewWidth = Height;
                try {
                    NewImageData.resize(Width * Height * 9);
                } catch (std::exception& e) {
                    std::cout << "Memory error during rotation!" << std::endl;
                    std::cerr << "Error: " << e.what( ) << std::endl;
                    std::cerr << "Type " << typeid( e ).name( ) << std::endl;
                    exit (1);
                }
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
            }
            std::cout << "Rotating finished!" << std::endl;
        }
        if (direction == 1) {
            std::cout << "Rotating counterclockwise..." << std::endl;
            if (Type == 5) {
                std::vector<byte> NewImageData;
                uint64_t NewWidth, NewHeight;
                NewHeight = Width;
                NewWidth = Height;
                try {
                    NewImageData.resize(Width * Height);
                } catch (std::exception& e) {
                    std::cout << "Memory error during rotation!" << std::endl;
                    std::cerr << "Error: " << e.what( ) << std::endl;
                    std::cerr << "Type " << typeid( e ).name( ) << std::endl;
                    exit (1);
                }
                for (uint64_t i = 0; i < Height; i++) {
                    for (uint64_t j = 0; j < Width; j++) {
                        NewImageData[j*NewWidth + i] = ImageData[(i + 1)*Width - 1 - j];
                    }
                }
                Width = NewWidth;
                Height = NewHeight;
                ImageData = NewImageData;
            } else {
                std::vector<byte> NewImageData;
                uint64_t NewWidth, NewHeight;
                NewHeight = Width;
                NewWidth = Height;
                try {
                    NewImageData.resize(Width * Height * 9);
                } catch (std::exception& e) {
                    std::cout << "Memory error during rotation!" << std::endl;
                    std::cerr << "Error: " << e.what( ) << std::endl;
                    std::cerr << "Type " << typeid( e ).name( ) << std::endl;
                    exit (1);
                }
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
            }
            std::cout << "Rotating finished!" << std::endl;
        }
    }
};

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout << "Error: not enough arguments!" << std::endl;
        exit(1);
    }
    int l = strlen(argv[1]);
    if (argv[1][l-1] != 'm' || argv[1][l-2] != 'n' || argv[1][l-3] != 'p' ||argv[1][l-4] != '.') {
        std::cout << "Error: unable to read this file format!" << std::endl;
        exit(1);
    }
    PNMImage image(argv[1]);
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
            std::cout << "Error: unknown command!"<< std::endl;
    }
    image.Export(argv[2]);
    return 0;
}
