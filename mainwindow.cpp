#include "mainwindow.h"
#include <QDebug>
#include <QStandardItemModel>
#include <QTableWidgetItem>
#include "ui_mainwindow.h"
#include "ffmpeg.h"
#include <QtConcurrent/QtConcurrent>

MainWindow::MainWindow(QWidget *parent, const std::vector<QStringList> &taskInfo) :
        QMainWindow(parent),
        ui(new Ui::MainWindow) {
    ui->setupUi(this);
    auto model = new QStandardItemModel(this);

    model->setHorizontalHeaderItem(0, new QStandardItem("课程名称"));
    model->setHorizontalHeaderItem(1, new QStandardItem("下载地址"));
    model->setHorizontalHeaderItem(2, new QStandardItem("下载状态"));

    ui->tableView->setModel(model);
    ui->tableView->setColumnWidth(0, 300);
    ui->tableView->setColumnWidth(1, 100);
    ui->tableView->setColumnWidth(2, 100);
    for (auto i:taskInfo) {
        if (i.count() >= 2) {
            qDebug() << "on item" << i[0] << i[1];
            QList<QStandardItem *> row;
            auto *item = new QStandardItem();
            auto *item2 = new QStandardItem();
            auto *item3 = new QStandardItem();
            item->setText(i[0]);
            row.append(item);
            item2->setText(i[1]);
            row.append(item2);
            // 默认状态
            item3->setText("未下载");
            row.append(item3);
            model->appendRow(row);
        }

    }
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::Download);
}

void ffmpegReadLists(QAbstractItemModel *itemModel) {
    for (int i = 0; i < itemModel->rowCount(); ++i) {
        auto idx = itemModel->index(i, 1);
        auto urlItem = itemModel->itemData(idx);
        // 修改状态
        auto statusidx = itemModel->index(i, 2);
        qDebug() << "修改任务状态" << itemModel->itemData(statusidx);
        itemModel->setData(statusidx, "下载中");
        // 调用ffmpeg
        // 创建一个文件
        auto urlidx = itemModel->index(i, 0);
        auto output = itemModel->itemData(urlidx);
        // 创建一个新线程
        auto ret = ffmpeg::read(reinterpret_cast<const QString &>(urlItem[0]), reinterpret_cast<QString &&>(output[0]));
        if (ret == -1) {
            itemModel->setData(statusidx, "下载失败");
        } else {
            itemModel->setData(statusidx, "下载完成");
        }

    }
}

void MainWindow::Download() {
    qDebug() << "下载数据" << this->ui->tableView;
    QAbstractItemModel *m = this->ui->tableView->model();
    QtConcurrent::run(ffmpegReadLists, m);

};

MainWindow::~MainWindow() {
    delete ui;
}

