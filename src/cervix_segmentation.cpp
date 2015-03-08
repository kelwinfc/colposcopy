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

void cervix_segmentation::segment_from_binary(vector<Mat>& src, int i,
                                              Mat& dst)
{
    this->segment(src[i], dst);
}

float cervix_segmentation::segment_from_binary(Mat& src, Mat& dst)
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
    
    for (int thrs = this->markers_threshold_max; thrs >= 0; thrs -= 10){
        Mat aux;
        threshold(binary, aux, thrs, this->markers_threshold_max,
                  THRESH_BINARY);

        if ( sum(aux)[0] >= 255 * 0.3 * src.rows * src.cols ){
            aux.copyTo(binary);
            break;
        }
    }
    
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

float watershed_cs::segment_from_binary(Mat& src, Mat& dst)
{
    src.copyTo(dst);
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

float blobs_cs::segment_from_binary(Mat& src, Mat& dst)
{
    this->filter_blobs(src, dst);
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

float convex_hull_cs::segment_from_binary(Mat& src, Mat& dst)
{
    this->get_convex_hull(src, dst);
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

/*****************************************************************************
 *                        K-Means Cervix Segmentation                        *
 *****************************************************************************/

kmeans_cs::kmeans_cs()
{
    
}

kmeans_cs::~kmeans_cs()
{
    
}
        
void kmeans_cs::read(const rapidjson::Value& json)
{
    // TODO
}

void kmeans_cs::write(rapidjson::Value& json, rapidjson::Document& d)
{
    // TODO
}

float kmeans_cs::segment(Mat& src, Mat& dst)
{
    Mat labels, centers;
    Mat samples = Mat::zeros(src.rows * src.cols, 3, CV_32F);
    
    int clusterCount = 2;
    int attempts = 10;
    
    Mat src_lab;
    cvtColor(src, src_lab, CV_BGR2Lab);
    vector<Mat> channels;
    split(src_lab, channels);
    
    for (int i = 0; i < src.rows; i++){
        for (int j = 0; j < src.cols; j++){
            for (int ch=0; ch<3; ch++)
                samples.at<float>(i * src.cols + j, ch) = 
                    (float)channels[ch].at<uchar>(i, j) / 255.0;
        }
    }
    
    kmeans(samples, clusterCount, labels,
           TermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 10000, 0.0001),
           attempts, KMEANS_PP_CENTERS, centers);
    
    dst = Mat::zeros(src.rows, src.cols, CV_8U);
    
    int pcounter = 0;
    int ncounter = 0;
    
    for (int i = 0; i < src.rows; i++){
        for (int j = 0; j < src.cols; j++){
            if (labels.at<int>(0, i * src.cols + j) == 1){
                dst.at<uchar>(i, j) = 255;
                pcounter++;
            } else {
                ncounter++;
            }
        }
    }
    
    if ( ncounter > pcounter ){
        dst = 255 - dst;
    }
}

float kmeans_cs::segment_from_binary(Mat& src, Mat& dst)
{
    src.copyTo(dst);
}


/*****************************************************************************
 *                       Find Hole Cervix Segmentation                       *
 *****************************************************************************/

find_hole_cs::find_hole_cs(cervix_segmentation* u)
{
    this->underlying = u;
}

find_hole_cs::~find_hole_cs()
{
    
}
        
void find_hole_cs::read(const rapidjson::Value& json)
{
    
}

void find_hole_cs::write(rapidjson::Value& json, rapidjson::Document& d)
{
    
}

float find_hole_cs::segment(Mat& src, Mat& dst)
{
    Mat src_gray, binary, aux, masked_src;
    this->underlying->segment(src, binary);
    
    cvtColor(src, src_gray, CV_BGR2GRAY);
    masked_src = min(src_gray, binary);
    
    for (int thrs = 1; thrs < 255; thrs *= 2){
        threshold(masked_src, aux, thrs, 255, THRESH_BINARY);

        this->segment_from_binary(aux, dst);
        
        if ( this->found ){
            break;
        }
    }
}

float find_hole_cs::segment_from_binary(Mat& src, Mat& dst)
{
    pair<int, int> center_img = get_center(src);
    this->found = false;
    
    Mat aux;
    this->fill_external_region(src, aux);
    aux = 255 - aux;
    this->found = this->get_center_blob(aux, dst, center_img);
}

void find_hole_cs::fill_external_region(Mat& src, Mat& dst)
{
    src.copyTo(dst);
    queue< pair<int, int> > q;
    
    // Top/Bottom border
    for (int j = 0; j < src.cols; j++){
        
        // Top border
        if ( dst.at<uchar>(0, j) == 0 ){
            q.push(make_pair(0, j));
        }
        
        // Bottom border
        if ( dst.at<uchar>(dst.rows - 1, j) == 0 ){
            q.push(make_pair(dst.rows - 1, j));
        }
    }
    
    // Left/Right border
    for (int i = 0; i < src.rows; i++){
        
        // Left border
        if ( dst.at<uchar>(i, 0) == 0 ){
            q.push(make_pair(i, 0));
        }
        
        // Right border
        if ( dst.at<uchar>(i, dst.cols - 1) == 0 ){
            q.push(make_pair(i, dst.cols - 1));
        }
    }
    
    while( !q.empty() ){
        pair<int, int> next = q.front();
        q.pop();
        
        int i = next.first;
        int j = next.second;
        
        if ( dst.at<uchar>(i, j) != 0 ){
            continue;
        }
        
        dst.at<uchar>(i, j) = 255;
        
        for (int di=-1; di<2; di++){
            for (int dj=-1; dj<2; dj++){
                int ni = i + di;
                int nj = j + dj;
                
                if ( 0 <= ni && ni < dst.rows && 0 <= nj && nj < dst.cols &&
                     dst.at<uchar>(ni, nj) == 0
                   )
                {
                    q.push(make_pair(ni, nj));
                }
            }
        }
    }
}

pair<pair<int, int>, int> find_hole_cs::get_center_from_point(Mat& src,
                                                              int i, int j)
{
    int reti = 0;
    int retj = 0;
    int n = 0;
    
    queue< pair<int, int> > q;
    q.push(make_pair(i, j));
    
    while( !q.empty() ){
        pair<int, int> next = q.front();
        q.pop();
        
        int i = next.first;
        int j = next.second;
        
        if ( src.at<uchar>(i, j) == 0 ){
            continue;
        }
        
        src.at<uchar>(i, j) = 0;
        reti += i;
        retj += j;
        n++;
        
        for (int di=-1; di<2; di++){
            for (int dj=-1; dj<2; dj++){
                int ni = i + di;
                int nj = j + dj;
                
                if ( 0 <= ni && ni < src.rows && 0 <= nj && nj < src.cols &&
                     src.at<uchar>(ni, nj) != 0
                   )
                {
                    q.push(make_pair(ni, nj));
                }
            }
        }
    }
    
    if ( n == 0 ){
        return make_pair(make_pair(src.rows / 2, src.cols / 2), n);
    }
    
    return make_pair(make_pair(reti / n, retj / n), n);
}

void find_hole_cs::draw_from_point(Mat& src, pair<int, int> start, Mat& dst)
{
    queue< pair<int, int> > q;
    q.push(start);
    
    dst = Mat::zeros(src.rows, src.cols, CV_8U);
    
    while( !q.empty() ){
        pair<int, int> next = q.front();
        q.pop();
        
        int i = next.first;
        int j = next.second;
        
        if ( dst.at<uchar>(i, j) != 0 ){
            continue;
        }
        
        dst.at<uchar>(i, j) = 255;
        
        for (int di=-1; di<2; di++){
            for (int dj=-1; dj<2; dj++){
                int ni = i + di;
                int nj = j + dj;
                
                if ( 0 <= ni && ni < src.rows && 0 <= nj && nj < src.cols &&
                     src.at<uchar>(ni, nj) != 0 && dst.at<uchar>(ni, nj) == 0
                   )
                {
                    q.push(make_pair(ni, nj));
                }
            }
        }
    }
}

void find_hole_cs::get_centers(Mat& src,
                               vector< pair<int, int> >& starting_points,
                               vector< pair<int, int> >& centers,
                               vector<int>& area
                              )
{
    Mat aux;
    src.copyTo(aux);
    
    for (int i=0; i<src.rows; i++){
        for (int j=0; j<src.cols; j++){
            if ( aux.at<uchar>(i, j) != 0 ){
                pair<pair<int, int>, int> next = 
                    this->get_center_from_point(aux, i, j);
                centers.push_back(next.first);
                area.push_back(next.second);
                starting_points.push_back(make_pair(i, j));
            }
        }
    }
}

bool find_hole_cs::get_center_blob(Mat& src, Mat& dst,
                                   pair<int, int> center_img)
{
    Mat aux;
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;
    
    src.copyTo(aux);
    
    vector< pair<int, int> > centers;
    vector< pair<int, int> > starting_points;
    vector<int> areas;
    
    get_centers(aux, starting_points, centers, areas);
    
    int best_contour = -1;
    float best_dist_to_center = src.rows * src.rows + src.cols * src.cols;
    
    for (int i = 0; i < centers.size(); i++ ){
        float next_dist = pair_distance(center_img, centers[i]);
        
        if ( areas[i] > 100 && next_dist < best_dist_to_center ){
            best_dist_to_center = next_dist;
            best_contour = i;
        }
    }
    
    if ( best_contour != -1 ){
        this->draw_from_point(src, centers[best_contour], dst);
        return true;
    } else {
        dst = Mat::zeros(src.rows, src.cols, CV_8U);
        return false;
    }
}
