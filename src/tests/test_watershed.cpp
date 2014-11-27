#include "test_watershed.hpp"

int main(int argc, char* argv[])
{
    Mat src = cv::imread(argv[1]);
    Mat dst;
    
    imshow("src", src);
    
    watershed_cs ws;
    ws.segment(src, dst);

    imshow("watershed", dst);

    blobs_cs bs(&ws, 0.2, 2);
    bs.segment(src, dst);
    imshow("blobs", dst);

    convex_hull_cs chs(&bs);
    chs.segment(src, dst);
    imshow("convex hull", dst);

    waitKey(0);

    return 0;
}
