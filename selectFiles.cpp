#include "mainwindow.h"
#include "QFileDialog"

#include <iostream>

void MainWindow::combineImages() {
    combinedPyr = ImagePyramid(
                leftPyr, rightPyr,
                imageMask(
                    leftPyr.getWidth(), leftPyr.getHeight(),
                    ui->startSlider->value(),
                    ui->endSlider->value()));

    displayImages();
}

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
        submitLeftImage();
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
        submitRightImage();
    }
}

/**
 * @brief MainWindow::handleLeftFileSubmit
 * Attempts to read an image from the path in the text line and
 * use it for merging with the right image.
 */
void MainWindow::submitLeftImage() {
    // Read string from text line
    QString path = ui->leftFileText->text();

    Mat leftImage;

    // Attempt loading an image, display error message if error
    switch (loadImage(leftImage, path)) {
    case 0:     // no error
        setLeftErrorMessage(emptyMsg);
        break;
    case 1:     // empty path
        setLeftErrorMessage(emptyPathMsg);
        break;
    case 2:     // Could not read file
        setLeftErrorMessage(imageInvalidMsg);
    default:    // unknown error, probably invalid image
        setLeftErrorMessage(imageInvalidMsg);
        break;
    }

    // set the image
    leftPyr.setImage(leftImage);

    // set right image size
    rightPyr.setSize(leftPyr.getSize());

    combineImages();
    displayImages();

}
void MainWindow::submitRightImage() {
    QString path = ui->rightFileText->text();

    Mat rightImage;

    // Attempt loading an image, display error message if error
    switch (loadImage(rightImage, path)) {
    case 0:     // no error
        setRightErrorMessage(emptyMsg);
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

    // set the image without changing the size used
    rightPyr.setImage(rightImage, false);

    combineImages();
    displayImages();
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

    dst = image.clone();

    return 0;
}
