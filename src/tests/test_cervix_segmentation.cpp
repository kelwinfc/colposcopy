#include "test_cervix_segmentation.hpp"

void show_img(string name, Mat& img, Mat& mask)
{
    vector<Mat> channels;
    Mat aux;
    
    for (int ch=0; ch<3; ch++)
        channels.push_back(mask);
    merge(channels, aux);
    
    aux = min(img, aux);
    imshow(name.c_str(), aux);
}

int main(int argc, char* argv[])
{
    Mat src = cv::imread(argv[1]);
    Mat dst, hole;

    imshow("src", src);

    watershed_cs ws;
    blobs_cs bs(&ws, 0.2, 2);
    find_hole_cs hw(&bs);
    bs.segment(src, dst);
    //show_img("watershed", src, dst);
    hw.segment(src, dst);
    //show_img("hole watershed", src, dst);
    /*
    kmeans_cs k;
    blobs_cs bsk(&k, 0.2, 2);
    find_hole_cs hk(&bsk);
    bsk.segment(src, dst);
    show_img("k-means", src, dst);
    hk.segment(src, dst);
    show_img("hole k-means", src, dst);
    
    waitKey(0);
    */
    return 0;
}
