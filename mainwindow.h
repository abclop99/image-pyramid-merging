#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <opencv2/core/core.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void combineImages();

private:
    Ui::MainWindow *ui;

    static const int layers = 4;

    // Images
    cv::Mat image1, image2;

    // Image Pyramids
    cv::Mat image1Pyr[layers];
    cv::Mat image2Pyr[layers];
    cv::Mat combinedPyr[layers];

    // reconstructed image
    cv::Mat reconstruction;

    void displayImages();
};
#endif // MAINWINDOW_H
