#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

using namespace cv;

// declare functions
void displayImage(QLabel *label, Mat img);
void laplacianPyr(const int layers, Mat pyr[], const Mat image);
void combinePyramids(const int layers, const Mat leftPyr[], const Mat rightPyr[], Mat outPyr[]);

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
    Mat combinedPyr[layers];

    // reconstructed image
    Mat reconstruction;

    // read in the images
    image1 = imread(image1Path, IMREAD_COLOR);
    image2 = imread(image2Path, IMREAD_COLOR);

    // generate laplacian pyramids
    laplacianPyr(layers, image1Pyr, image1);
    laplacianPyr(layers, image2Pyr, image2);

    // combine laplacian pyramids
    combinePyramids(layers, image1Pyr, image2Pyr, combinedPyr);

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

/*
 * Creates a horizontal gradient mask of type CV_32FC1
 */
void imageMask(Mat &mask, int rows, int cols, int start, int end) {

    if (start > end) {

        // If start > end, find mask with start and end swapped, then invert
        imageMask(mask, rows, cols, end, start);
        subtract(1, mask, mask);

    }
    else {

        // create array
        mask.create(rows, cols, CV_32FC1);

        // fill left and right
        rectangle(mask, Point(0, 0), Point(start, rows-1), 1.0, FILLED);
        rectangle(mask, Point(end, 0), Point(cols-1, rows-1), 0.0, FILLED);

        // generate gradient between start and end cols
        for (int col = start; col <= end; col++) {

            // linear gradient between start and end
            float value = (float)(end - col) / (end - start);
            mask.col(col).setTo(value);
        }

    }


}

void addMaskedLaplacian(const Mat left, const Mat right, const Mat leftMask, Mat &dst) {

    // assert left and right are same size
    assert(left.rows == right.rows);
    assert(left.cols == right.cols);
    assert(left.channels() == right.channels());
    assert(left.type() == right.type());
    assert(left.channels() == 3);
    assert(leftMask.type() == CV_32FC1);

    // create dst of same size as left and right
    dst.create(left.rows, left.cols, left.type());

    for (int col = 0; col < left.cols; col++) {
        for (int row = 0; row < left.rows; row++) {

            // mask values
            float leftMaskValue = leftMask.at<float>(row, col);
            float rightMaskValue = 1 - leftMaskValue;

            // colors of source images
            Vec3b leftColor = left.at<Vec3b>(row, col);
            Vec3b rightColor = right.at<Vec3b>(row, col);

            // reference for modifying dst image
            Vec3b & dstColor = dst.at<Vec3b>(row, col);

            // signed type
            if (left.depth() == CV_8S) {
                dstColor[0] = ((signed char)leftColor[0] * leftMaskValue + (signed char)rightColor[0] * rightMaskValue);
                dstColor[1] = ((signed char)leftColor[1] * leftMaskValue + (signed char)rightColor[1] * rightMaskValue);
                dstColor[2] = ((signed char)leftColor[2] * leftMaskValue + (signed char)rightColor[2] * rightMaskValue);
            }
            // unsigned type
            else if (left.depth() == CV_8U) {
                dstColor[0] = ((uint8_t)leftColor[0] * leftMaskValue + (uint8_t)rightColor[0] * rightMaskValue);
                dstColor[1] = ((uint8_t)leftColor[1] * leftMaskValue + (uint8_t)rightColor[1] * rightMaskValue);
                dstColor[2] = ((uint8_t)leftColor[2] * leftMaskValue + (uint8_t)rightColor[2] * rightMaskValue);
            }
        }
    }

}

/**
 * Combines two image pyramids (currently by splicing together in the middle, change later)
 */
void combinePyramids(const int layers, const Mat leftPyr[], const Mat rightPyr[], Mat combinedPyr[]) {

    Mat leftMask;

    imageMask(leftMask, 512, 512, 200, 312);

    for (int layer = 0; layer < layers; layer++) {

        // Combine layer
        addMaskedLaplacian(leftPyr[layer], rightPyr[layer], leftMask, combinedPyr[layer]);

        // Downsize mask to fit next layer
        pyrDown(leftMask, leftMask);

    }

}
