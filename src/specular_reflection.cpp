#include "specular_reflection.hpp"

/*****************************************************************************
 *                            Specular Reflection                            *
 *****************************************************************************/

specular_reflection_detection::specular_reflection_detection()
{
    // noop
}

void specular_reflection_detection::detect(Mat& img, Mat& dst)
{
    dst = Mat::zeros(img.rows, img.cols, CV_8UC1);
}

/*****************************************************************************
 *                       Threshold Specular Reflection                       *
 *****************************************************************************/

threshold_srd::threshold_srd(uchar thr, uchar erosion_size)
{
    this->thrs = thr;
    this->erosion_size = erosion_size;
}

void threshold_srd::detect(Mat& img, Mat& dst)
{
    vector<Mat> channels;
    vector<Mat>::iterator it;

    split(img, channels);
    dst = Mat::zeros(img.rows, img.cols, CV_8UC1);
    dst = dst + 255;

    for ( it = channels.begin(); it != channels.end(); ++it ){
        Mat next = *it;
        threshold(next, next, this->thrs, 255, THRESH_BINARY);
        dst = min(dst, next);
    }

    dst = this->dilate_mask(dst);
}

Mat threshold_srd::dilate_mask(Mat& src)
{
    Mat ret;
    Mat element = getStructuringElement(MORPH_ELLIPSE,
                                        Size(2 * this->erosion_size + 1,
                                             2 * this->erosion_size + 1), 
                                        Point(this->erosion_size,
                                              this->erosion_size)
                                       );
    dilate(src, ret, element);
    return ret;
}

void threshold_srd::set_threshold(uchar thr)
{
    this->thrs = thr;
}

img_inpaint::img_inpaint()
{
    
}

void img_inpaint::fill(Mat& img, Mat& holes, Mat& dst)
{
    img.copyTo(dst);
}

class point {
    public:
        int x;
        int y;
        int px;
        int py;
        
        point(int x, int y, int px, int py){
            this-> x = x;
            this-> y = y;
            this->px = px;
            this->py = py;
        }
};

float euclidean_distance(int x0, int y0, int x1, int y1)
{
    float diffx = x1 - x0;
    float diffy = y1 - y0;
    return sqrt(diffx * diffx + diffy * diffy);
}

inline bool operator<(const point& a, const point& b)
{
    return euclidean_distance(a.x, a.y, a.px, a.py) >
           euclidean_distance(b.x, b.y, b.px, b.py);
}

best_first_ip::best_first_ip()
{
}

void best_first_ip::fill(Mat& img, Mat& holes, Mat& dst)
{
    Mat v;
    priority_queue<point> q;

    v = 255 - holes;
    
    img.copyTo(dst);
    medianBlur(dst, dst, 5);
    
    /* Initialize the open nodes with the boundaries in the
     * specular reflecion region
     */
    for ( int x = 0; x < v.rows; x++ ){
        for ( int y = 0; y < v.cols; y++ ){
            if ( v.at<uchar>(x,y) == 0 &&
                 num_neighbors_with_information(v, x, y) > 0 )
            {
                q.push(point(x, y, x, y));
            }
        }
    }

    /* Process each SR pixel */
    while ( !q.empty() ){
        point next = q.top();
        q.pop();
        
        int x = next.x;
        int y = next.y;
        int px = next.px;
        int py = next.py;

        Vec3b new_pixel(0,0,0);
        
        if ( v.at<uchar>(x, y) != 0 ){
            continue;
        }
        
        get_neighbors_avg(dst, v, x, y, dst.at<Vec3b>(x, y));
        v.at<uchar>(x, y) = 255;
        
        for ( int dx = -1; dx < 2; dx++ ){
            for ( int dy = -1; dy < 2; dy++ ){
                int nx = x + dx;
                int ny = y + dy;

                if ( 0 <= nx && nx < v.rows && 0 <= ny && ny < v.cols &&
                     v.at<uchar>(nx, ny) == 0
                   )
                {
                    q.push(point(nx, ny, px, py));
                }
            }
        }
    }

    /* Remove the effect of the blur filter */
    for ( int x = 0; x < v.rows; x++ ){
        for ( int y = 0; y < v.cols; y++ ){
            if ( holes.at<uchar>(x, y) == 0 )
            {
                dst.at<Vec3b>(x, y) = img.at<Vec3b>(x, y);
            }
        }
    }
}

void best_first_ip::get_neighbors_avg(Mat& src, Mat& mask, int x, int y,
                                      Vec3b& ret)
{
    ret = Vec3b(0, 0, 0);
    int accum[3] = {0, 0, 0};
    int n = 0;
    
    for ( int dx = -1; dx < 2; dx++ ){
        for ( int dy = -1; dy < 2; dy++ ){
            int nx = x + dx;
            int ny = y + dy;
            
            if ( 0 <= nx && nx < mask.rows && 0 <= ny && ny < mask.cols &&
                 (dx != 0 || dy != 0) && mask.at<uchar>(nx, ny) != 0
               )
            {
                for ( uchar ch = 0; ch < 3; ch++ ){
                    accum[ch] += src.at<Vec3b>(nx, ny)[ch];
                }
                n++;
            }
        }
    }
    
    for ( uchar ch = 0; ch < 3; ch++ ){
        ret[ch] = (uchar)( (float)accum[ch] / (float)n );
    }
}

int best_first_ip::num_neighbors_with_information(Mat& mask, int x, int y)
{
    int ret = 0;

    for ( int dx = -1; dx < 2; dx++ ){
        int nx = x + dx;

        if ( 0 <= nx && nx < mask.rows ){
            for ( int dy = -1; dy < 2; dy++ ){
                int ny = y + dy;

                if ( 0 <= ny && ny < mask.cols &&
                     (dx != 0 || dy != 0) &&
                     mask.at<uchar>(nx, ny) != 0
                   )
                {
                    ret++;
                }
            }
        }
    }
    
    return ret;
}

telea_ip::telea_ip(float r)
{
    this->r = r;
}

void telea_ip::fill(Mat& img, Mat& holes, Mat& dst)
{
    inpaint(img, holes, dst, this->r, INPAINT_TELEA);
}

navier_stokes_ip::navier_stokes_ip(float r)
{
    this->r = r;
}

void navier_stokes_ip::fill(Mat& img, Mat& holes, Mat& dst)
{
    inpaint(img, holes, dst, this->r, INPAINT_NS);
}

void fill_with_d2(Mat& src, Mat& mask, Mat& dst)
{
    imshow("v", src);
}

