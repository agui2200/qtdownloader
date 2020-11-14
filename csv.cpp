//
// Created by 聂邦恒 on 2020/11/10.
//

#include "csv.h"
#include <qfile.h>
#include <qtextstream.h>
#include <QDir>
#include <QFileDialog>
#include <QtWidgets>
#include <QList>


std::vector<QStringList> CSV::read() {
    std::vector<QStringList> csvlines;
    QString fileName = QFileDialog::getOpenFileName(nullptr, tr("Open File"), "", tr("Excel(*.csv)"));
    if (fileName == "")
        return csvlines;

    QDir dir = QDir::current();
    QFile file(dir.filePath(fileName));
    if (!file.open(QIODevice::ReadOnly)) {
        return csvlines;
    }
    auto *out = new QTextStream(&file);//文本流
    QStringList tempOption = out->readAll().split("\n");//每行以\n区分
    for (int i = 0; i < tempOption.count(); i++) {
        QStringList tempbar = tempOption.at(i).split(",");//一行中的单元格以，区分
        csvlines.push_back(tempbar);
    }
    file.close();//操作完成后记得关闭文件
    return csvlines;

}