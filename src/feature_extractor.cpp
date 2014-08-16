#include "feature_extractor.hpp"

feature_extractor::feature_extractor()
{
    // noop
}

void feature_extractor::extract(vector<Mat>& in, int i, vector<float>& out)
{
    out.clear();
}

void feature_extractor::read(string filename)
{
    //noop
}

void feature_extractor::read(const rapidjson::Value& json)
{
    //noop
}


void feature_extractor::write(string filename)
{
    //TODO
}

void feature_extractor::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

identity_fe::identity_fe()
{
    // noop
}

void identity_fe::extract(vector<Mat>& in, int i, vector<float>& out)
{
    out.clear();

    int n = in[i].rows;
    int type = in[i].type();
    uchar depth = type & CV_MAT_DEPTH_MASK;

    out.clear();
    out.resize(n);

    for ( int j = 0; j < n; j++ ){
        switch ( depth ) {
            case CV_8U:  out[j] = in[i].at<uchar>(0,j); break;
            case CV_8S:  out[j] = in[i].at<char>(0,j); break;
            case CV_16U: out[j] = in[i].at<unsigned short>(0,j); break;
            case CV_16S: out[j] = in[i].at<short>(0,j); break;
            case CV_32S: out[j] = in[i].at<int>(0,j); break;
            case CV_32F: out[j] = in[i].at<float>(0,j); break;
            case CV_64F: out[j] = in[i].at<double>(0,j); break;
            default:     out[j] = in[i].at<uchar>(0,j); break;
        }
    }
}

void identity_fe::read(const rapidjson::Value& json)
{
    //noop
}


void identity_fe::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}


hue_histogram_fe::hue_histogram_fe()
{
    this->bindw = 10;
    this->normalize = true;
}

hue_histogram_fe::hue_histogram_fe(int bindw, bool normalize)
{
    this->bindw = bindw;
    this->normalize = normalize;
}

void hue_histogram_fe::extract(vector<Mat>& in, int i, vector<float>& out)
{
    Mat aux;
    float max_vh = 0;
    vector<Mat> hsv_planes;

    cvtColor(in[i], aux, CV_BGR2HSV);
    split( aux, hsv_planes );

    out.resize( 180 / this->bindw );
    fill(out.begin(), out.end(), 0.0);

    for ( int r = 0; r < in[i].rows; r++ ){
        for ( int c = 0; c < in[i].cols; c++ ){
            int bh = hsv_planes[0].at<uchar>(r, c) / this->bindw;
            out[bh] += 1.0;
            max_vh = max(max_vh, out[bh]);
        }
    }

    if ( this->normalize ){
        for ( uint i = 0; i < out.size(); i++ ){
            out[i] /= max_vh;
        }
    }
}

void hue_histogram_fe::read(const rapidjson::Value& json)
{
    //TODO
}

void hue_histogram_fe::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}


motion_fe::motion_fe()
{
    this->w      = 5;
    this->width  = 10;
    this->height = 10;
}

motion_fe::motion_fe(int w, int width, int height)
{
    this->w      = w;
    this->width  = width;
    this->height = height;
}
        
void motion_fe::extract(vector<Mat>& in, int i, vector<float>& out)
{
    out.resize(this->width * this->height);
    fill(out.begin(), out.end(), 0.0);
    
    Mat current_image;
    GaussianBlur(in[i], current_image, Size(3,3), 0.3);
    
    for ( int k = max(0, i - this->w - 1);
          k < min(i + this->w + 1, (int)in.size());
          k++ )
    {
        if ( k == i )
            continue;
        
        Mat next_image;
        GaussianBlur(in[k], next_image, Size(3,3), 0.3);
        
        for ( int r = 0; r < in[k].rows; r++ ){
            int fr = r / (in[k].rows / this->height);
            for ( int c = 0; c < in[k].cols; c++ ){
                int fc = c / (in[k].cols / this->width);
                
                Vec3b prev_pixel = next_image.at<Vec3b>(r, c);
                Vec3b next_pixel = current_image.at<Vec3b>(r, c);
                float diff = 0.0;
                
                for (uint x = 0; x < 3; x++){
                    diff += (prev_pixel[x] - next_pixel[x]) *
                            (prev_pixel[x] - next_pixel[x]);
                }
                
                out[fr * this->width + fc] += sqrt(diff);
            }
        }
    }
}

void motion_fe::read(const rapidjson::Value& json)
{
    //TODO
}

void motion_fe::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}
