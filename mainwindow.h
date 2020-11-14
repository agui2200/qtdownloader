#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include <QObjectUserData>
#include <QSortFilterProxyModel>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {

public:
    explicit MainWindow(QWidget *parent = nullptr, const std::vector<QStringList> &taskInfo = {});


    ~MainWindow() override;

private slots:

    void Download();


private:
    Ui::MainWindow *ui{};
};

#endif // MAINWINDOW_H