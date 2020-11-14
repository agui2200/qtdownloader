#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include "csv.h"
#include "mainwindow.h"
#include <QTextCodec>

using namespace std;


int main(int argc, char *argv[]) {
    QApplication a(argc, argv); //定义并创建应用程序
    // 解决读取文件乱码问题
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);
    QMessageBox messageBox;
    messageBox.setText("请选择从网站下载的csv文件");
    messageBox.exec();
    vector<QStringList> lines = CSV::read();
    if (lines.empty()) {
        messageBox.setText("文件读取错误。请重试");
        messageBox.exec();
        return -1;
    } else {
        // 打开一个窗口了
        MainWindow w(nullptr, lines);
        w.setFixedSize(w.width(), w.height());
        w.setWindowTitle("下载任务列表");

        w.show();
        return QApplication::exec(); //应用程序运行
    }
}


