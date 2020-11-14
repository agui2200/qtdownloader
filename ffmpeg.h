//
// Created by 聂邦恒 on 2020/11/11.
//

#ifndef D_6AKC_COM_FFMPEG_H
#define D_6AKC_COM_FFMPEG_H


#include <QString>

class ffmpeg {

public:

    explicit ffmpeg();

    static int read(const QString &url, QString outFileName);
};


#endif //D_6AKC_COM_FFMPEG_H
