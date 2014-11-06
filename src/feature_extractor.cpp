#include "feature_extractor.hpp"

/*****************************************************************************
 *                             Feature Extractor                             *
 *****************************************************************************/

feature_extractor::feature_extractor()
{
    // noop
}

void feature_extractor::extract(vector<Mat>& in, int i, vector<float>& out,
                             anonadado::instance* instance)
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

/*****************************************************************************
 *                        Identity Feature Extractor                         *
 *****************************************************************************/

identity_fe::identity_fe()
{
    // noop
}

void identity_fe::extract(vector<Mat>& in, int i, vector<float>& out,
                          anonadado::instance* instance)
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

/*****************************************************************************
 *                           Hue Feature Extractor                           *
 *****************************************************************************/

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

void hue_histogram_fe::extract(vector<Mat>& in, int i, vector<float>& out,
                               anonadado::instance* instance)
{
    Mat aux;
    vector<Mat> hsv_planes;

    cvtColor(in[i], aux, CV_BGR2HSV);
    split( aux, hsv_planes );

    out.resize( 180 / this->bindw + (256 % this->bindw != 0 ? 1 : 0) );
    fill(out.begin(), out.end(), 0.0);

    for ( int r = 0; r < in[i].rows; r++ ){
        for ( int c = 0; c < in[i].cols; c++ ){
            int bh = hsv_planes[0].at<uchar>(r, c) / this->bindw;
            out[bh] += 1.0;
        }
    }
    
    if ( this->normalize ){
        
        float total = 0.0;
        for ( uint i = 0; i < out.size(); i++ ){
            total += out[i];
        }
        
        for ( uint i = 0; i < out.size(); i++ ){
            out[i] /= total;
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

/*****************************************************************************
 *                         Motion Feature Extractor                          *
 *****************************************************************************/

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
        
void motion_fe::extract(vector<Mat>& in, int i, vector<float>& out,
                        anonadado::instance* instance)
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

/*****************************************************************************
 *                          Focus Feature Extractor                          *
 *****************************************************************************/

focus_fe::focus_fe()
{
    //TODO
}
        
void focus_fe::extract(vector<Mat>& in, int i, vector<float>& out,
                       anonadado::instance* instance)
{
    //TODO
}

void focus_fe::read(const rapidjson::Value& json)
{
    //TODO
}

void focus_fe::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

/*****************************************************************************
 *                   Specular Reflection Feature Extractor                   *
 *****************************************************************************/

specular_reflection_fe::specular_reflection_fe()
{
    this->detection = new threshold_srd();
}

specular_reflection_fe::specular_reflection_fe(
                                    specular_reflection_detection* d)
{
    this->detection = d;
}

int specular_reflection_fe::flood_fill(Mat& img, int r, int c)
{
    int ret = 0;
    queue< pair<int, int> > q;
    q.push(make_pair(r, c));
    
    while( !q.empty() ){
        pair<int, int> next = q.front();
        q.pop();
        
        int r = next.first;
        int c = next.second;
        
        if ( img.at<uchar>(r, c) == 0 ){
            continue;
        }
        
        img.at<uchar>(r, c) = 0;
        ret++;
        
        for (int dr=-1; dr<2; dr++){
            for (int dc=-1; dc<2; dc++){
                int nr = r + dr;
                int nc = c + dc;

                if ( 0 <= nr && nr < img.rows && 0 <= nc && nc < img.cols &&
                     img.at<uchar>(nr, nc) != 0 )
                {
                    q.push(make_pair(nr, nc));
                }
            }
        }
    }

    return ret;
}

void specular_reflection_fe::extract(vector<Mat>& in, int i,
                                     vector<float>& out,
                                     anonadado::instance* instance)
{
    Mat sr;
    int sum = 0;
    int largest = 0;
    int num_blobs = 0;

    this->detection->detect(in[i], sr);

    for (int r = 0; r < sr.rows; r++){
        for (int c = 0; c < sr.cols; c++){
            if ( sr.at<uchar>(r, c) != 0 ){
                int next_size = this->flood_fill(sr, r, c);
                sum += next_size;
                largest = max(largest, next_size);
                num_blobs++;
            }
        }
    }

    out.clear();
    out.push_back(num_blobs);
    out.push_back(((float)sum) / (sr.rows * sr.cols));
    out.push_back(((float)largest) / (sr.rows * sr.cols));
}

void specular_reflection_fe::read(const rapidjson::Value& json)
{
    //TODO
}

void specular_reflection_fe::write(rapidjson::Value& json,
                                   rapidjson::Document& d)
{
    //TODO
}

/*****************************************************************************
 *                      Color Cascade Feature Extractor                      *
 *****************************************************************************/

color_cascade_fe::color_cascade_fe()
{
    this->levels = 1;
    this->hard = true;
    this->sr_detection = 0;
    this->inpainting = 0;
    this->nfeatures = this->num_features();
    this->features_width = 180 / this->nfeatures;
}

color_cascade_fe::color_cascade_fe(int levels, bool hard,
                                   specular_reflection_detection* sr_detection,
                                   img_inpaint* inpainting)
{
    this->levels = levels;
    this->hard = hard;
    this->sr_detection = sr_detection;
    this->inpainting = inpainting;
    this->nfeatures = this->num_features();
    this->features_width = 180 / this->nfeatures;
}

int color_cascade_fe::num_features()
{
    int ret = 0;
    int s=1;
    for (int l = 0; l < this->levels; l++)
    {
        s = s << 1;
        ret += s;
    }
    
    return ret;
}

void color_cascade_fe::extract(vector<Mat>& in, int i, vector<float>& out,
                               anonadado::instance* instance)
{
    Mat aux, dst;
    
    if ( this->sr_detection != 0 && this->inpainting != 0 )
    {
        Mat sr;
        this->sr_detection->detect(in[i], sr);
        this->inpainting->fill(in[i], sr, aux);
    } else {
        in[i].copyTo(aux);
    }
    
    cvtColor(aux, dst, CV_BGR2HSV);

    out.clear();
    for ( int nf = 0; nf < this->nfeatures; nf++ ){
        out.push_back(0.0);
    }
    
    if ( this->hard ){
        for (int r = 0; r < dst.rows; r++){
            for (int c = 0; c < dst.cols; c++){
                int f = dst.at<Vec3b>(r, c)[0] / this->features_width;
                out[f] += 1.0;
            }
        }
    } else {
        for (int color = 0; color < this->nfeatures; color++){
            for (int r = 0; r < dst.rows; r++){
                for (int c = 0; c < dst.cols; c++){
                    int f = dst.at<Vec3b>(r, c)[0] / this->features_width;
                    out[color] += abs(color - f);
                }
            }
            out[color] /= (float)(dst.rows * dst.cols);
        }
    }
}

void color_cascade_fe::read(const rapidjson::Value& json)
{
    //TODO
}

void color_cascade_fe::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

/*****************************************************************************
 *                           HSV Feature Extractor                           *
 *****************************************************************************/

hsv_fe::hsv_fe(specular_reflection_detection* sr)
{
    this->sr_detection = sr;
}

void hsv_fe::extract(vector<Mat>& in, int i, vector<float>& out,
                     anonadado::instance* instance)
{
    Mat dst, mask;
    Mat mean, stddev;
    
    cvtColor(in[i], dst, CV_BGR2HSV);
    if ( this->sr_detection != 0 ){
        this->sr_detection->detect(in[i], mask);
        mask = 255 - mask;
        meanStdDev(in[i], mean, stddev, mask);
    } else {
        meanStdDev(in[i], mean, stddev);
    }

    out.clear();
    for (int ch = 0; ch < 3; ch++ ){
        out.push_back(mean.at<double>(0, ch));
        out.push_back(stddev.at<double>(0, ch));
    }
}

void hsv_fe::read(const rapidjson::Value& json)
{
    //TODO
}

void hsv_fe::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

/*****************************************************************************
 *                   Closest Transition Feature Extractor                    *
 *****************************************************************************/

closest_transition_fe::closest_transition_fe()
{
}
        
void closest_transition_fe::extract(vector<Mat>& in, int i, vector<float>& out,
                                    anonadado::instance* instance)
{
    vector<int> step_index;
    int num_frames = instance->num_frames();
    int closest_left = 0;
    int closest_right = num_frames;
    
    instance->get_annotations("diagnosis_step", step_index);
    
    anonadado::annotation* a = 
            instance->get_active_annotation(step_index[0], i);
    anonadado::choice_feature* current_step =
                            (anonadado::choice_feature*)a->get_feature("step");
    
    // Left side
    for ( int f = i - 1; f >= 0; f-- ){
        anonadado::annotation* a = 
            instance->get_active_annotation(step_index[0], f);
        
        if ( !a ){
            continue;
        }
        
        anonadado::choice_feature* step_feature =
                        (anonadado::choice_feature*)a->get_feature("step");
        
        if ( step_feature->get_value() != current_step->get_value() ){
            closest_left = f;
            break;
        }
    }
    
    // Right side
    for ( int f = i + 1; f < num_frames; f++ ){
        anonadado::annotation* a = 
            instance->get_active_annotation(step_index[0], f);
        
        if ( !a ){
            continue;
        }
        
        anonadado::choice_feature* step_feature =
                        (anonadado::choice_feature*)a->get_feature("step");
        
        if ( step_feature->get_value() != current_step->get_value() ){
            closest_right = f;
            break;
        }
    }
    
    
    float range = abs(closest_right - closest_left) + 1;
    float d_left = abs(i - closest_left);
    float d_right = abs(i - closest_right);
    
    out.clear();
    out.push_back(d_left / range);
    out.push_back(d_right / range);
    out.push_back((float)min(d_left, d_right) / range);
}

void closest_transition_fe::read(const rapidjson::Value& json)
{
    //TODO
}

void closest_transition_fe::write(rapidjson::Value& json,
                                  rapidjson::Document& d)
{
    //TODO
}

/*****************************************************************************
 *                    Edges Summations Feature Extractor                     *
 *****************************************************************************/

edges_summations_fe::edges_summations_fe()
{
    //TODO
}
        
void edges_summations_fe::extract(vector<Mat>& in, int i, vector<float>& out,
                                  anonadado::instance* instance)
{
    //TODO
}

void edges_summations_fe::read(const rapidjson::Value& json)
{
    //TODO
}

void edges_summations_fe::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}
