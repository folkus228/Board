#include <iostream>
#include <chrono>
#include <queue>
#include <unordered_map>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include <libswscale/swscale.h>   // sws_getContext, sws_scale
#include <libswresample/swresample.h> // swr_alloc_set_opts2, swr_convert
}

#include "../object/Component.h"
#include "TextureManager.h"

void PrintInfoFFMpeg() {
    std::cout << "FFmpeg version: " << av_version_info() << "\n";
    std::cout << "avformat: " << LIBAVFORMAT_VERSION_MAJOR << "\n";
    std::cout << "avcodec:  " << LIBAVCODEC_VERSION_MAJOR << "\n";
}

struct StreamData
{
    AVCodecContext* c_ctx;
    AVStream* stream;
    const AVCodec* codec;
    int stream_index;
};

class Video : public Component
{
public:
    Video(Object* obj) 
        : Component(obj) {
        _packet = av_packet_alloc();
        _frame = av_frame_alloc();
    }
    ~Video()
    {
        av_packet_free(&_packet);
        av_frame_free(&_frame);
    }
    void LoadVideo(const char* path) {
        if (avformat_open_input(&_f_ctx, path, nullptr, nullptr) < 0) {
            std::cerr << "Не удалось открыть файл: " << path << std::endl;
            return;
        }
        // Читаем инфу о потоках внутри mp4 (там обычно видео, аудио, субтитры)
        if (avformat_find_stream_info(_f_ctx, nullptr) < 0) {
            std::cerr << "Не удалось найти информацию о потоках" << std::endl;
            avformat_close_input(&_f_ctx);
            return;
        }
    }
    void GetCodecAndStream(StreamData& sd, AVMediaType type = AVMEDIA_TYPE_VIDEO)
    {
        // Функция сама найдет лучший видеопоток и вернет нужный для него декодер (кодек)
        sd.stream_index = av_find_best_stream(_f_ctx, type, -1, -1, &sd.codec, 0);
        if (sd.stream_index < 0) {
            std::cerr << "В этом MP4 нет видеопотока!" << std::endl;
        }
    }
    void InitCodecCTX(StreamData& sd)
    {
        sd.stream = _f_ctx->streams[sd.stream_index];
        sd.c_ctx = avcodec_alloc_context3(sd.codec);
        if (!sd.c_ctx) {
            std::cerr << "Не удалось выделить память под контекст декодера" << std::endl;
            return;
        }
        // Копируем параметры параметров из файла в наш декодер (разрешение, профиль и т.д.)
        avcodec_parameters_to_context(sd.c_ctx, sd.stream->codecpar);

        if (avcodec_open2(sd.c_ctx, sd.codec, nullptr) < 0) {
            std::cerr << "Failed to open codec" << std::endl;
            return;
        }
    }
    template<typename... SD>
    void SetFramePtr(double time, StreamData& sd, SD... sds)
    {
        int64_t target_pts = time / av_q2d(sd.stream->time_base);
        int seek_result = av_seek_frame(_f_ctx, sd.stream_index, target_pts, AVSEEK_FLAG_BACKWARD);
        avcodec_flush_buffers(sd.c_ctx);
        if (seek_result < 0) std::cerr << "Ошибка при поиске позиции (Seek error)\n";
        if constexpr (sizeof...(SD) > 0)
            SetFramePtr(time, sds...);
        else {
            for (auto& [idx, q] : _packet_queues) {
                while (!q.empty()) {
                    av_packet_free(&q.front());
                    q.pop();
                }
            }
        }
    }
    double GetVideoDuration() { return _f_ctx->duration / (double)AV_TIME_BASE; }
    bool NextFrame(StreamData& sd, AVFrame*& out_frame) {
        while (true) {
            // 1. Проверяем декодер. Если там есть кадр — отдаем его (работает и для обычного режима, и для flush)
            int ret = avcodec_receive_frame(sd.c_ctx, _frame);                           // читаем кадр
            if (ret == 0) { out_frame = _frame; return true; }                           // если кадр полностью собран
            if (ret == AVERROR_EOF || (ret < 0 && ret != AVERROR(EAGAIN))) return false; // если ошибка не свяана с запросом доп батча

            auto& q = _packet_queues[sd.stream_index]; // получили очередь
            if (!q.empty()) {
                AVPacket* pkt = q.front(); q.pop(); // берем пакет
                avcodec_send_packet(sd.c_ctx, pkt); // кладем в codec 
                av_packet_free(&pkt);               // уже в codec значит не нужен
                continue;                           // траим еще раз
            }

            // 3. Если очередь пуста, пытаемся прочитать новые пакеты из файла
            bool read_any_packet = false;

            while (av_read_frame(_f_ctx, _packet) >= 0) {
                read_any_packet = true;
                int idx = _packet->stream_index;

                if (idx == sd.stream_index) {
                    avcodec_send_packet(sd.c_ctx, _packet);
                    av_packet_unref(_packet);
                    break; // Выходим во внешний цикл, чтобы avcodec_receive_frame забрал кадр
                }
                else {
                    AVPacket* copy = av_packet_alloc();
                    av_packet_move_ref(copy, _packet);
                    _packet_queues[idx].push(copy);
                }
            }

            // av_read_frame не прочитал ни одного пакета
            if (!read_any_packet) avcodec_send_packet(sd.c_ctx, nullptr); // Передаем nullptr. Кодек понимает, что это финал, и подготавливает застрявшие кадры    
        }
    }
    bool SmartNextFrame(StreamData& sd, AVFrame*& out_frame, double time)
    {
        if (out_frame == nullptr)
            return NextFrame(sd, out_frame);
        else {
            double seconds = out_frame->pts * av_q2d(sd.stream->time_base);
            if (time > seconds)
                return NextFrame(sd, out_frame);
            else
                return true;
        }
    }
    void ReWriteNextAllFrameTestFicha(StreamData& sd, AVFrame*& out_frame, double time_sec)
    {
        double tick_rate = av_q2d(sd.stream->time_base);
        double sec = _frame->pts * tick_rate;
        while (time_sec > sec) {
            NextFrame(sd, out_frame);
            sec = _frame->pts * tick_rate;
        }
    }
    unsigned int CreateFrameTextureTestFicha(StreamData& sd, AVFrame*& frame)
    {
        // Видео: YUV420P → RGB через sws_scale
        SwsContext* sws_ctx = sws_getContext(
            frame->width, frame->height, (AVPixelFormat)frame->format,
            frame->width, frame->height, AV_PIX_FMT_RGB24,
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );

        std::vector<uint8_t> rgb_buffer(frame->width * frame->height * 3);
        uint8_t* dst[1] = { rgb_buffer.data() };
        int dst_stride[1] = { frame->width * 3 };
        sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height, dst, dst_stride);
        // rgb_buffer — готовый RGB, можно грузить в текстуру
        return TextureManager::LoadTexture(frame->height, frame->width, rgb_buffer.data(), GL_RGB);
    }
    void SetFrameTextureTestFicha(unsigned int texID, AVFrame*& frame)
    {
        // Видео: YUV420P → RGB через sws_scale
        SwsContext* sws_ctx = sws_getContext(
            frame->width, frame->height, (AVPixelFormat)frame->format,
            frame->width, frame->height, AV_PIX_FMT_RGB24,
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );

        std::vector<uint8_t> rgb_buffer(frame->width * frame->height * 3);
        uint8_t* dst[1] = { rgb_buffer.data() };
        int dst_stride[1] = { frame->width * 3 };
        sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height, dst, dst_stride);
        // rgb_buffer — готовый RGB, можно грузить в текстуру
        TextureManager::BindTexture(texID, 0);
        TextureManager::SetTexture(texID, frame->height, frame->width,
            rgb_buffer.data(), GL_RGB);
    }
private:
    std::unordered_map<int, std::queue<AVPacket*>> _packet_queues;
    AVFormatContext* _f_ctx = nullptr;
    AVCodecContext* _c_ctx;
    AVPacket* _packet;
    AVFrame* _frame;
};
