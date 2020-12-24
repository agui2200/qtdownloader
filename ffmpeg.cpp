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
#include <libavutil/timestamp.h>
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
    av_dump_format(ic, 0, ud, 0);
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
                qDebug() << "find video codec_type." << ic->streams[i]->codecpar->codec_type;
            }
            if (ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                audioindex = i;
                qDebug() << "find audio codec_type." << ic->streams[i]->codecpar->codec_type;
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
        for (;;) {
            int ret = av_read_frame(ic, packet);
            if (ret != 0) {
                break;
            }
            if (packet->stream_index == videoindex) {
                fwrite(packet->data, 1, packet->size, fpSave);
            }
            av_packet_unref(packet);
        }


        fclose(fpSave);
        av_free(ic);
        av_free(packet);
        return 0;
    }
    return -1;
}

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag) {
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           tag,
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->stream_index);
}

int ffmpeg::read2(const QString &url, QString outFileName) {
    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;
    const char *in_filename, *out_filename;
    int ret, i;
    int stream_index = 0;
    int *stream_mapping = NULL;
    int stream_mapping_size = 0;
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
    outFileName = dirPath + "/" + outFileName + ".mp4";
#if defined(Q_OS_WIN32)
    QTextCodec* pCodec = QTextCodec::codecForName("gb2312");
        auto f = pCodec->fromUnicode(outFileName.toLocal8Bit());
#else
    auto f = outFileName.toLocal8Bit();
#endif
    qDebug() << "outfile " << outFileName;


    in_filename = url.toLocal8Bit();
    out_filename = f;

    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        fprintf(stderr, "Could not open input file '%s'", in_filename);
        goto end;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
        goto end;
    }

    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if (!ofmt_ctx) {
        fprintf(stderr, "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    stream_mapping_size = ifmt_ctx->nb_streams;
    stream_mapping = static_cast<int *>(av_mallocz_array(stream_mapping_size, sizeof(*stream_mapping)));
    if (!stream_mapping) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    ofmt = ofmt_ctx->oformat;

    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *out_stream;
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;

        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_mapping[i] = -1;
            continue;
        }

        stream_mapping[i] = stream_index++;

        out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            fprintf(stderr, "Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy codec parameters\n");
            goto end;
        }
        out_stream->codecpar->codec_tag = 0;
    }
    av_dump_format(ofmt_ctx, 0, out_filename, 1);

    if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            fprintf(stderr, "Could not open output file '%s'", out_filename);
            goto end;
        }
    }

    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file\n");
        goto end;
    }

    while (1) {
        AVStream *in_stream, *out_stream;

        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
            break;

        in_stream = ifmt_ctx->streams[pkt.stream_index];
        if (pkt.stream_index >= stream_mapping_size ||
            stream_mapping[pkt.stream_index] < 0) {
            av_packet_unref(&pkt);
            continue;
        }

        pkt.stream_index = stream_mapping[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];
//        log_packet(ifmt_ctx, &pkt, "in");

        /* copy packet */
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
                                   static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
                                   static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
//        log_packet(ofmt_ctx, &pkt, "out");

        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if (ret < 0) {
            fprintf(stderr, "Error muxing packet\n");
//            break;
        }
        av_packet_unref(&pkt);
    }

    av_write_trailer(ofmt_ctx);
    end:

    avformat_close_input(&ifmt_ctx);

    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);

    av_freep(&stream_mapping);

    if (ret < 0 && ret != AVERROR_EOF) {
        fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        return 1;
    }

    return 0;

}

ffmpeg::ffmpeg() {
    avformat_network_init();

}