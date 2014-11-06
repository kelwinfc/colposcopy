#include "test_specular_reflection.hpp"

/**
 * @brief makeCanvas Makes composite image from the given images
 * @param vecMat Vector of Images.
 * @param windowHeight The height of the new composite image to be formed.
 * @param nRows Number of rows of images. (Number of columns will be calculated
 *              depending on the value of total number of images).
 * @return new composite image.
 */
Mat makeCanvas(vector<Mat>& vecMat, int windowHeight, int nRows) {
    vector<Mat> new_vecMat;
    for (size_t i = 0; i < vecMat.size(); i++)
    {
        Mat next = vecMat[i];
        Mat new_next;

        if ( next.channels() == 3 ){
            next.copyTo(new_next);
        } else {
            vector<Mat> ch;
            for (int k = 0; k < 3; k++)
                ch.push_back(next);
            merge(ch, new_next);
        }
        
        new_vecMat.push_back(new_next);
    }
    
    int N = vecMat.size();
    nRows  = nRows > N ? N : nRows; 
    int edgeThickness = 10;
    int imagesPerRow = ceil(double(N) / nRows);
    int resizeHeight = floor(2.0 * ((floor(double(windowHeight -
                                           edgeThickness) / nRows)) / 2.0))
                        - edgeThickness;
    int maxRowLength = 0;

    vector<int> resizeWidth;
    for (int i = 0; i < N;) {
            int thisRowLen = 0;
            for (int k = 0; k < imagesPerRow; k++) {
                    double aspectRatio = double(vecMat[i].cols) / 
                                         vecMat[i].rows;
                    int temp = int( ceil(resizeHeight * aspectRatio));
                    resizeWidth.push_back(temp);
                    thisRowLen += temp;
                    if (++i == N) break;
            }
            if ((thisRowLen + edgeThickness * (imagesPerRow + 1)) > 
                    maxRowLength)
            {
                    maxRowLength = thisRowLen + 
                                   edgeThickness * (imagesPerRow + 1);
            }
    }
    int windowWidth = maxRowLength;
    cv::Mat canvasImage(windowHeight, windowWidth, CV_8UC3,
                        Scalar(0, 0, 0));

    for (int k = 0, i = 0; i < nRows; i++) {
            int y = i * resizeHeight + (i + 1) * edgeThickness;
            int x_end = edgeThickness;
            for (int j = 0; j < imagesPerRow && k < N; k++, j++) {
                    int x = x_end;
                    cv::Rect roi(x, y, resizeWidth[k], resizeHeight);
                    cv::Mat target_ROI = canvasImage(roi);
                    cv::resize(new_vecMat[k], target_ROI, target_ROI.size());
                    x_end += resizeWidth[k] + edgeThickness;
            }
    }
    return canvasImage;
}

/// Global Variables
const int threshold_max = 255;
int thrs = 150;

/// Matrices to store images
Mat src;

map<int, Mat> preprocessed_canvas;

void on_trackbar( int, void* )
{
    if ( preprocessed_canvas.find(thrs) == preprocessed_canvas.end() ){
        vector<Mat> vcanvas;
        
        vcanvas.push_back(src);
        
        Mat mask, dst_md, dst_ed;
        
        threshold_srd thrs_ip(thrs, 5);
        thrs_ip.detect(src, mask);
        vcanvas.push_back(mask);

        best_first_ip bfs;
        telea_ip telea(10.0);
        navier_stokes_ip ns(10.0);
        
        int num_algorithms = 3;
        img_inpaint* it[] = {&bfs, &telea, &ns};
        for (int alg = 0; alg < num_algorithms; alg++ ){
            Mat dst;
            it[alg]->fill(src, mask, dst);
            vcanvas.push_back(dst);
        }
        preprocessed_canvas[thrs] = makeCanvas(vcanvas, src.cols, 1);
    }

    imshow("Specular Reflection", preprocessed_canvas[thrs]);
}

int main(int argc, const char* argv[])
{
    argc--;
    argv++;

    if ( argc == 0 ){
        return -1;
    }

    src = imread(argv[0]);

    if ( src.data == 0 ){
        return -1;
    }

    {
        Mat aux;
        resize(src, aux, Size(200, 200));
        aux.copyTo(src);
    }

    // Create Windows
    namedWindow("Specular Reflection", 1);

    // Create Trackbars
    char tb_name[50];
    sprintf(tb_name, "Threshold x %d", threshold_max );

    createTrackbar(tb_name, "Specular Reflection",
                   &thrs, threshold_max,
                   on_trackbar);

    on_trackbar(thrs, 0);
    
    waitKey(0);

    cout << "Bye!" << endl;
    return 0;
}
