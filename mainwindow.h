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

    // maximum and minimum dimensions for displaying the image
    const Size maxDisplayDim = Size(580, 700);
    const Size minDisplayDim = Size(300,700);   // resize to fit

    const int displayGap = 20;
    const int panelHeight = 121;

    // Width and height used for images
    // const int imageWidth = 512, imageHeight = 512;

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

    /**
     * @brief loadImage Attempts to load an image from a path and returns
     * andy errors
     * @param dst output image
     * @param path path to the image file
     * @return 0 if no error, 1 for empty path, 2 for couldn't read image
     */
    int loadImage(Mat &dst, QString path);

    /**
     * @brief displayImage displays an image on a label
     * @param label the label to display the image on
     * @param img the image to display
     */
    void displayImage(QLabel *label, Mat img);
    /**
     * @brief displayImages displays the images and resizes
     * the UI to fit
     */
    void displayImages();
    /**
     * @brief resizeUI resizes the UI to fit the size of the images
     * @param imageDimensions the dimenesions of the images
     */
    void resizeUI(Size imageDimensions);

    /**
     * @brief setLeftErrorMessage sets the error message for
     * the left side
     * @param msg the error message
     */
    void setLeftErrorMessage(QString msg);
    /**
     * @brief setRightErrorMessage sets the error message for
     * the right side
     * @param msg the error message
     */
    void setRightErrorMessage(QString msg);

    /**
     * @brief imageMask generates a mask with a linear gradient.
     * @param rows number of rows in the mask
     * @param cols number of columns in the mask
     * @param startPercent the start position for the graident
     * @param endPercent the end position for the gradient
     * @return the mask
     */
    static Mat imageMask(
            int rows, int cols,
            int startPercent, int endPercent
            );

};
#endif // MAINWINDOW_H
