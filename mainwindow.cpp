#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

void displayImage(QLabel *label, Mat img);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Merging 2 images using Laplacian Pyramids");

    const std::string image1Path = "C:/Users/abclo/Documents/Projects/image-pyramids/apple.jpg";
    const std::string image2Path = "C:/Users/abclo/Documents/Projects/image-pyramids/orange.jpg";

    Mat image1;
    Mat image2;

    Mat reconstruction;

    // read in the images
    image1 = imread(image1Path, IMREAD_COLOR);
    image2 = imread(image2Path, IMREAD_COLOR);

    // display images
    displayImage(ui->image1Label, image1);
    displayImage(ui->image2Label, image2);

}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
 * Displays an opencv image in a QLabel.
 */
void displayImage(QLabel *label, Mat img) {
    Mat image;
    cv::cvtColor(img, image, CV_BGR2RGB);
    label->setPixmap(QPixmap::fromImage((QImage(image.data, image.cols, image.rows, image.step, QImage::Format_RGB888))));
}

