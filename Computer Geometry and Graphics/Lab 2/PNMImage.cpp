//
// Created by Егор Назаров on 09.03.2020.
//

#include "PNMImage.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>
#include <cmath>
#include <exception>

const double EPS = 1e-5;

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

bool PNMImage::isGrey() {
    return Type == 5;
}

bool PNMImage::isColor() {
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

double PNMImage::opacity(double x, double y) { //// разобраться и дописать(!)
    Point A = {x-0.5, y-0.5};
    Point B = {x+0.5, y-0.5};
    Point C = {x+0.5, y+0.5};
    Point D = {x-0.5, y+0.5};

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
    for (double i = x-0.45; i <= x+0.45; i += 0.1) {
        for (double j = y-0.45; j <= y+0.45; j += 0.1) {
            if (checkPoint(line, {i,j})) {
                area += 0.01;
            }
        }
    }

    return area;
}
