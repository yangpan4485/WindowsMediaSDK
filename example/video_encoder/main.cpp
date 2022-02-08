#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include "video_encoder.h"
#include "video_encoder_factory.h"
#include "video_frame.h"

int main(void) {
    std::string filename = "../../../480x272_yuv420p.yuv";
    std::string output_name = "../../../\\output.h264";
    std::ofstream fout(output_name, std::ios::binary | std::ios::out);
    std::ifstream fin(filename, std::ios::binary | std::ios::out);
    if (!fin || !fin.is_open()) {
        std::cout << "file open failed" << std::endl;
        getchar();
        return 0;
    }
    uint32_t width = 480;
    uint32_t height = 272;
    // kEncodeTypeX264, kEncodeTypeX265
    std::shared_ptr<VideoEncoder> video_encoder =
        VideoEnocderFcatory::Instance().CreateEncoder(kEncodeTypeFFmpeg);
    video_encoder->SetOutputSize(480, 272);
    video_encoder->RegisterEncodeCalback([&](uint8_t* code_data, uint32_t code_len) {
        std::cout << "len: " << code_len << std::endl;
        fout.write((char*)code_data, code_len);
    });
    uint32_t len = width * height * 3 / 2;
    uint8_t* data = new uint8_t[len];
    std::shared_ptr<VideoFrame> frame = std::make_shared<VideoFrame>(480, 272, kFrameTypeI420, true);
    while (!fin.eof()) {
        fin.read((char*)frame->GetData(), len);
        video_encoder->EncodeFrame(frame, false);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    delete[] data;
    fin.close();
    std::cout << "end" << std::endl;
    getchar();
    fout.close();
    return 0;
}