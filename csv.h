//
// Created by 聂邦恒 on 2020/11/10.
//

#ifndef D_6AKC_COM_CSV_H
#define D_6AKC_COM_CSV_H

#include <QWidget>


class CSV : public QWidget {
public:
    static std::vector<QStringList> read();
};


#endif //D_6AKC_COM_CSV_H
