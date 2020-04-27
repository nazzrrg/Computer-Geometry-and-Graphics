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
#include <set>

const double EPS = 1e-7;

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
    }
}

bool PNMImage::isGrey() {
    return Type == 5;
}

bool PNMImage::isColor() {
    return Type == 6;
}

void PNMImage::thiccOctant(Point start, Point end, int thiccness, byte color, double gamma) {
    double ddx = end.x - start.x;
    double ddy = end.y - start.y;
    start.x += -ddx * 2/sqrt(ddx*ddx + ddy*ddy);
    start.y += -ddy * 2/sqrt(ddx*ddx + ddy*ddy);
    end.x += ddx * 2/sqrt(ddx*ddx + ddy*ddy);
    end.y += ddy * 2/sqrt(ddx*ddx + ddy*ddy);
    int dx = (int)round(ddx);
    int dy = (int)round(ddy);
    int pError = 0;
    int error = 0;
    int y = (int)round(start.y);
    int x = (int)round(start.x);
    if (steep) { // by y
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
//                drawPoint(x, y, 1, color, gamma);
                drawPoint(x, y, opacity(x, y), color, gamma);
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
//                drawPoint(x, y, 1, color, gamma);
                drawPoint(x, y, opacity(x, y), color, gamma);
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
                drawPoint(x, y, opacity(x, y), color, gamma);
//                drawPoint(x, y, 1, color, gamma);
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
                drawPoint(x, y, opacity(x, y), color, gamma);
//                drawPoint(x, y, 1, color, gamma);
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
                drawPoint(x, y, opacity(x, y), color, gamma);
//                drawPoint(x, y, 1, color, gamma);
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
                drawPoint(x, y, opacity(x, y), color, gamma);
//                drawPoint(x, y, 1, color, gamma);
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
            while (tk <= wthr) {
                drawPoint(x, y, opacity(x, y), color, gamma);
//                drawPoint(x, y, 1, color, gamma);
                if (error > threshold) {
                    x--;
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

            while (tk <= wthr) {
                drawPoint(x, y, opacity(x, y), color, gamma);
//                drawPoint(x, y, 1, color, gamma);
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
    if ((steep && (start.y > end.y)) || (!steep && (start.x > end.x))) {
        std::swap(start.x, end.x);
        std::swap(start.y, end.y);
    }

    Point vec = {(end.y - start.y) * 0.5 * thiccness / sqrt((end.y - start.y)*(end.y - start.y) + (start.x - end.x)*(start.x - end.x))
            , (start.x - end.x) * 0.5 * thiccness / sqrt((end.y - start.y)*(end.y - start.y) + (start.x - end.x)*(start.x - end.x))};
    Point A = {start.x + vec.x,start.y + vec.y};
    Point B = {end.x + vec.x, end.y + vec.y};
    Point C = {end.x - vec.x, end.y - vec.y};
    Point D = {start.x - vec.x, start.y - vec.y};
    Edge e1 = {A, B};
    Edge e2 = {B, C};
    Edge e3 = {C, D};
    Edge e4 = {D, A};
    line = {e1, e2, e3, e4}; // vector line
    thiccOctant(start, end, thiccness, color, gamma); // drawing raster line
//    Point LT{std::min(std::min(A.x, B.x),std::min(C.x, D.x)), std::min(std::min(A.y, B.y),std::min(C.y, D.y))};
//    Point RB{std::max(std::max(A.x, B.x),std::max(C.x, D.x)), std::max(std::max(A.y, B.y),std::max(C.y, D.y))};
//    for (int x = LT.x-3; x <= RB.x + 3; x++) {
//        for (int y = LT.y - 3; y < RB.y + 3; y++) {
//            drawPoint(x, y, opacity(x, y), color, gamma);
//        }
//    }
}

double PNMImage::opacity(double x, double y) { //// разобраться и дописать(!)
    Rect pixel = {{{x-0.5, y-0.5}, {x+0.5, y-0.5}}
                , {{x+0.5, y-0.5}, {x+0.5, y+0.5}}
                , {{x+0.5, y+0.5}, {x-0.5, y+0.5}}
                , {{x-0.5, y+0.5}, {x-0.5, y-0.5}}};

    struct PointAngle {
        Point point;
        double angle;
        PointAngle(Point point_, double angle_) : point(point_), angle(angle_) {};
        bool operator<(const PointAngle& rhs) {
            return this->angle < rhs.angle;
        }
    };
    std::vector<PointAngle> Points;
    auto calculateAngle = [](Point x0, Point A) -> double {
        Point vec {A.x - x0.x, A.y - x0.y}; // compared vector
        double mod = sqrt(vec.x*vec.x + vec.y*vec.y);
        double cos = vec.x/mod;
        if (cos < -1) cos = -1;
        if (cos > 1) cos = 1;
        double angle = acos(cos);
        if (vec.y < 0) {
            angle *= -1;
        }
        return angle;
    };
    // find edges intersection
    auto checkEdges = [](Edge A, Edge B) -> Point {
        double a1 = A.b.y - A.a.y;
        double b1 = A.a.x - A.b.x;
        double c1 = a1*(A.a.x) + b1*(A.a.y);

        double a2 = B.b.y - B.a.y;
        double b2 = B.a.x - B.b.x;
        double c2 = a2*(B.a.x)+ b2*(B.a.y);

        double determinant = a1*b2 - a2*b1;

        if (determinant == 0)
        {
            return {-1,-1};
        }
        else
        {
            double x = (b2*c1 - b1*c2)/determinant;
            double y = (a1*c2 - a2*c1)/determinant;
            if (x >= std::min(A.a.x, A.b.x) && x <= std::max(A.a.x, A.b.x)
                    && y >= std::min(A.a.y, A.b.y) && y <= std::max(A.a.y, A.b.y)
                    && x >= std::min(B.a.x, B.b.x) && x <= std::max(B.a.x, B.b.x)
                    && y >= std::min(B.a.y, B.b.y) && y <= std::max(B.a.y, B.b.y)) {
                return {x, y};
            } else {
                return {-1, -1};
            }
        }
    };
    Point test = {0, 0};
    PointAngle x0 = {{-1,-1}, -1};
    auto Find = [] (Point x, const std::vector<PointAngle>& vec) -> bool {
        for (auto item : vec) {
            if (item.point.x == x.x && item.point.y == x.y) {
                return true;
            }
        }
        return false;
    };
    if ((test = checkEdges(line.e1, pixel.e1)).x != -1 && test.y != -1) {
        if (Points.empty()) {
            Points.emplace_back(test, -10000);
            x0 = {test, -10000};
        } else {
            if (!Find(test, Points)) {
                Points.emplace_back(test, calculateAngle(x0.point, test));
            }
        }
    }
    if ((test = checkEdges(line.e1, pixel.e2)).x != -1 && test.y != -1)  {
        if (Points.empty()) {
            Points.emplace_back(test, -10000);
            x0 = {test, -10000};
        } else {
            if (!Find(test, Points)) {
                Points.emplace_back(test, calculateAngle(x0.point, test));
            }
        }
    }
    if ((test = checkEdges(line.e1, pixel.e3)).x != -1 && test.y != -1)  {
        if (Points.empty()) {
            Points.emplace_back(test, -10000);
            x0 = {test, -10000};
        } else {
            if (!Find(test, Points)) {
                Points.emplace_back(test, calculateAngle(x0.point, test));
            }
        }
    }
    if ((test = checkEdges(line.e1, pixel.e4)).x != -1 && test.y != -1)  {
        if (Points.empty()) {
            Points.emplace_back(test, -10000);
            x0 = {test, -10000};
        } else {
            if (!Find(test, Points)) {
                Points.emplace_back(test, calculateAngle(x0.point, test));
            }
        }
    }
    if ((test = checkEdges(line.e2, pixel.e1)).x != -1 && test.y != -1) {
        if (Points.empty()) {
            Points.emplace_back(test, -10000);
            x0 = {test, -10000};
        } else {
            if (!Find(test, Points)) {
                Points.emplace_back(test, calculateAngle(x0.point, test));
            }
        }
    }
    if ((test = checkEdges(line.e2, pixel.e2)).x != -1 && test.y != -1) {
        if (Points.empty()) {
            Points.emplace_back(test, -10000);
            x0 = {test, -10000};
        } else {
            if (!Find(test, Points)) {
                Points.emplace_back(test, calculateAngle(x0.point, test));
            }
        }
    }
    if ((test = checkEdges(line.e2, pixel.e3)).x != -1 && test.y != -1) {
        if (Points.empty()) {
            Points.emplace_back(test, -10000);
            x0 = {test, -10000};
        } else {
            if (!Find(test, Points)) {
                Points.emplace_back(test, calculateAngle(x0.point, test));
            }
        }
    }
    if ((test = checkEdges(line.e2, pixel.e4)).x != -1 && test.y != -1) {
        if (Points.empty()) {
            Points.emplace_back(test, -10000);
            x0 = {test, -10000};
        } else {
            if (!Find(test, Points)) {
                Points.emplace_back(test, calculateAngle(x0.point, test));
            }
        }
    }
    if ((test = checkEdges(line.e3, pixel.e1)).x != -1 && test.y != -1) {
        if (Points.empty()) {
            Points.emplace_back(test, -10000);
            x0 = {test, -10000};
        } else {
            if (!Find(test, Points)) {
                Points.emplace_back(test, calculateAngle(x0.point, test));
            }
        }
    }
    if ((test = checkEdges(line.e3, pixel.e2)).x != -1 && test.y != -1) {
        if (Points.empty()) {
            Points.emplace_back(test, -10000);
            x0 = {test, -10000};
        } else {
            if (!Find(test, Points)) {
                Points.emplace_back(test, calculateAngle(x0.point, test));
            }
        }
    }
    if ((test = checkEdges(line.e3, pixel.e3)).x != -1 && test.y != -1) {
        if (Points.empty()) {
            Points.emplace_back(test, -10000);
            x0 = {test, -10000};
        } else {
            if (!Find(test, Points)) {
                Points.emplace_back(test, calculateAngle(x0.point, test));
            }
        }
    }
    if ((test = checkEdges(line.e3, pixel.e4)).x != -1 && test.y != -1) {
        if (Points.empty()) {
            Points.emplace_back(test, -10000);
            x0 = {test, -10000};
        } else {
            if (!Find(test, Points)) {
                Points.emplace_back(test, calculateAngle(x0.point, test));
            }
        }
    }
    if ((test = checkEdges(line.e4, pixel.e1)).x != -1 && test.y != -1) {
        if (Points.empty()) {
            Points.emplace_back(test, -10000);
            x0 = {test, -10000};
        } else {
            if (!Find(test, Points)) {
                Points.emplace_back(test, calculateAngle(x0.point, test));
            }
        }
    }
    if ((test = checkEdges(line.e4, pixel.e2)).x != -1 && test.y != -1) {
        if (Points.empty()) {
            Points.emplace_back(test, -10000);
            x0 = {test, -10000};
        } else {
            if (!Find(test, Points)) {
                Points.emplace_back(test, calculateAngle(x0.point, test));
            }
        }
    }
    if ((test = checkEdges(line.e4, pixel.e3)).x != -1 && test.y != -1) {
        if (Points.empty()) {
            Points.emplace_back(test, -10000);
            x0 = {test, -10000};
        } else {
            if (!Find(test, Points)) {
                Points.emplace_back(test, calculateAngle(x0.point, test));
            }
        }
    }
    if ((test = checkEdges(line.e4, pixel.e4)).x != -1 && test.y != -1) {
        if (Points.empty()) {
            Points.emplace_back(test, -10000);
            x0 = {test, -10000};
        } else {
            if (!Find(test, Points)) {
                Points.emplace_back(test, calculateAngle(x0.point, test));
            }
        }
    }

    //find inner points
    auto checkPoint = [](Rect Rectangle, Point A) -> bool {
        auto product = [](Point P, Edge E) -> double {
            return (E.b.x-E.a.x)*(P.y-E.a.y) - (E.b.y-E.a.y)*(P.x-E.a.x);
        };
        double p1 = product(A, Rectangle.e1);
        double p2 = product(A, Rectangle.e2);
        double p3 = product(A, Rectangle.e3);
        double p4 = product(A, Rectangle.e4);
        return (p1<EPS&&p2<EPS&&p3<EPS&&p4<EPS) || (p1>EPS&&p2>EPS&&p3>EPS&&p4>EPS);
    };
    if (checkPoint(line, pixel.e1.a)) {
        Point t = pixel.e1.a;
        if (Points.empty()) {
            Points.emplace_back(t, -10000);
            x0 = {t, -10000};
        } else {
            if (!Find(t, Points)) {
                Points.emplace_back(t, calculateAngle(x0.point, t));
            }
        }
    }
    if (checkPoint(line, pixel.e1.b))  {
        Point t = pixel.e1.b;
        if (Points.empty()) {
            Points.emplace_back(t, -10000);
            x0 = {t, -10000};
        } else {
            if (!Find(t, Points)) {
                Points.emplace_back(t, calculateAngle(x0.point, t));
            }
        }
    }
    if (checkPoint(line, pixel.e3.a))  {
        Point t = pixel.e3.a;
        if (Points.empty()) {
            Points.emplace_back(t, -10000);
            x0 = {t, -10000};
        } else {
            if (!Find(t, Points)) {
                Points.emplace_back(t, calculateAngle(x0.point, t));
            }
        }
    }
    if (checkPoint(line, pixel.e3.b))  {
        Point t = pixel.e3.b;
        if (Points.empty()) {
            Points.emplace_back(t, -10000);
            x0 = {t, -10000};
        } else {
            if (!Find(t, Points)) {
                Points.emplace_back(t, calculateAngle(x0.point, t));
            }
        }
    }
    if (checkPoint(pixel, line.e1.a)) {
        Point t = line.e1.a;
        if (Points.empty()) {
            Points.emplace_back(t, -10000);
            x0 = {t, -10000};
        } else {
            if (!Find(t, Points)) {
                Points.emplace_back(t, calculateAngle(x0.point, t));
            }
        }
    }
    if (checkPoint(pixel, line.e1.b)) {
        Point t = line.e1.b;
        if (Points.empty()) {
            Points.emplace_back(t, -10000);
            x0 = {t, -10000};
        } else {
            if (!Find(t, Points)) {
                Points.emplace_back(t, calculateAngle(x0.point, t));
            }
        }
    }
    if (checkPoint(pixel, line.e3.a)) {
        Point t = line.e3.a;
        if (Points.empty()) {
            Points.emplace_back(t, -10000);
            x0 = {t, -10000};
        } else {
            if (!Find(t, Points)) {
                Points.emplace_back(t, calculateAngle(x0.point, t));
            }
        }
    }
    if (checkPoint(pixel, line.e3.b)) {
        Point t = line.e3.b;
        if (Points.empty()) {
            Points.emplace_back(t, -10000);
            x0 = {t, -10000};
        } else {
            if (!Find(t, Points)) {
                Points.emplace_back(t, calculateAngle(x0.point, t));
            }
        }
    }

    double area = 0.0;
    auto calculateArea = [](Point a, Point b, Point c) -> double {
        return abs(0.5*((a.x-c.x)*(b.y-c.y) - (a.y-c.y)*(b.x-c.x)));
    };

    std::sort(Points.begin(), Points.end(), [](const PointAngle& lhs, const PointAngle& rhs) {
        return lhs.angle < rhs.angle;
    });

    if (Points.size() < 3)
        return 0;

    Point Sx0 = Points[0].point;
    Point Sx1 = Points[1].point;

    for (int i = 2; i < Points.size(); i++) {
        Point Sx2 = Points[i].point;
        area += calculateArea(Sx0, Sx1, Sx2);
        Sx1 = Sx2;
    }
    return area;
}