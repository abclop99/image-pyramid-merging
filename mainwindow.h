#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ui_mainwindow.h"
#include "imagepyramid.h"

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

    // maximum dimensions for displaying the image
    const Size maxDisplayDim = Size(600, 700);

    // Width and height used for images
    const int imageWidth = 512, imageHeight = 512;

private slots:
    void combineImages();

    void handleLeftFileButton();
    void handleRightFileButton();
    void submitLeftImage();
    void submitRightImage();

private:
    Ui::MainWindow *ui;

    const QString imageFileFilter = tr("Images (*.png *.xpm *.jpg *.tiff);; All Files (*)");

    const QString emptyMsg = "";
    const QString emptyPathMsg  = tr("The path is empty.");
    const QString imageInvalidMsg   = tr("The image could not be read.");
    const QString unknownErrorMsg   = tr("An unknown error occurred");

    static const int initialLayers = 6;

    // Image Pyramids
    ImagePyramid leftPyr;
    ImagePyramid rightPyr;
    ImagePyramid combinedPyr;

    int loadImage(Mat &dst, QString path);

    void displayImage(QLabel *label, Mat img);
    void displayImages();

    void setLeftErrorMessage(QString msg);
    void setRightErrorMessage(QString msg);

    static Mat imageMask(
            int rows, int cols,
            int startPercent, int endPercent
            );

};
#endif // MAINWINDOW_H
