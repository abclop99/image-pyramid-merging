#include "mainwindow.h"
#include <QFile>

#include <iostream>

using namespace cv;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Merging 2 images using Laplacian Pyramids");

    const QString leftImagePath = ":/images/apple.jpg";
    const QString rightImagePath = ":/images/orange.jpg";

    ui->leftFileText->setText(leftImagePath);
    ui->rightFileText->setText(rightImagePath);

    loadImage(leftImage, leftImagePath);
    loadImage(rightImage, rightImagePath);

    // generate laplacian pyramids
    laplacianPyr(layers, leftPyr, leftImage);
    laplacianPyr(layers, rightPyr, rightImage);

    // Combine the images
    combineImages();

    // Connect slider changing values to recombining the images
    connect(ui->startSlider, SIGNAL(valueChanged(int)), this, SLOT(combineImages()));
    connect(ui->endSlider, SIGNAL(valueChanged(int)), this, SLOT(combineImages()));

    // Connect file selection UI to functionality
    connect(ui->leftFileButton, SIGNAL(clicked()), this, SLOT(handleLeftFileButton()));
    connect(ui->rightFileButton, SIGNAL(clicked()), this, SLOT(handleRightFileButton()));
    connect(ui->leftFileSubmit, SIGNAL(clicked()), this, SLOT(handleLeftFileSubmit()));
    connect(ui->rightFileSubmit, SIGNAL(clicked()), this, SLOT(handleRightFileSubmit()));
    connect(ui->leftFileText, SIGNAL(returnPressed()), this, SLOT(handleLeftFileSubmit()));
    connect(ui->rightFileText, SIGNAL(returnPressed()), this, SLOT(handleRightFileSubmit()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * Combines the pyramids, then reconstructs an image
 */
void MainWindow::combineImages() {

    combinePyramids(layers, leftPyr, rightPyr, combinedPyr, ui->startSlider->value(), ui->endSlider->value());

    reconstructImage(layers, combinedPyr, reconstruction);

    displayImages();

}

/**
 * Displays the images in the UI
 */
void MainWindow::displayImages() {
    displayImage(ui->leftImageLabel, leftImage);
    displayImage(ui->rightImageLabel, rightImage);
    displayImage(ui->reconstructionLabel, reconstruction);
}

/**
 * Displays an opencv image in a QLabel.
 */
void MainWindow::displayImage(QLabel *label, Mat img) {
    Mat image;
    cv::cvtColor(img, image, CV_BGR2RGB);
    label->setPixmap(QPixmap::fromImage((QImage(image.data, image.cols, image.rows, image.step, QImage::Format_RGB888))));
}

/**
 * Generate the laplacian pyramid for image with layer layers
 * and put in pyr
 */
void MainWindow::laplacianPyr(const int layers, Mat pyr[], const Mat image) {

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
        for (int col = start; col < end; col++) {

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
void MainWindow::combinePyramids(const int layers, const Mat leftPyr[], const Mat rightPyr[], Mat combinedPyr[],
                     const int maskStartPercent, const int maskEndPercent) {

    Mat leftMask;

    int maskStart   = leftPyr[0].cols * maskStartPercent / 100;
    int maskEnd     = leftPyr[0].cols * maskEndPercent / 100;

    imageMask(leftMask, leftPyr[0].rows, leftPyr[0].cols, maskStart, maskEnd);

    for (int layer = 0; layer < layers; layer++) {

        // Combine layer
        addMaskedLaplacian(leftPyr[layer], rightPyr[layer], leftMask, combinedPyr[layer]);

        // Downsize mask to fit next layer
        pyrDown(leftMask, leftMask);

    }

}

/**
 * Reconstructs an image from a Laplacian pyramid.
 */
void MainWindow::reconstructImage(const int layers, const Mat pyr[], Mat &dst) {

    // Start with smallest image (should be unsigned)
    dst = pyr[layers-1].clone();

    // From second smallest to largest images in pyramid
    for (int layer = layers - 2; layer >= 0; layer--) {
        //Upscale reconstruction and add previous layer
        pyrUp(dst, dst);
        add(dst, pyr[layer], dst, noArray(), dst.type());
    }

}

