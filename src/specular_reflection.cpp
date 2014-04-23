#include "specular_reflection.hpp"

void detect_specular_reflection_das(Mat& src, Mat& dst, uchar thr)
{
    vector<Mat> channels;
    vector<Mat>::iterator it;

    split(src, channels);
    dst = Mat::zeros(src.rows, src.cols, CV_8UC1);
    dst = dst + 255;
    
    for ( it = channels.begin(); it != channels.end(); ++it ){
        Mat next = *it;
        threshold(next, next, thr, 255, THRESH_BINARY);
        dst = min(dst, next);
    }
}

int num_neighbors_with_information(Mat& mask, int x, int y)
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

void get_neighbors_avg(Mat& src, Mat& mask, int x, int y, Vec3b& ret)
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
        //cout << (int)ret[ch] << " ";
    }
    //cout << endl;
}

void fill_with_avg(Mat& src, Mat& mask, Mat& dst)
{
    Mat v;
    queue< pair<int, int> > q;
    
    //mask.copyTo(v);
    v = 255 - mask;
    erode(v, v, Mat());
    imshow("v", v);
    
    medianBlur(src, dst, 3);
    
    for ( int x = 0; x < v.rows; x++ ){
        for ( int y = 0; y < v.cols; y++ ){
            if ( v.at<uchar>(x,y) == 0 &&
                 num_neighbors_with_information(v, x, y) > 0 )
            {
                q.push( make_pair(x,y) );
            }
        }
    }
    
    while ( !q.empty() ){
        pair<int, int> next = q.front();
        q.pop();
        
        int x = next.first;
        int y = next.second;
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
                    q.push( make_pair(nx, ny) );
                }
            }
        }
    }
}