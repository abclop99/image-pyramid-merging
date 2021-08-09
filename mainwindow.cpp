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

    // set the text edits to the path
    ui->leftFileText->setText(leftImagePath);
    ui->rightFileText->setText(rightImagePath);

    // Read the images
    Mat leftImage, rightImage;
    loadImage(leftImage, leftImagePath);
    loadImage(rightImage, rightImagePath);

    // Set the images
    leftPyr.setImage(leftImage);
    rightPyr.setImage(rightImage);

    // Layers
    leftPyr.setLayers(initialLayers);
    rightPyr.setLayers(initialLayers);

    // Combine them
    combineImages();

    // Display them
    displayImages();

    // Connect slider changing values to recombining the images
    connect(ui->startSlider, SIGNAL(valueChanged(int)), this, SLOT(combineImages()));
    connect(ui->endSlider, SIGNAL(valueChanged(int)), this, SLOT(combineImages()));

    // Connect file selection UI to functionality
    connect(ui->leftFileButton, SIGNAL(clicked()), this, SLOT(handleLeftFileButton()));
    connect(ui->rightFileButton, SIGNAL(clicked()), this, SLOT(handleRightFileButton()));
    connect(ui->leftFileSubmit, SIGNAL(clicked()), this, SLOT(submitLeftImage()));
    connect(ui->rightFileSubmit, SIGNAL(clicked()), this, SLOT(submitRightImage()));
    connect(ui->leftFileText, SIGNAL(returnPressed()), this, SLOT(submitLeftImage()));
    connect(ui->rightFileText, SIGNAL(returnPressed()), this, SLOT(submitRightImage()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::displayImages() {

    Size imSize = leftPyr.getSize();

    // calculate display size of images
    int width = imSize.width;
    int height = imSize.height;

    // if too small, make the image the largest possibls while
    // still fitting in the dimensions
    if (imSize.width < minDisplayDim.width &&
            imSize.width < minDisplayDim.width) {

        // If too small, make the image fit inside but barely
        if (imSize.width < minDisplayDim.width) {
            width = minDisplayDim.width;
            height = imSize.height * minDisplayDim.width / imSize.width;
        }

        if (imSize.height > minDisplayDim.height) {
            int w2 = imSize.width * minDisplayDim.height / imSize.height;
            if (width > w2) {
                width = w2;
                height = minDisplayDim.height;
            }
        }
    }
    else {

        // If too large, make the image fit inside
        if (imSize.width > maxDisplayDim.width) {
            width = maxDisplayDim.width;
            height = imSize.height * maxDisplayDim.width / imSize.width;
        }

        if (imSize.height > maxDisplayDim.height) {
            int w2 = imSize.width * maxDisplayDim.height / imSize.height;
            if (width > w2) {
                width = w2;
                height = maxDisplayDim.height;
            }
        }
    }

    Size displaySize = Size(width, height);

    // resize the UI components to fit the images if possible
    resizeUI(displaySize);

    displayImage(ui->leftImageLabel, leftPyr.getResizedImage(displaySize));
    displayImage(ui->rightImageLabel, rightPyr.getResizedImage(displaySize));
    displayImage(ui->reconstructionLabel, combinedPyr.getResizedImage(displaySize));
}

/**
 * Displays an opencv image in a QLabel.
 */
void MainWindow::displayImage(QLabel *label, Mat img) {
    Mat image;
    cv::cvtColor(img, image, CV_BGR2RGB);
    label->setPixmap(QPixmap::fromImage((QImage(image.data, image.cols, image.rows, image.step, QImage::Format_RGB888))));
}

/*
 * Creates a horizontal gradient mask of type CV_32FC1
 */
Mat MainWindow::imageMask(
        int rows, int cols,
        int startPercent, int endPercent
        ) {

    Mat mask;

    int start   = cols * startPercent / 100;
    int end     = cols * endPercent / 100;

    if (start > end) {

        // If start > end, find mask with start and end swapped, then invert
        mask = imageMask(rows, cols, endPercent, startPercent);
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

    return mask;

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

void MainWindow::resizeUI(Size imageDimensions) {
    int width = max(imageDimensions.width, minDisplayDim.width);
    int height = imageDimensions.height;

    int windowWidth = 4*displayGap + 3*width;
    int windowHeight = 3*displayGap + width + panelHeight;
    // resize window
    this->resize(windowWidth, windowHeight);
    this->setMinimumWidth(windowWidth);
    this->setMinimumHeight(windowHeight);
    this->setMaximumWidth(windowWidth);
    this->setMaximumHeight(windowHeight);

    // Image Display Labels
    ui->leftImageLabel->resize(width, height);
    ui->reconstructionLabel->resize(width, height);
    ui->rightImageLabel->resize(width, height);

    ui->leftImageLabel->move(displayGap, displayGap);
    ui->reconstructionLabel->move(2*displayGap + width, displayGap);
    ui->rightImageLabel->move(3*displayGap + 2*width, displayGap);

    // Resize slider panel
    ui->sliders->resize(width, panelHeight);
    ui->sliders->move(2*displayGap + width, 2 + displayGap + width);

    // resize other panels
    ui->leftFrame->resize(width, panelHeight);
    ui->rightFrame->resize(width, panelHeight);

    ui->leftFrame->move(displayGap, 2 + displayGap + width);
    ui->rightFrame->move(3*displayGap + 2*width, 2 + displayGap + width);
}
