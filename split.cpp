#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <filesystem>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

using namespace std;
namespace fs = filesystem;

#pragma pack(push, 1)
struct BMPHeader {
    uint16_t file_type = 0x4D42;
    uint32_t file_size;
    uint16_t reserved1 = 0;
    uint16_t reserved2 = 0;
    uint32_t offset_data = 62;

    uint32_t size = 40;
    int32_t width;
    int32_t height;
    uint16_t planes = 1;
    uint16_t bit_count = 1;
    uint32_t compression = 0;
    uint32_t size_image;
    int32_t x_pixels_per_meter = 2835;
    int32_t y_pixels_per_meter = 2835;
    uint32_t colors_used = 2;
    uint32_t colors_important = 2;

    // uint32_t red_mask   = 0x00000000;
    // uint32_t green_mask = 0x00000000;
    // uint32_t blue_mask  = 0x00000000;
    // uint32_t alpha_mask = 0x00000000;
    // uint32_t cs_type = 0x73524742;
    // uint32_t endpoints[9] = {0};
    // uint32_t gamma_red = 0;
    // uint32_t gamma_green = 0;
    // uint32_t gamma_blue = 0;
};
#pragma pack(pop)

void write_1bit_bmp(const string& filename, int width, int height, const vector<bool>& bits) {
    ofstream file(filename, ios::binary);
    if (!file) return;

    BMPHeader header;
    header.width = width;
    header.height = height;
    
    const int row_size = ((width + 31) / 32) * 4;
    header.size_image = row_size * height;
    header.file_size = sizeof(BMPHeader) + 8 + 40 + header.size_image;
    

    // Write BMP header
    file.write(reinterpret_cast<char*>(&header), sizeof(header));
    
    // Write color palette (0: black, 1: white)
    uint32_t palette[2] = {0x00000000, 0x00FFFFFF};
    file.write(reinterpret_cast<char*>(palette), sizeof(palette));

    // Write bitmap data
    vector<uint8_t> row(row_size, 0);
    for (int y = height-1; y >= 0; --y) {
        fill(row.begin(), row.end(), 0);
        for (int x = 0; x < width; ++x) {
            if (bits[y*width + x]) {
                row[x/8] |= (0x80 >> (x%8));
            }
        }
        file.write(reinterpret_cast<char*>(row.data()), row_size);
    }
}

void process_image(const string& input_path, int img_num, const string& output_dir) {
    int width, height, channels;
    unsigned char* img = stbi_load(input_path.c_str(), &width, &height, &channels, 1);
    if (!img) return;

    vector<float> quant_error(width * height, 0.0f);
    vector<vector<bool>> layers(5, vector<bool>(width * height));

    // Floyd-Steinberg dithering
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = y*width + x;
            float old_pixel = img[idx] + quant_error[idx];
            // float old_pixel = img[idx];

            float value = old_pixel / 51.0f;
            // int integer_part = floor(value);
            int integer_part = round(value);
            float fraction = value - integer_part;

            for (int k = 0; k < 5; ++k) {
                layers[k][idx] = (k < integer_part) || 
                                (fraction > static_cast<float>(rand()) / RAND_MAX);
                // layers[k][idx] = (k < integer_part);
            }

            float error = old_pixel - (integer_part + fraction) * 51;
            if (x < width-1)     quant_error[idx+1]      += error * 7/16.0f;
            if (y < height-1) {
                if (x > 0)       quant_error[idx+width-1] += error * 3/16.0f;
                                 quant_error[idx+width]   += error * 5/16.0f;
                if (x < width-1) quant_error[idx+width+1] += error * 1/16.0f;
            }
        }
    }

    // Save layers
    for (int i = 0; i < 5; ++i) {
        int output_num = (img_num-1)*6 + i + 1;
        string path = output_dir + "/" + to_string(output_num) + ".bmp";
        write_1bit_bmp(path, width, height, layers[i]);
    }

    // Save black image
    vector<bool> black(width * height, false);
    int output_num = (img_num-1)*6 + 6;
    string path = output_dir + "/" + to_string(output_num) + ".bmp";
    write_1bit_bmp(path, width, height, black);

    stbi_image_free(img);
}

int main() {
    string input_dir = "input/";
    string output_dir = "output/";
    fs::create_directories(output_dir);

    for (int i = 1; i <= 40; ++i) {
        string path = input_dir + to_string(i) + ".jpeg";
        if (!fs::exists(path)) continue;
        process_image(path, i, output_dir);
        cout << "Processed image: " << i << endl;
    }
    return 0;
}
