#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

// declare functions
void displayImage(QLabel *label, Mat img);
void laplacianPyr(const int layers, Mat pyr[], const Mat image);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Merging 2 images using Laplacian Pyramids");

    // total layers, including the original size
    const int layers = 4;

    const std::string image1Path = "C:/Users/abclo/Documents/Projects/image-pyramids/apple.jpg";
    const std::string image2Path = "C:/Users/abclo/Documents/Projects/image-pyramids/orange.jpg";

    // Images
    Mat image1;
    Mat image2;

    // image pyramids
    Mat image1Pyr[layers];
    Mat image2Pyr[layers];

    // reconstructed image
    Mat reconstruction;

    // read in the images
    image1 = imread(image1Path, IMREAD_COLOR);
    image2 = imread(image2Path, IMREAD_COLOR);

    // generate laplacian pyramids
    laplacianPyr(layers, image1Pyr, image1);
    laplacianPyr(layers, image2Pyr, image2);

    // combine laplacian pyramids

    // reconstruct image

    // display images
    displayImage(ui->image1Label, image1);
    displayImage(ui->image2Label, image2);

}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * Displays an opencv image in a QLabel.
 */
void displayImage(QLabel *label, Mat img) {
    Mat image;
    cv::cvtColor(img, image, CV_BGR2RGB);
    label->setPixmap(QPixmap::fromImage((QImage(image.data, image.cols, image.rows, image.step, QImage::Format_RGB888))));
}

/**
 * Generate the laplacian pyramid for image with layer layers
 * and put in pyr
 */
void laplacianPyr(const int layers, Mat pyr[], const Mat image) {

    Mat upscaledImage;
    Mat tempLaplacian;

    // layer 0 = original
    pyr[0] = image.clone();

    // loop through other layers
    for (int layer = 1; layer < layers; layer++) {

        // gaussian downsize
        pyrDown(pyr[layer-1], pyr[layer]);

        // upscale
        pyrUp(pyr[layer], upscaledImage);

        // find difference, must be signed ddepth
        subtract(pyr[layer-1], upscaledImage, tempLaplacian, noArray(), CV_8S);

        // copy into pyramid
        pyr[layer-1] = tempLaplacian.clone();

    }

}

