#include <iostream>
#include <chrono>
#include <queue>
#include <unordered_map>
#include <algorithm>

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
    StreamData() 
        : _thisFrame(av_frame_alloc()), 
        _nextFrame(av_frame_alloc()) {}
    AVCodecContext* c_ctx;
    AVStream* stream;
    const AVCodec* codec;
    AVFrame* _thisFrame;
    AVFrame* _nextFrame;
    int stream_index;
};

class Video : public Component
{
public:
    Video(Object* obj)
        : Component(obj) {
        _packet = av_packet_alloc();
    }
    ~Video()
    {
        av_packet_free(&_packet);
        for (auto& sd : _streams) {
            av_frame_free(&sd._thisFrame);
            av_frame_free(&sd._nextFrame);
        }
    }
    inline void Start() override
    {
        for (auto& sd : _streams) {
            NextFrame(sd);
            NextFrame(sd);
        }

        AVFrame* frame = _streams[0]._thisFrame;
        _sws_ctx = sws_getContext(
            frame->width, frame->height, (AVPixelFormat)frame->format,
            frame->width, frame->height, AV_PIX_FMT_RGB24,
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );

        texID = CreateFrameTextureTestFicha(_streams[0]);
        std::string ourTexture = "ourTexture";
        GetObject()->GetComponent<Material>()->SetTextureValue(ourTexture, texID);
    }
    void LoadVideo(const char* path, size_t streamsCount = 1) {

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
        _streams.resize(streamsCount);
    }
    // First stream only AVMEDIA_TYPE_VIDEO
    template<typename... AVMT>
    inline void InitStreams(AVMT... types)
    {
        _InitStreams(0, types...);
    }
    void SetFramePtr(double time)
    {
        this->time = time;
        for (auto& sd : _streams) {
            int64_t target_pts = time / av_q2d(sd.stream->time_base);
            av_seek_frame(_f_ctx, sd.stream_index, target_pts, AVSEEK_FLAG_BACKWARD);
            avcodec_flush_buffers(sd.c_ctx);
        }
        for (auto& [idx, q] : _packet_queues) {
            while (!q.empty()) { av_packet_free(&q.front()); q.pop(); }
        }
        // переинициализируем буферы кадров
        for (auto& sd : _streams) {
            NextFrame(sd);
            NextFrame(sd);
        }
        SetFrameTextureTestFicha(_streams[0]._thisFrame);
    }
    bool NextFrame(StreamData& sd) {
        std::swap(sd._thisFrame, sd._nextFrame); // меняем местами
        while (true) {
            // 1. Проверяем декодер. Если там есть кадр — отдаем его (работает и для обычного режима, и для flush)
            int ret = avcodec_receive_frame(sd.c_ctx, sd._nextFrame);                           // читаем кадр
            if (ret == 0) { return true; }                           // если кадр полностью собран
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
    void Update() override
    {
        if (isPausa)
            return;
        if (time > GetVideoDuration())
            return;
        time += Time::GetDeltaTime();
        for (auto& sd : _streams) {
            double nextTime = sd._nextFrame->pts * av_q2d(sd.stream->time_base);
            if (nextTime < time)
            {
                NextFrame(sd);
                nextTime = sd._nextFrame->pts * av_q2d(sd.stream->time_base);
                if (time > nextTime)
                    time = sd._thisFrame->pts * av_q2d(sd.stream->time_base);
            }
        }
        // processing current frame
        SetFrameTextureTestFicha(_streams[0]._thisFrame);
    }
    unsigned int CreateFrameTextureTestFicha(StreamData& sd)
    {
        std::vector<uint8_t> rgb_buffer(sd._thisFrame->width * sd._thisFrame->height * 3);
        uint8_t* dst[1] = { rgb_buffer.data() };
        int dst_stride[1] = { sd._thisFrame->width * 3 };
        sws_scale(_sws_ctx, sd._thisFrame->data, sd._thisFrame->linesize, 0, sd._thisFrame->height, dst, dst_stride);
        // rgb_buffer — готовый RGB, можно грузить в текстуру
        return TextureManager::LoadTexture(sd._thisFrame->height, sd._thisFrame->width, rgb_buffer.data(), GL_RGB);
    }
    void SetFrameTextureTestFicha(AVFrame*& frame)
    {
        std::vector<uint8_t> rgb_buffer(frame->width * frame->height * 3);
        uint8_t* dst[1] = { rgb_buffer.data() };
        int dst_stride[1] = { frame->width * 3 };
        sws_scale(_sws_ctx, frame->data, frame->linesize, 0, frame->height, dst, dst_stride);
        // rgb_buffer — готовый RGB, можно грузить в текстуру
        TextureManager::BindTexture(texID, 0);
        TextureManager::SetTexture(texID, frame->height, frame->width,
            rgb_buffer.data(), GL_RGB);
    }
    double GetVideoDuration() { return _f_ctx->duration / (double)AV_TIME_BASE; }
    void Pausa() { isPausa = !isPausa; }
    void Pausa(bool pausa) { isPausa = pausa; }
    bool IsPausa() { return isPausa; }
private:
    template<typename... AVMT>
    void _InitStreams(int idx, AVMediaType type, AVMT... types)
    {
        _streams[idx].stream_index = av_find_best_stream(_f_ctx, type, -1, -1, &_streams[idx].codec, 0);
        if (_streams[idx].stream_index < 0) {
            std::cerr << "This file not have a stream!" << std::endl;
        }
        _streams[idx].stream = _f_ctx->streams[_streams[idx].stream_index];
        _streams[idx].c_ctx = avcodec_alloc_context3(_streams[idx].codec);
        if (!_streams[idx].c_ctx) {
            std::cerr << "Faile to allocate memora for decoder" << std::endl;
            return;
        }
        // Копируем параметры параметров из файла в наш декодер (разрешение, профиль и т.д.)
        avcodec_parameters_to_context(_streams[idx].c_ctx, _streams[idx].stream->codecpar);

        if (avcodec_open2(_streams[idx].c_ctx, _streams[idx].codec, nullptr) < 0) {
            std::cerr << "Failed to open codec" << std::endl;
            return;
        }
        if constexpr (sizeof...(AVMT) > 0)
            _InitStreams(idx + 1, types...);
    }

    std::unordered_map<int, std::queue<AVPacket*>> _packet_queues;
    std::vector<StreamData> _streams;
    AVFormatContext* _f_ctx = nullptr;
    AVPacket* _packet;
    SwsContext* _sws_ctx;
    double time = 0;
    unsigned int texID;
    bool isPausa = false;
};
