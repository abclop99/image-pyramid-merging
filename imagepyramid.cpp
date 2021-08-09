#include "imagepyramid.h"

ImagePyramid::ImagePyramid(const Mat &src, const Size &size) :
    ImagePyramid(src)
{
    setSize(size);
}

ImagePyramid::ImagePyramid(const Mat &src) {
    setImage(src);
}

ImagePyramid::ImagePyramid(
        const ImagePyramid &src1,
        const ImagePyramid &src2,
        const Mat &src1Mask
        ) {

    Mat mask = src1Mask.clone();  // so it can be modified

    //combine pyramids
    for (int layer = 0; layer < src1.getLayers(); layer++) {

        laplacianPyr.push_back(
                    addMaskedLaplacian(
                        src1.getLaplacian(layer),
                        src2.getLaplacian(layer),
                        mask
                        )
                    );

        // downsize mask to fit next layer
        pyrDown(mask, mask);

    }

    // reconstruct image and set both resizedImage and image
    reconstructImage();
}

int ImagePyramid::setImage(const Mat &src, bool keepSize) {
    if (src.empty()) {
        return 1;   // error
    }
    else {
        src.copyTo(image);

        // if keepsize, use src's size, otherwise, keep current
        // size and resize
        if (keepSize) {
            setSize(image.size());
        }
        else {
            resizeImage();
        }

        // generates Laplacian pyramid
        generatePyramid();
        return 0;
    }
}

int ImagePyramid::setSize (const Size &size) {
    if (size.height <= 0 || size.width <= 0) {
        return 1;   // error
    }
    else {
        this->imageSize = size;
        resizeImage();
        generatePyramid();  // generates Laplacian pyramid
        return 0;
    }
}

int ImagePyramid::setLayers(int layers) {
    if (layers <= 1) {
        return -1;
    }
    else if (layers > maxLayers()) {
        return 1;
    }
    else {
        if (layers == getLayers()); // do nothing
        else if (layers < getLayers()) {
            // shrink pyramid
            while (layers < getLayers()) {
                shrinkPyramid();
            }
        }
        else if (layers > getLayers()) {
            // expand pyramid
            while(layers > getLayers()) {
                expandPyramid();
            }
        }
        return 0;
    }
}

void ImagePyramid::generatePyramid() {

    // Save number of layers
    unsigned int layers = getLayers();

    // Start from scratch
    laplacianPyr.clear();

    // Add resized image as layer 0
    // changed to CV_8S
    Mat layer0;
    laplacianPyr.push_back(resizedImage.clone());

    // Expand pyramd until there are enough layers
    while (laplacianPyr.size() < layers) {
        expandPyramid();
    }

}

void ImagePyramid::expandPyramid() {
    Mat layer1 = laplacianPyr.back().clone();
    laplacianPyr.pop_back();

    Mat layer2;
    pyrDown(layer1, layer2);

    Mat layer2Upscaled;
    pyrUp(layer2, layer2Upscaled);

    Mat layer1Laplacian;
    subtract(layer1, layer2Upscaled, layer1Laplacian, noArray(), CV_8S);

    laplacianPyr.push_back(layer1Laplacian);
    laplacianPyr.push_back(layer2);

}

void ImagePyramid::shrinkPyramid() {
    Mat layer1 = laplacianPyr.back();
    laplacianPyr.pop_back();
    Mat layer2 = laplacianPyr.back();
    laplacianPyr.pop_back();

    pyrUp(layer2, layer2);

    Mat layer1New;
    subtract(layer1, layer2, layer1New);

    laplacianPyr.push_back(layer1New);
}

Mat ImagePyramid::addMaskedLaplacian(
        const Mat &src1, const Mat &src2,
        const Mat &src1Mask) const {

    Mat combined;

    // assert left and right are same size
    assert(src1.rows == src2.rows);
    assert(src1.cols == src2.cols);
    assert(src1.channels() == src2.channels());
    assert(src1.type() == src2.type());
    assert(src1.channels() == 3);
    assert(src1Mask.type() == CV_32FC1);

    // create dst of same size as left and right
    combined.create(src1.rows, src1.cols, src1.type());

    for (int col = 0; col < src1.cols; col++) {
        for (int row = 0; row < src1.rows; row++) {

            // mask values
            float leftMaskValue = src1Mask.at<float>(row, col);
            float rightMaskValue = 1 - leftMaskValue;

            // colors of source images
            Vec3b leftColor = src1.at<Vec3b>(row, col);
            Vec3b rightColor = src2.at<Vec3b>(row, col);

            // reference for modifying dst image
            Vec3b & dstColor = combined.at<Vec3b>(row, col);

            // signed type
            if (src1.depth() == CV_8S) {
                dstColor[0] = ((signed char)leftColor[0] * leftMaskValue + (signed char)rightColor[0] * rightMaskValue);
                dstColor[1] = ((signed char)leftColor[1] * leftMaskValue + (signed char)rightColor[1] * rightMaskValue);
                dstColor[2] = ((signed char)leftColor[2] * leftMaskValue + (signed char)rightColor[2] * rightMaskValue);
            }
            // unsigned type
            else if (src1.depth() == CV_8U) {
                dstColor[0] = ((uint8_t)leftColor[0] * leftMaskValue + (uint8_t)rightColor[0] * rightMaskValue);
                dstColor[1] = ((uint8_t)leftColor[1] * leftMaskValue + (uint8_t)rightColor[1] * rightMaskValue);
                dstColor[2] = ((uint8_t)leftColor[2] * leftMaskValue + (uint8_t)rightColor[2] * rightMaskValue);
            }
        }
    }

    return combined;

}

void ImagePyramid::reconstructImage() {

    Mat image;
    int layers = getLayers();

    // start with the last layer (should be unsigned)
    image = laplacianPyr.back();

    // from second last to first
    for (int layer = layers-2; layer >= 0; layer--) {
        // upscale and add previous layer
        pyrUp(image, image);
        add(image, laplacianPyr[layer], image, noArray(), image.type());
    }

    // set image and resizedImage without using setters
    this->image = image.clone();
    this->resizedImage = image.clone();

}
