#include "cervix_segmentation.hpp"

/*****************************************************************************
 *                            Cervix Segmentation                            *
 *****************************************************************************/

cervix_segmentation::cervix_segmentation()
{
    
}

cervix_segmentation::~cervix_segmentation()
{
    
}

void cervix_segmentation::read(const rapidjson::Value& json)
{
    
}

void cervix_segmentation::write(rapidjson::Value& json, rapidjson::Document& d)
{
    
}
        
void cervix_segmentation::segment(vector<Mat>& src, int i, Mat& dst)
{
    this->segment(src[i], dst);
}

float cervix_segmentation::segment(Mat& src, Mat& dst)
{
    src.copyTo(dst);
}

/*****************************************************************************
 *                       Watershed Cervix Segmentation                       *
 *****************************************************************************/

watershed_cs::watershed_cs(uchar markers_threshold_min,
                           uchar markers_threshold_max,
                           uchar markers_bg_threshold_min,
                           uchar markers_bg_threshold_max)
{
    this->markers_threshold_min = markers_threshold_min;
    this->markers_threshold_max = markers_threshold_max;
    this->markers_bg_threshold_min = markers_bg_threshold_min;
    this->markers_bg_threshold_max = markers_bg_threshold_max;
}

watershed_cs::~watershed_cs()
{
    
}

void watershed_cs::read(const rapidjson::Value& json)
{
    
}

void watershed_cs::write(rapidjson::Value& json, rapidjson::Document& d)
{
    
}

float watershed_cs::segment(Mat& src, Mat& dst)
{
    Mat binary;
    cvtColor(src, binary, CV_BGR2GRAY);
    threshold(binary, binary,
              this->markers_threshold_min, this->markers_threshold_max,
              THRESH_BINARY);

    // Eliminate noise and smaller objects
    Mat fg;
    erode(binary, fg, Mat(), Point(-1, -1), 2);

    // Identify src pixels without objects
    Mat bg;
    dilate(binary, bg, Mat(), Point(-1, -1), 3);
    threshold(bg, bg, 1, 128, THRESH_BINARY_INV);
    
    // Create markers src
    Mat markers(binary.size(), CV_8U, Scalar(0));
    markers = fg + bg;
    
    // Create watershed segmentation object
    markers.convertTo(markers, CV_32S);
    watershed(src, markers);
    markers.convertTo(markers, CV_8U);
    
    threshold(markers, dst, 128, 255, THRESH_BINARY);
}

/*****************************************************************************
 *                         Blobs Cervix Segmentation                         *
 *****************************************************************************/

blobs_cs::blobs_cs(cervix_segmentation* a, float min_size, int num_max_blobs)
{
    this->underlying = a;
    this->min_size = min_size;
    this->num_max_blobs = num_max_blobs;
}

blobs_cs::~blobs_cs()
{
    
}

void blobs_cs::read(const rapidjson::Value& json)
{
    
}

void blobs_cs::write(rapidjson::Value& json, rapidjson::Document& d)
{
    
}

float blobs_cs::segment(Mat& src, Mat& dst)
{
    Mat aux;
    this->underlying->segment(src, aux);
    this->filter_blobs(aux, dst);
}

void blobs_cs::filter_blobs(Mat& src, Mat& dst)
{
    Mat aux;
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;

    src.copyTo(aux);
    findContours(aux, contours, hierarchy, CV_RETR_CCOMP,
                 CV_CHAIN_APPROX_SIMPLE);

    float msize = this->min_size * src.rows * src.cols;
    vector< pair<float, int> > blobs;
    
    for (size_t i = 0; i < contours.size(); i++) {
        double area = contourArea(contours[i]);
        blobs.push_back(make_pair(-area, i));
    }
    
    sort(blobs.begin(), blobs.end());
    src.copyTo(dst);
    
    for (size_t i = 0; i < blobs.size(); i++){
        if ( abs(blobs[i].first) < msize || i >= this->num_max_blobs ){
            drawContours(dst, contours, blobs[i].second,
                         cv::Scalar(0), CV_FILLED, 8);
        }
    }
}

/*****************************************************************************
 *                      Convex Hull Cervix Segmentation                      *
 *****************************************************************************/

convex_hull_cs::convex_hull_cs(cervix_segmentation* a)
{
    this->underlying = a;
}

convex_hull_cs::~convex_hull_cs()
{
    
}

void convex_hull_cs::read(const rapidjson::Value& json)
{
    
}

void convex_hull_cs::write(rapidjson::Value& json, rapidjson::Document& d)
{
    
}

float convex_hull_cs::segment(Mat& src, Mat& dst)
{
    Mat aux;
    this->underlying->segment(src, aux);
    this->get_convex_hull(aux, dst);
}

void convex_hull_cs::get_convex_hull(Mat& src, Mat& dst)
{
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;

    findContours(src, contours, hierarchy, CV_RETR_EXTERNAL,
                 CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

    vector<vector<Point> > hull(contours.size());
    for( int i = 0; i < contours.size(); i++ ){
        convexHull( Mat(contours[i]), hull[i], false );
    }

    dst = Mat::zeros(src.size(), CV_8U);
    for( int i = 0; i < contours.size(); i++ )
    {
        drawContours(dst, hull, i, cv::Scalar(255), CV_FILLED, 8);
    }
}
