//
// Created by 聂邦恒 on 2020/11/11.
//

#include "ffmpeg.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#ifdef __cplusplus
} // endof extern "C"
#endif

#include <utility>
#include "QString"
#include <QDebug>
#include <sys/stat.h>
#include <QTextCodec>


int ffmpeg::read(const QString &url, QString outFileName) {


    QByteArray ba = url.toUtf8();
    const char *ud = ba.data();
    qDebug() << "on download url:" << ud;
    qDebug() << "ffmpeg version:" << av_version_info();
    AVFormatContext *ic = nullptr;
    int ret = avformat_open_input(&ic, ud, nullptr, nullptr);
    qDebug() << "avformat open ret" << ret;
    if (ret == 0) {
        // 创建一个按时分秒的为文件名的文件夹
        char now[80];
        time_t rawtime;
        time(&rawtime);
        auto info = localtime(&rawtime);
        strftime(now, 80, "%Y-%m-%d", info);
        QString dirPath = "./";
        dirPath.append(now);
        qDebug() << "mkdir " << dirPath;
#if defined(Q_OS_WIN32)
        mkdir(dirPath.toUtf8());
#else
        mkdir(dirPath.toUtf8(), 0777);
#endif
        //查找码流信息
        if (avformat_find_stream_info(ic, nullptr) < 0) {
            qDebug("Couldn't find stream information.\n");
            return -1;
        }
        int videoindex = -1;
        int audioindex = -1;
        unsigned i = 0;
        for (i = 0; i < ic->nb_streams; i++) {
            if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                videoindex = i;
                break;
            }
            if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                audioindex = i;
                break;
            }
        }

        AVPacket *packet;
        packet = (AVPacket *) av_malloc(sizeof(AVPacket));
        FILE *fpSave;
        if (videoindex != -1) {
            outFileName += ".mp4";
        } else if (audioindex != -1) {
            outFileName += ".mp3";
        } else {
            qDebug("Didn't find a stream.\n");
            return -1;
        }

        outFileName = dirPath + "/" + outFileName;
        qDebug() << "open file" << outFileName;
#if defined(Q_OS_WIN32)
        QTextCodec* pCodec = QTextCodec::codecForName("gb2312");
        auto f = pCodec->fromUnicode(outFileName.toLocal8Bit());
#else
        auto f = outFileName.toLocal8Bit();
#endif
        fpSave = fopen(f, "wb");
        if (fpSave == nullptr) {
            return -1;
        }
        while (av_read_frame(ic, packet) >= 0) {
            fwrite(packet->data, 1, packet->size, fpSave);
            av_packet_unref(packet);
        }

        fclose(fpSave);
        av_free(ic);
        av_free(packet);
        return 0;
    }
    return -1;
}

ffmpeg::ffmpeg() {
    avformat_network_init();

}