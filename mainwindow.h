#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ui_mainwindow.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using namespace cv;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Width and height used for images
    const int imageWidth = 512, imageHeight = 512;

private slots:
    void combineImages();

    void handleLeftFileButton();
    void handleRightFileButton();
    void handleLeftFileSubmit();
    void handleRightFileSubmit();

private:
    Ui::MainWindow *ui;

    const QString imageFileFilter = tr("Images (*.png *.xpm *.jpg *.tiff);; All Files (*)");

    const QString emptyMsg = "";
    const QString emptyPathMsg  = tr("The path is empty.");
    const QString imageInvalidMsg   = tr("The image could not be read.");
    const QString unknownErrorMsg   = tr("An unknown error occurred");

    static const int layers = 6;

    // Images
    Mat leftImage, rightImage;

    // Image Pyramids
    Mat leftPyr[layers];
    Mat rightPyr[layers];
    Mat combinedPyr[layers];

    // reconstructed image
    Mat reconstruction;

    int loadImage(Mat &dst, QString path);

    void displayImage(QLabel *label, Mat img);
    void displayImages();

    // Pyramid related functions
    void laplacianPyr(const int layers, Mat pyr[], const cv::Mat image);
    void combinePyramids(const int layers, const Mat leftPyr[], const Mat rightPyr[], Mat combinedPyr[],
                         const int maskStartPercent=40, const int maskEndPercent=60);
    void reconstructImage(const int layers, const Mat pyr[], Mat &dst);

    void setLeftErrorMessage(QString msg);
    void setRightErrorMessage(QString msg);
};
#endif // MAINWINDOW_H
