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

    // status bar
    ui->statusbar->showMessage(
                "Layers Used: " + QString::number(leftPyr.getLayers())
                + "\t Image size: " + QString::number(leftPyr.getWidth())
                + " x " + QString::number(leftPyr.getHeight())
                );
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
        int width, int height,
        int startPercent, int endPercent
        ) {

    Mat mask;

    int start   = width * startPercent / 100;
    int end     = width * endPercent / 100;

    if (start > end) {

        // If start > end, find mask with start and end swapped, then invert
        mask = imageMask(width, height, endPercent, startPercent);
        subtract(1, mask, mask);

    }
    else {

        // create array
        mask.create(height, width, CV_32FC1);

        // fill left and right
        rectangle(mask, Point(0, 0), Point(start, height-1), 1.0, FILLED);
        rectangle(mask, Point(end, 0), Point(width-1, height-1), 0.0, FILLED);

        // generate gradient between start and end cols
        for (int col = start; col < end; col++) {

            // linear gradient between start and end
            float value = (float)(end - col) / (end - start);
            mask.col(col).setTo(value);
        }

    }

    return mask;

}

void MainWindow::resizeUI(Size imageDimensions) {
    int width = max(imageDimensions.width, minDisplayDim.width);
    int height = imageDimensions.height;

    int windowWidth = 4*displayGap + 3*width;
    int windowHeight = 3*displayGap + height + panelHeight;
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
    ui->sliders->move(2*displayGap + width, 2 + displayGap + height);

    // resize other panels
    ui->leftFrame->resize(width, panelHeight);
    ui->rightFrame->resize(width, panelHeight);

    ui->leftFrame->move(displayGap, 2 * displayGap + height);
    ui->rightFrame->move(3*displayGap + 2*width, 2 * displayGap + height);
}
