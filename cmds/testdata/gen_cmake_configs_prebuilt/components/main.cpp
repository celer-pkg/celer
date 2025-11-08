#include <iostream>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

int main() {
    std::cout << "========== FFmpeg 5.x Verification Demo (Windows) ==========\n";

    std::cout << "libavutil     version: " << avutil_version() << "\n";
    std::cout << "libavcodec    version: " << avcodec_version() << "\n";
    std::cout << "libavformat   version: " << avformat_version() << "\n";
    std::cout << "libavfilter   version: " << avfilter_version() << "\n";
    std::cout << "libswscale    version: " << swscale_version() << "\n";
    std::cout << "libswresample version: " << swresample_version() << "\n";

    // 查找 H264 解码器
    const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        std::cerr << "❌ Failed to find H.264 decoder!" << std::endl;
        return 1;
    }

    std::cout << "✅ Found decoder: "
              << (codec->long_name ? codec->long_name : codec->name)
              << std::endl;

    // 统计解码器数量（兼容旧接口）
    int decoderCount = 0;

#if LIBAVCODEC_VERSION_MAJOR >= 58
    void *iter = nullptr;
    while ((codec = av_codec_iterate(&iter))) {
        if (av_codec_is_decoder(codec))
            decoderCount++;
    }
#else
    for (codec = av_codec_next(nullptr); codec; codec = av_codec_next(codec)) {
        if (av_codec_is_decoder(codec))
            decoderCount++;
    }
#endif

    std::cout << "Total available decoders: " << decoderCount << std::endl;
    std::cout << "🎉 FFmpeg 5.x environment test passed successfully!" << std::endl;
    std::cout << "============================================================\n";
    return 0;
}
