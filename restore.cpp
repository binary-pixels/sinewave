#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "stb_image_write.h"

namespace fs = std::filesystem;

#pragma pack(push, 1)
struct BMPHeader {
    uint16_t signature = 0x4D42;
    uint32_t file_size;
    uint16_t reserved1 = 0;
    uint16_t reserved2 = 0;
    uint32_t data_offset = 62;

    uint32_t header_size = 40;
    int32_t width;
    int32_t height;
    uint16_t planes = 1;
    uint16_t bits_per_pixel = 1;
    uint32_t compression = 0;
    uint32_t image_size;
    int32_t x_resolution = 2835;
    int32_t y_resolution = 2835;
    uint32_t colors_used = 2;
    uint32_t colors_important = 2;
};
#pragma pack(pop)

struct ImageData {
    int width;
    int height;
    std::vector<bool> pixels;
};

ImageData read_1bit_bmp(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("无法打开文件: " + path);

    BMPHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(BMPHeader));
    
    if (header.bits_per_pixel != 1)
        throw std::runtime_error("非1位BMP文件: " + path);

    // 读取调色板（2个颜色项）
    uint32_t palette[2];
    file.seekg(sizeof(BMPHeader)); // 跳过文件头
    // file.seekg(sizeof(BMPHeader)); // 跳过文件头
    file.read(reinterpret_cast<char*>(palette), 8);

    // 计算行字节数（4字节对齐）
    const int row_size = ((header.width + 31) / 32) * 4;
    const int data_size = row_size * header.height;
    
    // 读取位图数据
    std::vector<uint8_t> buffer(data_size);
    file.seekg(header.data_offset);
    file.read(reinterpret_cast<char*>(buffer.data()), data_size);

    // 转换为像素数据
    ImageData result;
    result.width = header.width;
    result.height = header.height;
    result.pixels.resize(header.width * header.height);

    for (int y = 0; y < header.height; ++y) {
        int actual_y = header.height - 1 -y;
        const uint8_t* r_start = &buffer[actual_y*row_size];

        for (int x = 0; x < header.width; ++x) {
            const int byte_idx = x / 8;
            const int bit_idx = 7 - (x % 8);
            const uint8_t byte = r_start[byte_idx];
            //const uint8_t byte = buffer[actual_y * row_size + byte_idx];
            result.pixels[y * header.width + x] = (byte >> bit_idx) & 1;
        }
    }

    return result;
}

void restore_images(const std::string& input_dir, const std::string& output_dir) {
    fs::create_directories(output_dir);

    for (int img_num = 1; img_num <=40 ; ++img_num) {
        std::vector<ImageData> layers;
        
        // 读取5个有效层
        for (int layer = 0; layer <= 5; ++layer) {
            const int file_num = (img_num - 1) * 6 + layer + 1;
            const std::string path = input_dir + "/" + std::to_string(file_num) + ".bmp";
                layers.push_back(read_1bit_bmp(path));
        }

        // 生成灰度图像
        std::vector<uint8_t> output(layers[0].width * layers[0].height);
        for (size_t i = 0; i < output.size(); ++i) {
            int sum = 0;
            for (const auto& layer : layers)
                sum += layer.pixels[i];
            output[i] = static_cast<uint8_t>(sum * 51); // 51 = 255/5
        }

        // 保存8位BMP
        const std::string out_path = output_dir + "/" + std::to_string(img_num) + ".bmp";
        stbi_write_bmp(out_path.c_str(), 
                      layers[0].width, 
                      layers[0].height, 
                      1, // 通道数（灰度）
                      output.data());
        
    }
}

int main() {
    try {
        restore_images("output", "restored");
        std::cout << "图像恢复完成！共处理40组240张输入文件" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
