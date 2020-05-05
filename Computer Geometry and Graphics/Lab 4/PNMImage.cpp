#include "PNMImage.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>
#include <cmath>
#include <exception>
#include <random>

const double EPS = 1e-5;
using byte = unsigned char;

PNMImage::PNMImage(const char* path) {
    //GET FILE SIZE
    std::error_code ec{};
    Size = std::filesystem::file_size(path, ec);
    if (ec != std::error_code{}) {
        throw std::runtime_error("Error when accessing file.");
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
            ImageData.push_back(c);
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
                    }
                    else if (Buffer[it] == '-') {
                        throw std::runtime_error("Error: negative numbers in header!");
                    }
                    else {
                        break;
                    }
                    it--;
                }
                if (number != -1)
                    numbers.push_back(number);
                if (numbers.size() == 4) {
                    flag = 2;
                }
                Header.push_back('-');
                continue;
            }
            Header.push_back(c);
        }
        if (flag == 1 && numbers.size() < 4) {
            if (c == '\r' || c == '\n') {
                flag = 0;
            }
        }
    }

    //PARSE HEADER
    Type = numbers[0];
    if ((Type != 5 && Type != 6) || Buffer[0] != 'P') {
        throw std::runtime_error("Error: unable to read this file format!");
    }
    Width = numbers[1];
    Height = numbers[2];
    if (Width*Height*(Type - 4 + ((Type + 1) % 2)) != ImageData.size()) {
        throw std::runtime_error("Error: Unexpected EOF!");
    }
    ColourDepth = numbers[3];
    if (ColourDepth != 255) {
        throw std::runtime_error("Error: unable to read this file format!");
    }
}

PNMImage::PNMImage(uint64_t Width_, uint64_t Height_, uint64_t ColourDepth_, uint8_t Type_) {
    Width = Width_;
    Height = Height_;
    ColourDepth = ColourDepth_;
    Type = Type_;
}

PNMImage &PNMImage::operator=(const PNMImage &other) {
    if (this == &other) {
        return *this;
    }
    this->Size = other.Size;
    this->Height = other.Height;
    this->Width = other.Width;
    this->ColourDepth = other.ColourDepth;
    this->Type = other.Type;
    for (auto c : other.ImageData) {
        this->ImageData.push_back(c);
    }
    return *this;
}

PNMImage::PNMImage(const PNMImage& other) {
    this->Size = other.Size;
    this->Height = other.Height;
    this->Width = other.Width;
    this->ColourDepth = other.ColourDepth;
    this->Type = other.Type;
    for (auto c : other.ImageData) {
        this->ImageData.push_back(c);
    }
}

bool PNMImage::operator==(const PNMImage &other) const {
    return (this->Size == other.Size && this->Type == other.Type &&
            this->Width == other.Width && this->Height == other.Height &&
            this->ColourDepth == other.ColourDepth);
}

bool PNMImage::operator!=(const PNMImage &other) const {
    return !(this->Size == other.Size && this->Type == other.Type &&
             this->Width == other.Width && this->Height == other.Height &&
             this->ColourDepth == other.ColourDepth);
}

std::vector<byte> PNMImage::ReadBinary(const char* path, uint64_t length) {
    std::ifstream is(path, std::ios::binary);
    if (!is) {
        throw std::runtime_error("Error when opening file.");
    }
    std::vector<byte> data;
    try {
        data.resize(length);
    } catch (std::exception& e) {
        char* error = nullptr;
        strcat(error, "Buffer error, file too large!\n");
        strcat(error, e.what());
        throw std::runtime_error(error);
    }

    try {
        is.read(reinterpret_cast<char*>(data.data()), length);
    } catch (std::exception& e) {
        char* error = nullptr;
        strcat(error, "Reading error, file could not be read properly!\n");
        strcat(error, e.what());
        throw std::runtime_error(error);
    }
    is.close();
    return data;
}

void PNMImage::WriteBinary(const char* path, const std::vector<byte>& vector) {
    std::ofstream os(path, std::ios::binary);
    if (!os) {
        throw std::runtime_error("Error creating output file!");
    }
    try {
        os.write(reinterpret_cast<const char*>(&vector[0]), vector.size());
    } catch (std::exception& e) {
        char* error = nullptr;
        strcat(error, "Writing error, file could not be written properly!\n");
        strcat(error, e.what());
        throw std::runtime_error(error);
    }
    os.flush();
    os.close();
}

void PNMImage::Export(const char* path) {
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

    Buffer.push_back('\n'); // fix 5 for mac ' '

    for (auto c : ImageData) {
        Buffer.push_back(c);
    }

    WriteBinary(path, Buffer);
}

void PNMImage::Invert() {
    for (auto & i : ImageData) {
        i = ~i;
    }
}

void PNMImage::Mirror(int direction) {
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
    }
    if (direction == 1) {
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
    }
}

void PNMImage::Rotate(int direction) {
    // 0 - clockwise
    // 1 - counterclockwise
    if (direction == 0) {
        if (Type == 5) {
            std::vector<byte> NewImageData;
            uint64_t NewWidth, NewHeight;
            NewHeight = Width;
            NewWidth = Height;
            try {
                NewImageData.resize(Width * Height);
            } catch (std::exception& e) {
                char* error = nullptr;
                strcat(error, "Memory error during rotation!\n");
                strcat(error, e.what());
                throw std::runtime_error(error);
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
                NewImageData.resize(Width * Height * 3);
            } catch (std::exception& e) {
                char* error = nullptr;
                strcat(error, "Memory error during rotation!\n");
                strcat(error, e.what());
                throw std::runtime_error(error);
            }
            for (uint64_t i = 0; i < Height; i++) {
                for (uint64_t j = 0; j < Width; j++) {
                    NewImageData[(NewWidth - i - 1)*3 + j*NewWidth*3] = ImageData[j*3 + Width*3*i];
                    NewImageData[(NewWidth - i - 1)*3 + j*NewWidth*3 + 1] = ImageData[j*3 + Width*3*i + 1];
                    NewImageData[(NewWidth - i - 1)*3 + j*NewWidth*3 + 2] = ImageData[j*3 + Width*3*i + 2];
                }
            }
            Width = NewWidth;
            Height = NewHeight;
            ImageData = NewImageData;
        }
    }
    if (direction == 1) {
        if (Type == 5) {
            std::vector<byte> NewImageData;
            uint64_t NewWidth, NewHeight;
            NewHeight = Width;
            NewWidth = Height;
            try {
                NewImageData.resize(Width * Height);
            } catch (std::exception& e) {
                char* error = nullptr;
                strcat(error, "Memory error during rotation!\n");
                strcat(error, e.what());
                throw std::runtime_error(error);
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
                NewImageData.resize(Width * Height * 3);
            } catch (std::exception& e) {
                char* error = nullptr;
                strcat(error, "Memory error during rotation!\n");
                strcat(error, e.what());
                throw std::runtime_error(error);
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
    }
}

bool PNMImage::isGrey() const {
    return Type == 5;
}

bool PNMImage::isColor() const {
    return Type == 6;
}

void PNMImage::drawPoint(int x, int y, double opacity, byte color, double gamma) { // видимо не умею считать вернул в развернутый вариант
    opacity = std::max(std::min(opacity, 1.0), 0.0);
    if (y < 0 || y >= Height || x < 0 || x >= Width)
        return;
    if (opacity == 0) {
        return;
    }
    if (gamma == 0) {
        double lineColorSRGB = color / 255.0;
        double lineColorLinear = lineColorSRGB <= 0.04045 ? lineColorSRGB / 12.92 : pow((lineColorSRGB + 0.055) / 1.055, 2.4);
        double picColorSRGB = ImageData[Width * y + x] / 255.0;
        double picColorLinear = picColorSRGB <= 0.04045 ? picColorSRGB / 12.92 : pow((picColorSRGB + 0.055) / 1.055, 2.4);
        double c = (1 - opacity) * picColorLinear + opacity * lineColorLinear;
        double cSRGB = c <= 0.0031308 ? 12.92 * c : 1.055 * pow(c, 1 / 2.4) - 0.055;
        ImageData[Width * y + x] = 255 * cSRGB;
    } else {
        double lineColorGamma = color / 255.0;
        double lineColorLinear = pow(lineColorGamma, gamma);
        double picColorGamma = ImageData[Width * y + x] / 255.0;
        double picColorLinear = pow(picColorGamma, gamma);
        double c = (1 - opacity) * picColorLinear + opacity * lineColorLinear;
        double cGamma = pow(c, 1.0 / gamma);
        ImageData[Width * y + x] = 255 * cGamma;
    }
}

void PNMImage::drawThickLine(double x0, double y0, double x1, double y1, byte color, double thiccness, double gamma) {
    if (!isGrey()) {
        throw std::runtime_error("Error: Incorrect color!");
    }
    if (thiccness <= 0)
        return;
    start = {x0,y0};
    end = {x1,y1};

    Point vec = {(end.y - start.y) * 0.5 * thiccness / sqrt((end.y - start.y)*(end.y - start.y) + (start.x - end.x)*(start.x - end.x))
            , (start.x - end.x) * 0.5 * thiccness / sqrt((end.y - start.y)*(end.y - start.y) + (start.x - end.x)*(start.x - end.x))};
    Point A = {start.x + vec.x,start.y + vec.y};
    Point B = {end.x + vec.x, end.y + vec.y};
    Point C = {end.x - vec.x, end.y - vec.y};
    Point D = {start.x - vec.x, start.y - vec.y};
    line = {A, B, C, D}; // vector line
    // drawing raster line
    Point LT{std::min(std::min(A.x, B.x),std::min(C.x, D.x)), std::min(std::min(A.y, B.y),std::min(C.y, D.y))};
    Point RB{std::max(std::max(A.x, B.x),std::max(C.x, D.x)), std::max(std::max(A.y, B.y),std::max(C.y, D.y))};
    for (int x = (int)LT.x - 3; x <= RB.x + 3; x++) {
        for (int y = (int)LT.y - 3; y <= RB.y + 3; y++) {
            drawPoint(x, y, opacity(x, y), color, gamma);
        }
    }
}

double PNMImage::opacity(double x, double y) {
    Point A = {x, y};
    Point B = {x + 1, y};
    Point C = {x + 1, y + 1};
    Point D = {x, y + 1};

    auto checkPoint = [](Rect Rectangle, Point p) -> bool {
        auto triangleArea = [](Point A, Point B, Point C) -> double{
            return sqrt((
                                (
                                        sqrt((A.x - B.x) * (A.x - B.x) + (A.y - B.y) * (A.y - B.y)) +
                                        sqrt((A.x - C.x) * (A.x - C.x) + (A.y - C.y) * (A.y - C.y)) + // p
                                        sqrt((C.x - B.x) * (C.x - B.x) + (C.y - B.y) * (C.y - B.y))
                                ) / 2
                        ) * (
                                (
                                        -sqrt((A.x - B.x) * (A.x - B.x) + (A.y - B.y) * (A.y - B.y)) +
                                        sqrt((A.x - C.x) * (A.x - C.x) + (A.y - C.y) * (A.y - C.y)) + // p - a
                                        sqrt((C.x - B.x) * (C.x - B.x) + (C.y - B.y) * (C.y - B.y))
                                ) / 2
                        ) * (
                                (
                                        sqrt((A.x - B.x) * (A.x - B.x) + (A.y - B.y) * (A.y - B.y)) -
                                        sqrt((A.x - C.x) * (A.x - C.x) + (A.y - C.y) * (A.y - C.y)) + // p - b
                                        sqrt((C.x - B.x) * (C.x - B.x) + (C.y - B.y) * (C.y - B.y))
                                ) / 2
                        ) * (
                                (
                                        sqrt((A.x - B.x) * (A.x - B.x) + (A.y - B.y) * (A.y - B.y)) +
                                        sqrt((A.x - C.x) * (A.x - C.x) + (A.y - C.y) * (A.y - C.y)) - // p - c
                                        sqrt((C.x - B.x) * (C.x - B.x) + (C.y - B.y) * (C.y - B.y))
                                ) / 2
                        ));
        };
        return fabs(
                sqrt((Rectangle.A.x - Rectangle.D.x) * (Rectangle.A.x - Rectangle.D.x) +
                     (Rectangle.A.y - Rectangle.D.y) * (Rectangle.A.y - Rectangle.D.y)) *
                sqrt((Rectangle.A.x - Rectangle.B.x) * (Rectangle.A.x - Rectangle.B.x) +
                     (Rectangle.A.y - Rectangle.B.y) * (Rectangle.A.y - Rectangle.B.y))
                -
                triangleArea(p, Rectangle.A, Rectangle.B) - triangleArea(p, Rectangle.B, Rectangle.C) -
                triangleArea(p, Rectangle.C, Rectangle.D) - triangleArea(p, Rectangle.D, Rectangle.A)
        ) < EPS;
    };
    if (checkPoint(line, A) &&
        checkPoint(line, B) &&
        checkPoint(line, C) &&
        checkPoint(line, D))
        return 1;
    double area = 0;
    for (double i = x; i < x+1; i += 0.1) {
        for (double j = y; j < y+1; j += 0.1) {
            if (checkPoint(line, {i,j})) {
                area += 0.01;
            }
        }
    }

    return area;
}

byte& PNMImage::pixel(int y, int x) {
    if (x < 0 || y < 0 || y >= Height || x >= Width)
        throw std::runtime_error("Index out of bounds!");
    return ImageData[Width * y + x];
}

double PNMImage::decodeGamma(double value, double gamma) {
    return gamma == 0 ? value <= 0.04045 ? value / 12.92 : pow((value + 0.055) / 1.055, 2.4) : pow(value, gamma);
}

double PNMImage::encodeGamma(double value, double gamma) {
    return gamma == 0 ? value <= 0.0031308 ? 12.92 * value : 1.055 * pow(value, 1 / 2.4) - 0.055 : pow(value,
                                                                                                       1.0 / gamma);
}

double PNMImage::closestPaletteColor(byte px, byte bitRate) {
    int t = bitRate;
    byte result = px;
    while (t < 8) {
        result = result>>bitRate;
        result += (px>>(8-bitRate))<<(8-bitRate);
        t += bitRate;
    }
    return result;
}

void PNMImage::fillGradient(double gamma) {
    for (int i = 0; i < Height; ++i) {
        for (int j = 0; j < Width; ++j) {
            pixel(i, j) = encodeGamma((double)j/(Width - 1.0), gamma)*255; // fix gradient 0-255 not 254
        }
    }
}

void PNMImage::ditherNone(byte bitRate, double gamma) {
    for (int i = 0; i < Height; ++i) { // fix 1
        for (int j = 0; j < Width; ++j) {
            double value = decodeGamma(pixel(i, j)/255.0, gamma);
            value = std::min(std::max(value, 0.0), 1.0);
            double newPaletteColor = closestPaletteColor((byte)(value*255), bitRate);
            pixel(i, j) = (byte)(encodeGamma(newPaletteColor/255, gamma)*255);
        }
    }
}

void PNMImage::ditherOrdered(byte bitRate, double gamma) {
    const double orderedMatrix[8][8] = {
            {1.0 / 64.0, 49.0 / 64.0, 13.0 / 64.0, 61.0 / 64.0, 4.0 / 64.0, 52.0 / 64.0, 16.0 / 64.0, 64.0 / 64.0},
            {33.0 / 64.0, 17.0 / 64.0, 45.0 / 64.0, 29.0 / 64.0, 36.0 / 64.0, 20.0 / 64.0, 48.0 / 64.0, 32.0 / 64.0},
            {9.0 / 64.0, 57.0 / 64.0, 5.0 / 64.0, 53.0 / 64.0, 12.0 / 64.0, 60.0 / 64.0, 8.0 / 64.0, 56.0 / 64.0},
            {41.0 / 64.0, 25.0 / 64.0, 37.0 / 64.0, 21.0 / 64.0, 44.0 / 64.0, 28.0 / 64.0, 40.0 / 64.0, 24.0 / 64.0},
            {3.0 / 64.0, 51.0 / 64.0, 15.0 / 64.0, 63.0 / 64.0, 2.0 / 64.0, 50.0 / 64.0, 14.0 / 64.0, 62.0 / 64.0},
            {35.0 / 64.0, 19.0 / 64.0, 47.0 / 64.0, 31.0 / 64.0, 34.0 / 64.0, 18.0 / 64.0, 46.0 / 64.0, 30.0 / 64.0},
            {11.0 / 64.0, 59.0 / 64.0, 7.0 / 64.0, 55.0 / 64.0, 10.0 / 64.0, 58.0 / 64.0, 6.0 / 64.0, 54.0 / 64.0},
            {43.0 / 64.0, 27.0 / 64.0, 39.0 / 64.0, 23.0 / 64.0, 42.0 / 64.0, 26.0 / 64.0, 38.0 / 64.0, 22.0 / 64.0}
    };
    for (int i = 0; i < Height; i++) {
        for (int j = 0; j < Width; j++) {
            double value = decodeGamma(pixel(i, j)/255.0, gamma);
            value = value + (orderedMatrix[i % 8][j % 8] - 0.5) / bitRate;
            value = std::min(std::max(value, 0.0), 1.0);
            double newPaletteColor = closestPaletteColor((byte)(value*255), bitRate);
            pixel(i, j) = (byte)(encodeGamma(newPaletteColor/255, gamma)*255);
        }
    }
}

void PNMImage::ditherRandom(byte bitRate, double gamma) {
    std::random_device rd;
    std::mt19937 gen(rd());
    for (int i = 0; i < Height; i++) {
        for (int j = 0; j < Width; j++) {
            double value = decodeGamma(pixel(i, j)/255.0, gamma);
            double noise = (double)gen()/UINT32_MAX + 1e-7;
            value = value + (noise - 0.5) / bitRate;
            value = std::min(std::max(value, 0.0), 1.0);
            double newPaletteColor = closestPaletteColor((byte)(value*255), bitRate);
            pixel(i, j) = (byte)(encodeGamma(newPaletteColor/255, gamma)*255);
        }
    }
}

void PNMImage::ditherFloydSteinberg(byte bitRate, double gamma) {
    std::vector<double> errors(Height * Width, 0);
    auto getError = [&](int h, int w) -> double& {
        return errors[h * Width + w];
    };

    for (int i = 0; i < Height; i++) {
        for (int j = 0; j < Width; j++) {
            double value = decodeGamma(pixel(i, j)/255.0, gamma);
            value = value + getError(i, j) / 255.0;
            value = std::min(std::max(value, 0.0), 1.0);

            double newPaletteColor = closestPaletteColor((byte)(value*255), bitRate);

            double error = pixel(i, j) + getError(i, j) - newPaletteColor;

            pixel(i, j) = (byte)(encodeGamma(newPaletteColor/255, gamma)*255);

            if (j + 1 < Width)
                getError(i, j + 1) += error * 7.0 / 16.0;
            if (i + 1 < Height && j + 1 < Width)
                getError(i + 1, j + 1) += error * 1.0 / 16.0;
            if (i + 1 < Height)
                getError(i + 1, j) += error * 5.0 / 16.0;
            if (i + 1 < Height && j - 1 >= 0)
                getError(i + 1, j - 1) += error * 3.0 / 16.0;
        }
    }
}

void PNMImage::ditherJJN(byte bitRate, double gamma) {
    const double matrixJJN[3][5] = {
            {0, 0, 0, 7.0 / 48.0, 5.0 / 48.0},
            {3.0 / 48.0, 5.0 / 48.0, 7.0 / 48.0, 5.0 / 48.0, 3.0 / 48.0},
            {1.0 / 48.0, 3.0 / 48.0, 5.0 / 48.0, 3.0 / 48.0, 1.0 / 48.0}
    };

    std::vector<double> errors(Height * Width, 0);
    auto getError = [&](int h, int w) -> double& {
        return errors[h * Width + w];
    };

    for (int i = 0; i < Height; i++) {
        for (int j = 0; j < Width; j++) {
            double value = decodeGamma(pixel(i, j)/255.0, gamma);
            value = value + getError(i, j) / 255.0;
            value = std::min(std::max(value, 0.0), 1.0);

            double newPaletteColor = closestPaletteColor((byte)(value*255), bitRate);

            double error = pixel(i, j) + getError(i, j) - newPaletteColor;

            pixel(i, j) = (byte)(encodeGamma(newPaletteColor/255, gamma)*255);

            for (int ie = 0; ie < 3; ie++) {
                for (int je = 0; je < 5; je++) {
                    if (i + ie >= Height || j + (je - 2) >= Width || j + (je - 2) < 0)
                        continue; // fix 3
                    if (ie == 0 && je < 3)
                        continue;

                    getError(i + ie, j + (je - 2)) += error * matrixJJN[ie][je];
                }
            }
        }
    }
}

void PNMImage::ditherSierra(byte bitRate, double gamma) {
    const double matrixSierra3[3][5] = {
            {0, 0, 0, 5.0 / 32.0, 3.0 / 32.0},
            {2.0 / 32.0, 4.0/ 32.0, 5.0 / 32.0, 4.0 / 32.0, 2.0 / 32.0},
            {0, 2.0 / 32.0, 3.0 / 32.0, 2.0 / 32.0, 0}
    };

    std::vector<double> errors(Height * Width, 0);
    auto getError = [&](int h, int w) -> double& {
        return errors[h * Width + w];
    };

    for (int i = 0; i < Height; i++) {
        for (int j = 0; j < Width; j++) {
            double value = decodeGamma(pixel(i, j)/255.0, gamma);
            value = value + getError(i, j) / 255.0;
            value = std::min(std::max(value, 0.0), 1.0);

            double newPaletteColor = closestPaletteColor((byte)(value*255), bitRate);

            double error = pixel(i, j) + getError(i, j) - newPaletteColor;

            pixel(i, j) = (byte)(encodeGamma(newPaletteColor/255, gamma)*255);

            for (int ie = 0; ie < 3; ie++) {
                for (int je = 0; je < 5; je++) {
                    if (i + ie >= Height || j + (je - 2) >= Width || j + (je - 2) < 0)
                        continue; // fix 3
                    if (ie == 0 && je <= 2)
                        continue;

                    getError(i + ie, j + (je - 2)) += error * matrixSierra3[ie][je];
                }
            }
        }
    }
}

void PNMImage::ditherAtkinson(byte bitRate, double gamma) {
    const int matrixAtkinson[3][5] = {
            {0, 0, 0, 1, 1},
            {0, 1, 1, 1, 0},
            {0, 0, 1, 0, 0}
    };

    std::vector<double> errors(Height * Width, 0);
    auto getError = [&](int h, int w) -> double& {
        return errors[h * Width + w];
    };

    for (int i = 0; i < Height; i++) {
        for (int j = 0; j < Width; j++) {
            double value = decodeGamma(pixel(i, j)/255.0, gamma);
            value = value + getError(i, j) / 255.0;
            value = std::min(std::max(value, 0.0), 1.0);

            double newPaletteColor = closestPaletteColor((byte)(value*255), bitRate);

            double error = pixel(i, j) + getError(i, j) - newPaletteColor;

            pixel(i, j) = (byte)(encodeGamma(newPaletteColor/255, gamma)*255);

            for (int ie = 0; ie < 3; ie++) {
                for (int je = 0; je < 5; je++) {
                    if (i + ie >= Height || j + (je - 2) >= Width || j + (je - 2) < 0)
                        continue; // fix 3
                    if (ie == 0 && je <= 2)
                        continue;

                    getError(i + ie, j + (je - 2)) += error * matrixAtkinson[ie][je] / 8.0;
                }
            }
        }
    }
}

void PNMImage::ditherHalftone(byte bitRate, double gamma) {
//    const double halftoneMatrix[4][4] = {7 / 16.0, 13 / 16.0, 11 / 16.0, 4 / 16.0,
//                                          12 / 16.0, 16 / 16.0, 14 / 16.0, 8 / 16.0,
//                                          10 / 16.0, 15 / 16.0, 6 / 16.0, 2 / 16.0,
//                                          5 / 16.0, 9 / 16.0, 3 / 16.0, 1 / 16.0};
    const double halftoneMatrix[4][4] = {7 / 17.0, 13 / 17.0, 11 / 17.0, 4 / 17.0, // fix 2
                                         12 / 17.0, 16 / 17.0, 14 / 17.0, 8 / 17.0,
                                         10 / 17.0, 15 / 17.0, 6 / 17.0, 2 / 17.0,
                                         5 / 17.0, 9 / 17.0, 3 / 17.0, 1 / 17.0};
    for (int i = 0; i < Height; i++) {
        for (int j = 0; j < Width; j++) {
            double value = decodeGamma(pixel(i, j)/255.0, gamma);
            value = value + (halftoneMatrix[i % 4][j % 4] - 0.5) / bitRate;
            value = std::min(std::max(value, 0.0), 1.0);
            double newPaletteColor = closestPaletteColor((byte)(value*255), bitRate);
            pixel(i, j) = (byte)(encodeGamma(newPaletteColor/255, gamma)*255);
        }
    }
}

PNMImage PNMImage::mergeBytes(const PNMImage &source1, const PNMImage &source2, const PNMImage &source3) {
    if (source1 != source2 || source2 != source3) {
        throw std::runtime_error("Error, merging images' headers do not match!");
    }
    if (!source1.isGrey()) {
        throw std::runtime_error("Error, merging images are not grey!");
    }

    PNMImage result(source1.Width, source1.Height, source1.ColourDepth, 6);

    for(int i = 0; i < result.Height*result.Width; i++) {
        result.ImageData.push_back(source1.ImageData[i]);
        result.ImageData.push_back(source2.ImageData[i]);
        result.ImageData.push_back(source3.ImageData[i]);
    }

    return result;
}

PNMImage PNMImage::pull1stByte(const PNMImage &source) {
    if (!source.isColor()) {
        throw std::runtime_error("Error, splitted image is not color!");
    }

    PNMImage result(source.Width, source.Height, source.ColourDepth, 5);

    for(int i = 0; i < source.ImageData.size(); i+=3) {
        result.ImageData.push_back(source.ImageData[i]);
    }

    return result;
}

PNMImage PNMImage::pull2ndByte(const PNMImage &source) {
    if (!source.isColor()) {
        throw std::runtime_error("Error, splitted image is not color!");
    }

    PNMImage result(source.Width, source.Height, source.ColourDepth, 5);

    for(int i = 1; i < source.ImageData.size(); i+=3) {
        result.ImageData.push_back(source.ImageData[i]);
    }

    return result;
}

PNMImage PNMImage::pull3rdByte(const PNMImage &source) {
    if (!source.isColor()) {
        throw std::runtime_error("Error, splitted image is not color!");
    }

    PNMImage result(source.Width, source.Height, source.ColourDepth, 5);

    for(int i = 2; i < source.ImageData.size(); i+=3) {
        result.ImageData.push_back(source.ImageData[i]);
    }

    return result;
}

void PNMImage::convertColorSpace(char *from, char *to) {
    std::vector<byte> result;
    std::vector<byte> RGB;

    auto roundToByte = [](int val) -> byte{
        return (byte)std::min(std::max(val, 0), 255);
    };

    if (!strcmp(from, "RGB")) {
         RGB = ImageData;
    } else if (!strcmp(from, "HSL")) {
        RGB.resize(ImageData.size());
        for (int i = 0; i < ImageData.size(); i+=3) {
            double H = ImageData[i]/255.0 * 6;
            double S = ImageData[i+1]/255.0;
            double L = ImageData[i+2]/255.0;
            double C = (1 - abs(2*L - 1)) * S;
            double X = C * (1 - abs((int)(H)%2 + (H - (int)H) - 1));
            double m = L - C / 2.0;
            double R,G,B;
            if (std::ceil(H) - 1.0 < 1e-4) {
                R = C; G = X; B = 0;
            } else if (std::ceil(H) - 2.0 < 1e-4) {
                R = X; G = C; B = 0;
            } else if (std::ceil(H) - 3.0 < 1e-4) {
                R = 0; G = C; B = X;
            } else if (std::ceil(H) - 4.0 < 1e-4) {
                R = 0; G = X; B = C;
            } else if (std::ceil(H) - 5.0 < 1e-4) {
                R = X; G = 0; B = C;
            } else if (std::ceil(H) - 6.0 < 1e-4) {
                R = C; G = 0; B = X;
            } else {
                throw std::runtime_error("Error H value out of range!");
            }
            RGB[i]   = roundToByte((int)((R+m)*255));
            RGB[i+1] = roundToByte((int)((G+m)*255));
            RGB[i+2] = roundToByte((int)((B+m)*255));
        }
    } else if (!strcmp(from, "HSV")) {
        RGB.resize(ImageData.size());
        for (int i = 0; i < ImageData.size(); i+=3) {
            double H = ImageData[i]/255.0*6;
            double S = ImageData[i+1]/255.0;
            double V = ImageData[i+2]/255.0;
            const double PI = 3.1416926;
            double C = V * S;
            double X = C * (1 - abs((int)(H) % 2 +(H-(int)H)- 1));
            double m = V - C;
            double R,G,B;
            if (H >= 0.0 && H <= 1.0) {
                R = C; G = X; B = 0;
            } else if (H >= 1.0 && H <= 2.0) {
                R = X; G = C; B = 0;
            } else if (H >= 2.0 && H <= 3.0) {
                R = 0; G = C; B = X;
            } else if (H >= 3.0 && H <= 4.0) {
                R = 0; G = X; B = C;
            } else if (H >= 4.0 && H <= 5.0) {
                R = X; G = 0; B = C;
            } else if (H >= 5.0 && H <= 6.0) {
                R = C; G = 0; B = X;
            } else {
                throw std::runtime_error("Error H value out of range!");
            }
            RGB[i]   = roundToByte((int)((R+m)*255));
            RGB[i+1] = roundToByte((int)((G+m)*255));
            RGB[i+2] = roundToByte((int)((B+m)*255));
        }
    } else if (!strcmp(from, "YCbCr.601")) {
        double Kb = 0.299;
        double Kr = 0.587;
        double Kg = 0.114;
        RGB.resize(ImageData.size());
        for (int i = 0; i < ImageData.size(); i+=3) {
            double Y =  ImageData[i]     / 255.0;
            double Cb = ImageData[i + 1] / 255.0 - 0.5;
            double Cr = ImageData[i + 2] / 255.0 - 0.5;
            double R = Y + (2 - 2 * Kr) * Cr;
            double G = Y - Kb * (2 - 2 * Kb) * Cb / Kg - Kr * (2 - 2 * Kr) * Cr / Kg;
            double B = Y + (2 - 2 * Kb) * Cb;
            RGB[i]   = roundToByte((int)(R*255));
            RGB[i+1] = roundToByte((int)(G*255));
            RGB[i+2] = roundToByte((int)(B*255));
        }
    } else if (!strcmp(from, "YCbCr.709")) {
        double Kb = 0.0722;
        double Kr = 0.2126;
        double Kg = 0.7152;
        RGB.resize(ImageData.size());
        for (int i = 0; i < ImageData.size(); i+=3) {
            double Y =   ImageData[i]     / 255.0;
            double Cb = ImageData[i + 1] / 255.0 - 0.5;
            double Cr = ImageData[i + 2] / 255.0 - 0.5;
            double R = Y + (2 - 2 * Kr) * Cr;
            double G = Y - Kb * (2 - 2 * Kb) * Cb / Kg - Kr * (2 - 2 * Kr) * Cr / Kg;
            double B = Y + (2 - 2 * Kb) * Cb;
            RGB[i]   = roundToByte((int)(R*255));
            RGB[i+1] = roundToByte((int)(G*255));
            RGB[i+2] = roundToByte((int)(B*255));
        }
    } else if (!strcmp(from, "YCoCg")) {
        RGB.resize(ImageData.size());
        for (int i = 0; i < ImageData.size(); i+=3) {
            double Y = ImageData[i]/255.0;
            double Co = ImageData[i+1]/255.0 - 0.5;
            double Cg = ImageData[i+2]/255.0 - 0.5;
            RGB[i]   = roundToByte((int)((Y+Co-Cg)*255));
            RGB[i+1] = roundToByte((int)((Y+Cg)*255));
            RGB[i+2] = roundToByte((int)((Y-Co-Cg)*255));
        }
    } else if (!strcmp(from, "CMY")) {
        RGB.resize(ImageData.size());
        for (int i = 0; i < ImageData.size(); i+=3) {
            double C = ImageData[i]/255.0;
            double M = ImageData[i+1]/255.0;
            double Y = ImageData[i+2]/255.0;
            RGB[i]   = roundToByte((int)((1-C)*255));
            RGB[i+1] = roundToByte((int)((1-M)*255));
            RGB[i+2] = roundToByte((int)((1-Y)*255));
        }
    } else {
        throw std::runtime_error("Error, unsupported input color space!");
    }

    if (!strcmp(to, "RGB")) {
        result = RGB;
    } else if (!strcmp(to, "HSL")) {
        result.resize(RGB.size()); // to HSL
        for (int i = 0; i < RGB.size(); i+=3) {
            double R = RGB[i]/255.0;
            double G = RGB[i + 1]/255.0;
            double B = RGB[i + 2]/255.0;
            double Cmin = std::min(R, std::min(G, B));
            double Cmax = std::max(R, std::max(G, B));
            double delta = Cmax - Cmin;
            double L = (Cmax + Cmin)/2.0;
            double S = 0, H = 0;
            if (L == 0 || L == 1) {
                S = 0;
            } else {
                S = (Cmax - L)/std::min(1.0, 1 - L);
            }

            if (delta == 0) {
                H = 0;
            } else if (Cmax == R) {
                H = (G-B)/delta;
            } else if (Cmax == G) {
                H = (B-R)/delta + 2;
            } else if (Cmax == B) {
                H = (R-G)/delta + 4;
            } else {
                throw std::runtime_error("Error unable to convert to HSL");
            }
            H *= 60;
            if (H < 0)  H += 360;
            result[i] = roundToByte((int)(H/360.0*255));
            result[i+1] = roundToByte((int)(S*255));
            result[i+2] = roundToByte((int)(L*255));
        }
    } else if (!strcmp(to, "HSV")) {
        result.resize(RGB.size()); // to HSV
        for (int i = 0; i < RGB.size(); i+=3) {
            double R = RGB[i]/255.0;
            double G = RGB[i + 1]/255.0;
            double B = RGB[i + 2]/255.0;
            double Cmin = std::min(R, std::min(G, B));
            double Cmax = std::max(R, std::max(G, B));
            double delta = Cmax - Cmin;
            double V = Cmax;
            double S = 0, H = 0;
            if (V == 0) {
                S = 0;
            } else {
                S = delta/Cmax;
            }
            if (delta == 0) {
                H = 0;
            } else if (Cmax == R) {
                H = (G-B)/delta;
            } else if (Cmax == G) {
                H = (B-R)/delta + 2;
            } else if (Cmax == B) {
                H = (R-G)/delta + 4;
            } else {
                throw std::runtime_error("Error unable to convert to HSL");
            }
            H *= 60;
            if (H<0) H+=360;
            result[i] = roundToByte((int)(H/360*255));
            result[i+1] = roundToByte((int)(S*255));
            result[i+2] = roundToByte((int)(V*255));
        }
    } else if (!strcmp(to, "YCbCr.601")) {
        double Kb = 0.299;
        double Kr = 0.587;
        double Kg = 0.114;
        result.resize(RGB.size()); // to YCbCr.601
        for (int i = 0; i < RGB.size(); i+=3) {
            double R = RGB[i] / 255.0;
            double G = RGB[i + 1] / 255.0;
            double B = RGB[i + 2] / 255.0;
            double Y =   Kr * R + Kg * G + Kb * B;
            double Cb = (B - Y) / (2 * (1 - Kb));
            double Cr = (R - Y) / (2 * (1 - Kr));
            result[i]   = roundToByte((int)(Y*255)); // Y
            result[i+1] = roundToByte((int)((Cb+0.5)*255)); // Cb
            result[i+2] = roundToByte((int)((Cr+0.5)*255)); // Cr
        }
    } else if (!strcmp(to, "YCbCr.709")) {
        double Kb = 0.0722;
        double Kr = 0.2126;
        double Kg = 0.7152;
        result.resize(RGB.size()); // to YCbCr.601
        for (int i = 0; i < RGB.size(); i+=3) {
            double R = RGB[i] / 255.0;
            double G = RGB[i + 1] / 255.0;
            double B = RGB[i + 2] / 255.0;
            double Y =   Kr * R + Kg * G + Kb * B;
            double Cb = (B - Y) / (2 * (1 - Kb));
            double Cr = (R - Y) / (2 * (1 - Kr));
            result[i]   = roundToByte((int)(Y*255)); // Y
            result[i+1] = roundToByte((int)((Cb+0.5)*255)); // Cb
            result[i+2] = roundToByte((int)((Cr+0.5)*255)); // Cr
        }
    } else if (!strcmp(to, "YCoCg")) {
        result.resize(RGB.size()); // to YCoCg
        for (int i = 0; i < RGB.size(); i+=3) {
            double R = RGB[i]/255.0;
            double G = RGB[i + 1]/255.0;
            double B = RGB[i + 2]/255.0;
            result[i]   = roundToByte((int)((R/4.0 + G/2.0 + B/4.0)*255)); // Y
            result[i+1] = roundToByte((int)((R/2.0 - B/2.0 + 0.5)*255)); // Co
            result[i+2] = roundToByte((int)((-R/4.0 + G/2.0 - B/4.0 + 0.5)*255)); // Cg
        }
    } else if (!strcmp(to, "CMY")) {
        result.resize(RGB.size()); // to YCoCg
        for (int i = 0; i < RGB.size(); i+=3) {
            double R = RGB[i]/255.0;
            double G = RGB[i + 1]/255.0;
            double B = RGB[i + 2]/255.0;
            result[i]   = roundToByte((int)((1.0-R)*255)); // C
            result[i+1] = roundToByte((int)((1.0-G)*255)); // M
            result[i+2] = roundToByte((int)((1.0-B)*255)); // Y
        }
    } else {
        throw std::runtime_error("Error, unsupported input color space!");
    }
    ImageData = result;
}
