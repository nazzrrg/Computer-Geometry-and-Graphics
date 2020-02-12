#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

using byte = unsigned char;

class PNMImage {
private:
    std::vector<byte> Buffer;
    char Type;
    uint64_t Size;
    uint8_t BytesPerPx;
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
        std::error_code ec{};
        Size = std::filesystem::file_size(path, ec);
        if (ec != std::error_code{})
            std::cout << "error when accessing test file, size is: " << Size << " message: " << ec.message() << '\n';

        Buffer = ReadBinary(path, Size);

        Type = Buffer[1];
        if (Type == '6') {
            BytesPerPx = 3;
        } else if (Type == '5') {
            BytesPerPx = 1;
        } else {
            std::cout << "Не удалось открыть файл: формат файла npm не поддерживается" << std::endl;
        }
        std::cout << "Type: P" << Type << std::endl;
        std::cout << "Size: " << Size << " bytes" << std::endl;
        for (auto c : Buffer) {
            std::cout << c;
        }
    }
};

int main(int argc, char** argv) {

    PNMImage image(argv[1]);

    return 0;
}
