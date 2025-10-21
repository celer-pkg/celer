#include <iostream>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

int main() {
    std::cout << "=== FFmpeg C++ Test Program ===" << std::endl;
    
    // Display version information
    std::cout << "FFmpeg Version Information:" << std::endl;
    std::cout << "AVCodec version: " << avcodec_version() << std::endl;
    std::cout << "AVFormat version: " << avformat_version() << std::endl;
    std::cout << "AVUtil version: " << avutil_version() << std::endl;
    
    // Extract version components for better readability
    unsigned libavcodec_version = avcodec_version();
    std::cout << "AVCodec version (human readable): " 
              << ((libavcodec_version >> 16) & 0xFF) << "."  // Major
              << ((libavcodec_version >> 8) & 0xFF) << "."   // Minor
              << (libavcodec_version & 0xFF) << std::endl;   // Micro
    
    // Display configuration info
    std::cout << "Build configuration: " << avcodec_configuration() << std::endl;
    
    // Test listing available codecs
    std::cout << "\nAvailable Video Decoders:" << std::endl;
    const AVCodec* codec = nullptr;
    void* iterator = nullptr;
    
    int video_decoder_count = 0;
    while ((codec = av_codec_iterate(&iterator))) {
        if (av_codec_is_decoder(codec) && codec->type == AVMEDIA_TYPE_VIDEO) {
            std::cout << "  " << codec->name << " - " << codec->long_name << std::endl;
            video_decoder_count++;
        }
    }
    std::cout << "Total video decoders found: " << video_decoder_count << std::endl;
    
    // Test listing available formats
    std::cout << "\nAvailable Input Formats:" << std::endl;
    const AVInputFormat* input_format = nullptr;
    void* format_iterator = nullptr;
    
    int input_format_count = 0;
    while ((input_format = av_demuxer_iterate(&format_iterator))) {
        std::cout << "  " << input_format->name << " - " << input_format->long_name << std::endl;
        input_format_count++;
    }
    std::cout << "Total input formats found: " << input_format_count << std::endl;
    
    // Test basic memory allocation and cleanup
    std::cout << "\nTesting basic AVFrame allocation..." << std::endl;
    AVFrame* frame = av_frame_alloc();
    if (frame) {
        std::cout << "AVFrame allocation: SUCCESS" << std::endl;
        av_frame_free(&frame);
        std::cout << "AVFrame cleanup: SUCCESS" << std::endl;
    } else {
        std::cout << "AVFrame allocation: FAILED" << std::endl;
    }
    
    // Test AVPacket allocation
    std::cout << "Testing AVPacket allocation..." << std::endl;
    AVPacket* packet = av_packet_alloc();
    if (packet) {
        std::cout << "AVPacket allocation: SUCCESS" << std::endl;
        av_packet_free(&packet);
        std::cout << "AVPacket cleanup: SUCCESS" << std::endl;
    } else {
        std::cout << "AVPacket allocation: FAILED" << std::endl;
    }
    
    std::cout << "\n=== FFmpeg Test Completed Successfully ===" << std::endl;
    
    return 0;
}