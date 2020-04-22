#ifndef LAB_2_PNMIMAGE_H
#define LAB_2_PNMIMAGE_H

#include <vector>
#include <cstdint>
#include <iostream>
#include <fstream>

using byte = unsigned char;

class PNMImage {
private:
    struct Point {
        double x;
        double y;
    };
    std::vector<byte> Buffer;
    std::vector<byte> ImageData;
    uint64_t Size, Width, Height, ColourDepth;
    uint8_t Type;

    void drawPoint(int, int, double, byte);
    void drawPoint(int, int, double, byte, double);
public:

    static std::vector<byte> ReadBinary(const char*, uint64_t);

    static void WriteBinary(const char*, const std::vector<byte>&);

    explicit PNMImage(const char*);

    void Export(const char*);

    void Invert();

    void Mirror(int);

    void Rotate(int direction);

    bool isGrey();

    bool isColor();

    void drawLine(Point start, Point end, byte color, double thickness = 1.0, double gamma = 0);
    void drawLine(double x0, double y0, double x1, double y1, byte color, double thickness, double gamma = 0);
};


#endif
