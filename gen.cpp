#include <iostream>
#include <vector>
#include <fstream>

#pragma pack(push, 1)
struct BMPHeader {
    // BITMAPFILEHEADER (14 bytes)
    uint16_t file_type = 0x4D42;     // 'BM'
    uint32_t file_size;              // Total file size
    uint16_t reserved1 = 0;
    uint16_t reserved2 = 0;
    uint32_t offset_data = 62;       // Pixel data offset

    // BITMAPINFOHEADER (40 bytes)
    uint32_t size = 40;              // Header size
    int32_t width;
    int32_t height;
    uint16_t planes = 1;
    uint16_t bit_count = 1;          // 1-bit
    uint32_t compression = 0;
    uint32_t size_image;             // Pixel data size
    int32_t x_pixels_per_meter = 2835;
    int32_t y_pixels_per_meter = 2835;
    uint32_t colors_used = 2;
    uint32_t colors_important = 2;
};
#pragma pack(pop)

void create_black_white_bmp(const std::string& filename, int width, int height) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "无法创建文件: " << filename << std::endl;
        return;
    }

    // 初始化文件头
    BMPHeader header;
    header.width = width;
    header.height = height;
    
    // 计算行字节数（4字节对齐）
    const int row_size = ((width + 31) / 32) * 4;
    header.size_image = row_size * height;
    header.file_size = sizeof(BMPHeader) + 8 + header.size_image; // 54 + 8 + pixel data

    // 写入文件头
    file.write(reinterpret_cast<char*>(&header), sizeof(header));
    
    // 写入调色板（0:黑, 1:白）
    uint32_t palette[2] = {0x00000000, 0x00FFFFFF};
    file.write(reinterpret_cast<char*>(palette), sizeof(palette));

    // 生成像素数据
    std::vector<uint8_t> row(row_size, 0);
    const int mid = width / 2;
    
    // BMP从下到上存储
    for (int y = height-1; y >= 0; --y) {
        std::fill(row.begin(), row.end(), 0);
        for (int x = 0; x < width; ++x) {
            // 左半黑(0)，右半白(1)
            if (x >= mid) {
                // 高位在前（左侧像素对应高bit）
                row[x/8] |= (0x80 >> (x%8));
            }
        }
        file.write(reinterpret_cast<char*>(row.data()), row_size);
    }
}

int main() {
    const int width = 500;
    const int height = 500;
    create_black_white_bmp("black_white.bmp", width, height);
    std::cout << "已生成500x500黑白分半BMP文件: black_white.bmp" << std::endl;
    return 0;
}