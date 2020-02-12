#include <iostream>
#include <fstream>
#include <filesystem>

class PNMImage {
private:
    char* buffer;
public:
    PNMImage (const char* path) {
        std::error_code ec{};
        auto size = std::filesystem::file_size(path, ec);
        if (ec == std::error_code{})
            std::cout << "size: " << size << '\n';
        else
            std::cout << "error when accessing test file, size is: "
                      << size << " message: " << ec.message() << '\n';
    }

};

int main(int argc, char** argv) {

    PNMImage image(argv[1]);

    return 0;
}
