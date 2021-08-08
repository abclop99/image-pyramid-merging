#include "mainwindow.h"
#include "QFileDialog"

#include <iostream>

/**
 * @brief MainWindow::handleLeftFileButton
 * Shows a file dialog to select an image attempts to load it as the left image.
 */
void MainWindow::handleLeftFileButton() {
    QString path = QFileDialog::getOpenFileName(
                this, tr("Select Left Image"), "", imageFileFilter
    );

    // Only attempt loading if a file was selected
    if (path != emptyMsg) {
        ui->leftFileText->setText(path);
        handleLeftFileSubmit();
    }
}

/**
 * @brief MainWindow::handleRightFileButton
 * Shows a file dialog to select an image attempts to load it as the right image.
 */
void MainWindow::handleRightFileButton() {
    QString path = QFileDialog::getOpenFileName(
                this, tr("Select Right Image"), "", imageFileFilter
    );

    // Only attempt loading if a file was selected
    if (path != emptyMsg) {
        ui->rightFileText->setText(path);
        handleRightFileSubmit();
    }
}

/**
 * @brief MainWindow::handleLeftFileSubmit
 * Attempts to read an image from the path in the text line and
 * use it for merging with the right image.
 */
void MainWindow::handleLeftFileSubmit() {
    // Read string from text line
    QString path = ui->leftFileText->text();

    // Attempt loading an image, display error message if error
    switch (loadImage(leftImage, path)) {
    case 0:     // no error
        break;
    case 1:     // empty path
        setLeftErrorMessage(emptyPathMsg);
        break;
    case 2:     // Could not read file
        setLeftErrorMessage(imageInvalidMsg);
    default:    // unknown error
        setLeftErrorMessage(unknownErrorMsg);
        break;
    }

    // Do pyramid stuff again
    laplacianPyr(layers, leftPyr, leftImage);
    combineImages();

}
void MainWindow::handleRightFileSubmit() {
    QString path = ui->rightFileText->text();

    // Attempt loading an image, display error message if error
    switch (loadImage(rightImage, path)) {
    case 0:     // no error
        break;
    case 1:     // empty path
        setRightErrorMessage(emptyPathMsg);
        break;
    case 2:     // Could not read file because it ended up empty
        setRightErrorMessage(imageInvalidMsg);
    default:    // Invalid image
        setRightErrorMessage(imageInvalidMsg);
        break;
    }

    // Do pyramid stuff again
    laplacianPyr(layers, rightPyr, rightImage);
    combineImages();
}

/**
 * @brief MainWindow::setLeftErrorMessage
 * @param msg the message to set
 */
void MainWindow::setLeftErrorMessage(QString msg) {
    ui->leftErrorMessage->setText(msg);
}

/**
 * @brief MainWindow::setRightErrorMessage
 * @param msg the message to set
 */
void MainWindow::setRightErrorMessage(QString msg) {
    ui->rightErrorMessage->setText(msg);
}

/**
 * @brief MainWindow::loadImage
 * @param dst Mat to store the image in
 * @param path filepath of the image
 * @return 0 if no error, 1 for empty path, 2 for couldn't read image
 */
int MainWindow::loadImage(Mat &dst, QString path) {

    if (path == emptyMsg) {
        return 1;
    }

    // read in the images from resource
    QFile file(path);

    // Load image
    Mat image;
    if (file.open(QIODevice::ReadOnly)){
        qint64 sz = file.size();
        std::vector<uchar> buf(sz);
        file.read((char*)buf.data(), sz);
        image = imdecode(buf, IMREAD_COLOR);
    }

    // Check if image was read correctly (not empty)
    if (image.empty()) {
        return 2;
    }

    // Set dst image, resize if neccessary
    if (image.rows == imageWidth && image.cols == imageHeight) {
        dst = image.clone();
    }
    else {
        cv::resize(image, dst, cv::Size(imageWidth, imageHeight), INTER_CUBIC);
    }

    return 0;
}
