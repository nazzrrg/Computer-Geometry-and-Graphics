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

std::vector<byte> PNMImage::ReadBinary(const char* path, uint64_t length) {
    std::ifstream is(path, std::ios::binary);
    if (!is) {
        std::cerr << "Error when opening file." << std::endl;
        exit(1);
    }
    std::vector<byte> data;
    try {
        data.resize(length);
    } catch (std::exception& e) {
        std::cerr << "Buffer error, file too large!" << std::endl;
        std::cerr << "Error: " << e.what( ) << std::endl;
        std::cerr << "Type " << typeid( e ).name( ) << std::endl;
        exit (1);
    }

    try {
        is.read(reinterpret_cast<char*>(data.data()), length);
    } catch (std::exception& e) {
        std::cerr << "Reading error, file could not be read properly!" << std::endl;
        std::cerr << "Error: " << e.what( ) << std::endl;
        std::cerr << "Type " << typeid( e ).name( ) << std::endl;
        exit (1);
    }

    is.close();
    return data;
}

void PNMImage::WriteBinary(const char* path, const std::vector<byte>& vector) {
    std::ofstream os(path, std::ios::binary);
    if (!os) {
        std::cerr << "Error creating output file!" << std::endl;
        exit(1);
    }
    try {
        os.write(reinterpret_cast<const char*>(&vector[0]), vector.size());
    } catch (std::exception& e) {
        std::cerr << "Writing error, file could not be written properly!" << std::endl;
        std::cerr << "Error: " << e.what( ) << std::endl;
        std::cerr << "Type " << typeid( e ).name( ) << std::endl;
        exit (1);
    }
}

PNMImage::PNMImage(const char* path) {
    //GET FILE SIZE
    std::error_code ec{};
    Size = std::filesystem::file_size(path, ec);
    if (ec != std::error_code{}) {
        std::cerr << "Error when accessing file. Message: " << ec.message() << '\n';
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
                        std::cerr << "Error: negative numbers in header!" << std::endl;
                        exit(1);
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
        std::cerr << "Error: unable to read this file format!" << std::endl;
        exit(1);
    }
    Width = numbers[1];
    Height = numbers[2];
    if (Width*Height*(Type - 4 + ((Type + 1) % 2)) != ImageData.size()) {
        std::cerr << "Error: Unexpected EOF!" << std::endl; // 11
        exit(1);
    }
    ColourDepth = numbers[3];
    if (ColourDepth != 255) {
        std::cerr << "Error: unable to read this file format!" << std::endl;
        exit(1);
    }

    //OUTPUT
//    std::cout << "Type: P" << numbers[0] << std::endl
//              << "Size: "  << Size << " bytes" << std::endl
//              << "Width: " << Width << "px" << std::endl
//              << "Height: "<< Height<< "px" << std::endl
//              << "Colour Depth: "<< ColourDepth << "bits" << std::endl;
}

void PNMImage::Export(const char* path) {
//    std::cout << "Exporting..." << std::endl;
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

//    std::cout << "Export Successful!" << std::endl;
}

void PNMImage::Invert() {
//    std::cout << "Inverting..." << std::endl;
    for (auto & i : ImageData) {
        i = ~i;
    }
//    std::cout << "Inverting finished!" << std::endl;
}

void PNMImage::Mirror(int direction) {
    // 0 - horizontal
    // 1 - vertical
    if (direction == 0) {
//        std::cout << "Mirroring horizontally..." << std::endl;
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
//        std::cout << "Mirroring finished!" << std::endl;
    }
    if (direction == 1) {
//        std::cout << "Mirroring vertically..." << std::endl;
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
//        std::cout << "Mirroring finished!" << std::endl;
    }
}

void PNMImage::Rotate(int direction) {
    // 0 - clockwise
    // 1 - counterclockwise
    if (direction == 0) {
//        std::cout << "Rotating clockwise..." << std::endl;
        if (Type == 5) {
            std::vector<byte> NewImageData;
            uint64_t NewWidth, NewHeight;
            NewHeight = Width;
            NewWidth = Height;
            try {
                NewImageData.resize(Width * Height);
            } catch (std::exception& e) {
                std::cerr << "Memory error during rotation!" << std::endl;
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
                NewImageData.resize(Width * Height * 3);
            } catch (std::exception& e) {
                std::cerr << "Memory error during rotation!" << std::endl;
                std::cerr << "Error: " << e.what( ) << std::endl;
                std::cerr << "Type " << typeid( e ).name( ) << std::endl;
                exit (1);
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
//        std::cout << "Rotating finished!" << std::endl;
    }
    if (direction == 1) {
//        std::cout << "Rotating counterclockwise..." << std::endl;
        if (Type == 5) {
            std::vector<byte> NewImageData;
            uint64_t NewWidth, NewHeight;
            NewHeight = Width;
            NewWidth = Height;
            try {
                NewImageData.resize(Width * Height);
            } catch (std::exception& e) {
                std::cerr << "Memory error during rotation!" << std::endl;
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
                NewImageData.resize(Width * Height * 3);
            } catch (std::exception& e) {
                std::cerr << "Memory error during rotation!" << std::endl;
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
//        std::cout << "Rotating finished!" << std::endl;
    }
}

bool PNMImage::isGrey() {
    return Type == 5;
}

bool PNMImage::isColor() {
    return Type == 6;
}

void PNMImage::thiccOctant(int x0, int y0, int x1, int y1, int thiccness, byte color, double gamma) {
    if (steep) { // by y
        int dx = x1 - x0;
        int dy = y1 - y0;
        int pError = 0;
        int error = 0;
        int y = y0;
        int x = x0;
        int length = dy + 1;
        if (dx < 0) { // down left
            dx *= -1; ////
            int threshold = dy - 2 * dx;//// выше
            int Ediag = -2 * dy;
            int Esquare = 2 * dx;//// выше
            for (int p = 0; p < length; p++) {
                pOctant(x, y, -dx, dy, pError, thiccness, error, color, gamma);////
                if (error > threshold) {
                    x--;
                    error += Ediag;
                    if (pError > threshold) {
                        pOctant(x, y, -dx, dy, pError + Ediag + Esquare, thiccness, error, color, gamma);////
                        pError += Ediag;
                    }
                    pError += Esquare;
                }
                error += Esquare;
                y++;
            }
        } else { // down right
            int threshold = dy - 2 * dx;
            int Ediag = -2 * dy;
            int Esquare = 2 * dx;
            for (int p = 0; p < length; p++) {
                pOctant(x, y, dx, dy, pError, thiccness, error, color, gamma);
                if (error > threshold) {
                    x++;
                    error += Ediag;
                    if (pError > threshold) {
                        pOctant(x, y, dx, dy, pError + Ediag + Esquare, thiccness, error, color, gamma);
                        pError += Ediag;
                    }
                    pError += Esquare;
                }
                error += Esquare;
                y++;
            }
        }
    } else { // by x (traditional)
        int dx = x1 - x0;
        int dy = y1 - y0;
        int pError = 0;
        int error = 0;
        int y = y0;
        int x = x0;
        int length = dx + 1;
        if (dy < 0) { // right up
            dy *= -1;
            int threshold = dx - 2 * dy;////!!! перед dy - -- выше
            int Ediag = -2 * dx;
            int Esquare = 2 * dy;////!!! перед dy - -- выше
            for (int p = 0; p < length; p++) {
//                drawPoint(x, y, 1, color, gamma);
                pOctant(x, y, dx, -dy, pError, thiccness, error, color, gamma);//// -dy
                if (error > threshold) {
                    y--;
                    error += Ediag;
                    if (pError > threshold) {
                        pOctant(x, y, dx, -dy, pError + Ediag + Esquare, thiccness, error, color, gamma);//// -dy
                        pError += Ediag;
                    }
                    pError += Esquare;
                }
                error += Esquare;
                x++;
            }
        } else { // right down
            int threshold = dx - 2 * dy;
            int Ediag = -2 * dx;
            int Esquare = 2 * dy;
            for (int p = 0; p < length; p++) {
                pOctant(x, y, dx, dy, pError, thiccness, error, color, gamma);
                if (error > threshold) {
                    y++;
                    error += Ediag;
                    if (pError > threshold) {
                        pOctant(x, y, dx, dy, pError + Ediag + Esquare, thiccness, error, color, gamma);
                        pError += Ediag;
                    }
                    pError += Esquare;
                }
                error += Esquare;
                x++;
            }
        }
    }
}

void PNMImage::pOctant(int x0, int y0, int dx, int dy, int errorInit, int sideWidth, int widthInit, byte color, double gamma) {
    double wthr = 2 * ((int) floor((sideWidth / 2)) + 1) * sqrt(dx * dx + dy * dy);
    int y = y0;
    int x = x0;

    if (steep) { // by y
        if (dx < 0) {
            dx *=-1; ////
            int threshold = dy - 2 * dx;
            int Ediag = -2 * dy;
            int Esquare = 2 * dx;
            int error = -errorInit;////

            int tk = dx + dy + widthInit;////

            while (tk <= wthr) {
                drawPoint(x, y, 1, color, gamma);
                if (error > threshold) {
                    y++;////
                    error += Ediag;
                    tk += 2 * dx;
                }
                error += Esquare;
                x++;
                tk += 2 * dy;
            }

            x = x0;////
            y = y0;////
            error = errorInit;////
            tk = dx + dy - widthInit;////

            while (tk <= wthr) {
                drawPoint(x, y, 1, color, gamma);
                if (error > threshold) {
                    y--;////
                    error += Ediag;
                    tk += 2 * dx;
                }
                error += Esquare;
                x--;
                tk += 2 * dy;
            }
        } else {
            int threshold = dy - 2 * dx;
            int Ediag = -2 * dy;
            int Esquare = 2 * dx;
            int error = errorInit;

            int tk = dx + dy - widthInit;

            while (tk <= wthr) {
                drawPoint(x, y, 1, color, gamma);////
                if (error > threshold) {
                    y--;
                    error += Ediag;
                    tk += 2 * dx;
                }
                error += Esquare;
                x++;
                tk += 2 * dy;
            }

            x = x0;////
            y = y0;////
            error = -errorInit;
            tk = dx + dy + widthInit;

            while (tk <= wthr) {
                drawPoint(x, y, 1, color, gamma);////
                if (error > threshold) {
                    y++;
                    error += Ediag;
                    tk += 2 * dx;
                }
                error += Esquare;
                x--;
                tk += 2 * dy;
            }
        }
    } else { // by x traditional
        if (dy < 0) {
            dy *= -1; //// -1*dy
            int threshold = dx - 2 * dy;
            int Ediag = -2 * dx;
            int Esquare = 2 * dy;
            int error = -errorInit; //// - перед init
            int tk = dx + dy + widthInit;//// + перед W
            while (tk <= wthr) {
                drawPoint(x, y, 1, color, gamma);
                if (error > threshold) {
                    x++;//// ++
                    error += Ediag;
                    tk += 2 * dy;
                }
                error += Esquare;
                y++;
                tk += 2 * dx;
            }

            x = x0;
            y = y0;
            error = errorInit; //// + перед init
            tk = dx + dy - widthInit; //// - перед W

            while (tk <= wthr) {
                drawPoint(x, y, 1, color, gamma);
                if (error > threshold) {
                    x--;//// --
                    error += Ediag;
                    tk += 2 * dy;
                }
                error += Esquare;
                y--;
                tk += 2 * dx;
            }
        } else {
            int threshold = dx - 2 * dy;
            int Ediag = -2 * dx;
            int Esquare = 2 * dy;
            int error = errorInit;

            int tk = dx + dy - widthInit;
//        int tk = dx + dy + widthInit;
            while (tk <= wthr) {
                drawPoint(x, y, 1, color, gamma);
                if (error > threshold) {
                    x--;
//                x++;
                    error += Ediag;
                    tk += 2 * dy;
                }
                error += Esquare;
                y++;
                tk += 2 * dx;
            }

            x = x0;
            y = y0;
            error = -errorInit;
            tk = dx + dy + widthInit;
//        tk = dx + dy - widthInit;

            while (tk <= wthr) {
                drawPoint(x, y, 1, color, gamma);
                if (error > threshold) {
                    x++;
                    error += Ediag;
                    tk += 2 * dy;
                }
                error += Esquare;
                y--;
                tk += 2 * dx;
            }
        }
    }
}
/*
void PNMImage::drawLine(Point start, Point end, byte color, double thiccness, double gamma) {
    bool steep = abs(end.y - start.y) > abs(end.x - start.x);

    auto intPart = [](double x) -> int {return (int) x;};
    auto plot = [&](int x, int y, double intensity) -> void {
        if (gamma == 0) {
            if (steep)
                drawPoint(y, x, 1.0 - intensity, color);
            else
                drawPoint(x, y, 1.0 - intensity, color);
        }
        else {
            if (steep)
                drawPoint(y, x, 1.0 - intensity, color, gamma);
            else
                drawPoint(x, y, 1.0 - intensity, color, gamma);
        }
    };
    auto distance = [](Point a, Point b) -> double {
        return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
    };

    if (steep) {
        std::swap(start.x, start.y);
        std::swap(end.x, end.y);
    }
    if (start.x > end.x) {
        std::swap(start.x, end.x);
        std::swap(start.y, end.y);
    }

    double dx = end.x - start.x;
    double dy = end.y - start.y;

    double gradient = dy / dx;

    double y = start.y + gradient * (round(start.x) - start.x);



    if (gradient == 1) {
        for(int plotX = round(start.x); plotX <= round(end.x); plotX++) {
            for (int plotY = intPart(y - (thiccness - 1) / 2); plotY <= intPart(y + (thiccness + 1) / 2); plotY++)
            {
                plot(plotX, plotY, std::min(1.0, (thiccness + 1.0) / 2.0 - fabs(y - plotY)));
            }
            y += gradient;
        }
    } else {
        for (int plotX = round(start.x); plotX <= round(end.x); plotX++) {
            for (int plotY = intPart(y - (thiccness - 1) / 2); plotY <= intPart(y + (thiccness + 1) / 2); plotY++) {
                plot(plotX, plotY, std::min(1.0, (thiccness + 1.0) / 2.0 - fabs(y - plotY)));
            }
            y += gradient;
        }
    }

    Point plotStart = {round(start.x), round(start.y)};
    for (int plotX = round(start.x) - thiccness / 2; plotX < round(start.x); plotX++) {
        y = start.y + gradient * (plotX - start.x);
        for (int plotY = int(y - (thiccness - 1) / 2.0); plotY <= int(y + (thiccness + 1) / 2.0); plotY++) {
            plot(plotX, plotY, std::min(1.0, (thiccness + 0.5) / 2.0 -
                                        distance({(double) plotX, (double) plotY}, {plotStart.x, plotStart.y})));
        }
    }

    Point plotEnd = {round(end.x), round(end.y)};
    for (int plotX = round(end.x) + 1; plotX <= round(end.x) + thiccness / 2; plotX++) {
        y = start.y + gradient * (plotX - start.x);
        for (int plotY = int(y - (thiccness - 1) / 2.0); plotY <= int(y + (thiccness + 1) / 2.0); plotY++) {
            plot(plotX, plotY, std::min(1.0, (thiccness + 0.5) / 2.0 -
                                        distance({(double) plotX, (double) plotY}, {plotEnd.x, plotEnd.y})));
        }
    }
}

void PNMImage::drawLine(double x0, double y0, double x1, double y1, byte color, double thiccness, double gamma) {
    if (!isGrey()) {
        std::cerr << "Error: Incorrect color!" << std::endl;
        exit(1);
    }
    if (thiccness <= 0)
        return;
//    drawLine({x0, y0}, {x1, y1}, color, thiccness, gamma);
    thiccOctant(x0, y0, x1, y1, thiccness, color, gamma);
}
 */

//    что-то тут не так? яркость и фон - в гамме то есть сначала надо перевести ее в линию а затем обратно в гамму
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
        std::cerr << "Error: Incorrect color!" << std::endl;
        exit(1);
    }
    if (thiccness <= 0)
        return;
    start = {x0,y0};
    end = {x1,y1};
    steep = abs(end.y - start.y) > abs(end.x - start.x);
//    if (!steep && start.x > end.x) {
//        std::swap(start.x, end.x);
//        std::swap(start.y, end.y);
//    }
//    if (steep && start.y > end.y) {
//        std::swap(start.x, end.x);
//        std::swap(start.y, end.y);
//    }



    thiccOctant(round(start.x), round(start.y), round(end.x), round(end.y), thiccness, color, gamma);
    /*
    beg = {x0, y0};
    end = {x1, y1};
    drawLine(beg.x, beg.y, end.x, end.y, color, gamma);
    double dx = x1 - x0;
    double dy = y1 - y0;
    double k = 0.5/sqrt(dx*dx + dy*dy);

    double accmThk = 1;
    Point startP = beg, endP = end;
    while (accmThk <= thiccness/2) {
        startP.x += k * dy;
        startP.y -= k * dx;
        endP.x += k * dy;
        endP.y -= k * dx;
        drawLine(startP.x, startP.y, endP.x, endP.y, color, gamma);
        accmThk += 0.5;
    } */
}

double PNMImage::opacity(double x1, double y1, double x2, double y2, double x0, double y0, int pos) { //// разобраться и дописать(!)

}

//void PNMImage::drawLine(double x0, double y0, double x1, double y1, byte color, double gamma) {
//    int xs = (int)round(x0);
//    int ys = (int)round(y0);
//    int xe = (int)round(x1);
//    int ye = (int)round(y1);
//    int dx = xe - xs;
//    int dy = ye - ys;
//    int y = ys;
//    int error = 0;
//    for (int x = xs; x < xe; x++) {
//        drawPoint(x, y, 1, color, gamma);
//        error += 2*dy;
//        if (error > dx) {
//            y++;
//            error -= 2*dx;
//        }
//    }
//}

