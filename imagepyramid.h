#ifndef IMAGEPYRAMID_H
#define IMAGEPYRAMID_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <math.h>

#include <iostream>

using namespace cv;

/**
 * @brief The imagePyramid class
 */
class ImagePyramid
{
public:
    /* Constructors */
    /**
     * @brief imagePyramid creates an imagePyramid with a specified
     * size
     * @param src the image
     * @param size the size to use for the image
     */
    ImagePyramid(const Mat &src, const Size &size);
    /**
     * @brief imagePyramid creates an imagePyramid using the size
     * of the image
     * @param src the image
     */
    ImagePyramid(const Mat &src);
    /**
     * @brief ImagePyramid default constructor for default
     * constructor purposes.
     */
    ImagePyramid() {imageSize = Size(512, 512);}

    /**
     * @brief imagePyramid creates an imagePyramid by combining
     * the layers of two imagePyramids of the image size and
     * layer number
     * @param src1 the first source
     * @param src2 the second source. Must be the same image
     * size and number of layers as src1
     * @param src1Mask the mask for src1. Must be CV_32FC1. The
     * mask for src2 will be calculated from this.
     */
    ImagePyramid(
            const ImagePyramid &src1,
            const ImagePyramid &src2,
            const Mat &src1Mask
            );

    /* Getters for image */
    /**
     * @brief getImage gets the original image
     * @return the image
     */
    Mat getImage() const {return image.clone();}
    /**
     * @brief getResizedImage gets the resized image used for the
     * pyramids
     * @return the resized version of the image
     */
    Mat getResizedImage() const {return resizedImage.clone();}
    /**
     * @brief getResizedImage gets a resized version of the
     * image
     * @param size size of image to get
     * @return resized image
     */
    Mat getResizedImage(const Size &size) const;
    /**
     * @brief getSize changes the size of the image
     * @return the size of the image
     */
    Size getSize() const {return imageSize;}
    /**
     * @brief getWidth gets the width used for the image
     * @return the width
     */
    int getWidth() const {return getSize().width;}
    /**
     * @brief getHeight gets the height used for the image
     * @return the height
     */
    int getHeight() const {return getSize().height;}

    /* Setters for image */
    /**
     * @brief setImage sets the image used and generates the
     * Laplacian pyramid
     * @param img the image to set
     * @param resize if true, use the image's size, if false,
     * use the current size. Default is true.
     * @param generatePyr whether to gernerate the image pyramid
     * @return 0 if no error
     */
    int setImage(const Mat &src, bool keepSize=true);
    /**
     * @brief setSize changes the size of the image and generates
     * the Laplacian pyramid
     * @param size the size to use for the image;
     * @return 0 if no error
     */
    int setSize (const Size &size);
    /**
     * @brief setSize changes the size of the image and generates
     * the Laplacian pyramid. This function is an overload of
     * the above one.
     * @param width width to use for the image
     * @param height height to use for the image
     * @return 0 if no error
     */
    int setSize (int width, int height)
    {return setSize(Size(width, height));}

    /**
     * @brief getLaplacian gets the specified layer of the
     * Laplacian pyramid
     * @param layer the layer to get
     * @return the layer. Empty if layer not valid
     */
    Mat getLaplacian(int layer) const {
        if (layer < 0 || layer >= getLayers()) {
            return Mat();   // empty matrix
        }
        else {
            return laplacianPyr[layer].clone();
        }
    }

    /* Layers */
    /**
     * @brief maxLayers gets the maximum number of layers for
     * the image pyramid
     * @return the maximum number of layers possible for the image
     * at the size used
     */
    unsigned int maxLayers() const {

        //return (int) log2(min(getWidth(), getHeight()));

        // max number of times the dimensions are divisible by
        // 2 and still and int

        unsigned int n = 1; // 0th layer
        int width = getWidth(), height = getHeight();

        // another pyrDown is possible if
        while (
               (width % 2 == 0) && (height % 2 == 0) &&
               (width >= 32) && (height >= 32)
               ) {
            width /= 2;
            height /= 2;
            n++;
        }

        // maximum layers possible without resizing?
        return n;

    }
    /**
     * @brief getLayers gets the number of layers used
     * @return the number of layers
     */
    int getLayers() const {return laplacianPyr.size();}
    /**
     * @brief setLayers sets the number of layers used. This
     * will automatically change the number of layers in
     * the Laplacian pyramid.
     * @param layers the number of layers to use, must be
     * positive and less than or equal to maxLayers()
     * @return 0 if no error, -1 if too small, 1 if too large
     */
    int setLayers(int layers);

private:
    Mat image;

    Mat resizedImage;
    Size imageSize;

    std::vector<Mat> laplacianPyr;

    /**
     * @brief resizeImage sets resizedImage based on imageSize
     */
    void resizeImage() {
        resize(image, resizedImage, imageSize, INTER_CUBIC);
        generatePyramid();
    }

    /* Helpers for the pyramid */
    /**
     * @brief generatePyramid generates the Laplacian pyramid
     * up to layers layers
     */
    void generatePyramid();
    /**
     * @brief expandPyramid expands the Laplacian pyramid by 1
     * layer, incrementing layer variable
     */
    void expandPyramid();
    /**
     * @brief shrinkPyramid shrinks the Laplacian pyramid by 1
     * layer, decrementing the layer variable
     */
    void shrinkPyramid();

    /**
     * @brief addMaskedLaplacian Adds 2 signed or unsigned
     * images using a mask
     * @param src1 the first image
     * @param src2 the second image
     * @param src1Mask the mask for the first image. The mask
     * for the second image is this mask inverted
     * @return the combined image
     */
    Mat addMaskedLaplacian(
            const Mat &src1, const Mat &src2,
            const Mat &src1Mask) const;

    void reconstructImage();


};

#endif // IMAGEPYRAMID_H
