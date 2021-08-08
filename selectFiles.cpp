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
        ui->leftFIleText->setText(path);
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
        ui->rightFIleText->setText(path);
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
    QString path = ui->leftFIleText->text();

    // if path empty, error
    if (path == emptyMsg) {
        setLeftErrorMessage(emptyPathMsg);
        return;
    }

    // Read image s
    cv::Mat image = cv::imread(path.toStdString(), cv::IMREAD_COLOR);

    // Check if the image was read correctly
    if (image.empty()) {
        setLeftErrorMessage(imageInvalidMsg);
        return;
    }

    setLeftErrorMessage(emptyMsg);

    // Set left image, resize if neccessary
    if (image.rows == imageWidth && image.cols == imageHeight) {
        leftImage = image.clone();
    }
    else {
        cv::resize(image, leftImage, cv::Size(imageWidth, imageHeight), INTER_CUBIC);
    }

    laplacianPyr(layers, leftPyr, leftImage);
    combineImages();



}
void MainWindow::handleRightFileSubmit() {
    QString path = ui->rightFIleText->text();
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

