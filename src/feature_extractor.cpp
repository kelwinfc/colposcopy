#include "feature_extractor.hpp"

/*****************************************************************************
 *                             Feature Extractor                             *
 *****************************************************************************/

feature_extractor::feature_extractor()
{
    // noop
}

void feature_extractor::extract_by_inst(anonadado::instance& inst, int i,
                                        vector<float>& out)
{
    vector<Mat> seq;
    Mat frame;
    
    inst.get_frame(i, frame);
    seq.push_back(frame);
    
    this->extract(seq, 0, out, &inst);
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

void feature_extractor::get_names(vector<string>& names)
{
    names.clear();
}

/*****************************************************************************
 *                        Identity Feature Extractor                         *
 *****************************************************************************/

identity_fe::identity_fe()
{
    // noop
}

void identity_fe::extract_by_inst(anonadado::instance& inst, int i,
                                  vector<float>& out)
{
    out.clear();
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

void identity_fe::get_names(vector<string>& names)
{
    names.clear();
}

/*****************************************************************************
 *                       Add Inverse Feature Extractor                       *
 *****************************************************************************/

add_inverse_fe::add_inverse_fe(feature_extractor* u)
{
    this->underlying_fe = u;
}

void add_inverse_fe::extract_by_inst(anonadado::instance& inst, int i,
                                     vector<float>& out)
{
    this->underlying_fe->extract_by_inst(inst, i, out);
    this->add_inverse(out);
}

void add_inverse_fe::extract(vector<Mat>& in, int i, vector<float>& out,
                             anonadado::instance* instance)
{
    this->underlying_fe->extract(in, i, out, instance);
    this->add_inverse(out);
}

void add_inverse_fe::read(const rapidjson::Value& json)
{
    //TODO
}

void add_inverse_fe::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

void add_inverse_fe::get_names(vector<string>& names)
{
    names.clear();
    this->underlying_fe->get_names(names);
    
    size_t n = names.size();
    for (size_t i = 0; i < n; i++ ){
        names.push_back("-" + names[i]);
    }
}

void add_inverse_fe::add_inverse(vector<float>& out)
{
    size_t n = out.size();
    for (size_t i = 0; i < n; i++ ){
        out.push_back(-out[i]);
    }
}

/*****************************************************************************
 *                          Merge Feature Extractor                          *
 *****************************************************************************/

merge_fe::merge_fe(vector<feature_extractor*>& fe_seq)
{
    this->fe_seq = fe_seq;
}

void merge_fe::extract_by_inst(anonadado::instance& inst, int i,
                               vector<float>& out)
{
    out.clear();
    for (uint k = 0; k < this->fe_seq.size(); k++){
        vector<float> next_features;
        this->fe_seq[k]->extract_by_inst(inst, i, next_features);
        out.insert(out.end(), next_features.begin(), next_features.end());
    }
}

void merge_fe::extract(vector<Mat>& in, int i, vector<float>& out,
                       anonadado::instance* instance)
{
    out.clear();
    for (uint k = 0; k < this->fe_seq.size(); k++){
        vector<float> next_features;
        this->fe_seq[k]->extract(in, i, next_features, instance);
        out.insert(out.end(), next_features.begin(), next_features.end());
    }
}

void merge_fe::read(const rapidjson::Value& json)
{
    //TODO
}

void merge_fe::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

void merge_fe::get_names(vector<string>& names)
{
    names.clear();
    for (uint k = 0; k < this->fe_seq.size(); k++){
        vector<string> next_names;
        this->fe_seq[k]->get_names(next_names);
        names.insert(names.end(), next_names.begin(), next_names.end());
    }
}

/*****************************************************************************
 *                   Merge Single-Frame Feature Extractor                    *
 *****************************************************************************/

merge_single_frame_fe::merge_single_frame_fe
                                        (vector<feature_extractor*>& fe_seq)
{
    this->fe_seq = fe_seq;
}

void merge_single_frame_fe::extract_by_inst(anonadado::instance& inst, int i,
                                            vector<float>& out)
{
    vector<Mat> seq;
    Mat frame;

    inst.get_frame(i, frame);
    seq.push_back(frame);
    this->extract(seq, 0, out, &inst);
}

void merge_single_frame_fe::extract(vector<Mat>& in, int i, vector<float>& out,
                                    anonadado::instance* instance)
{
    out.clear();
    for (uint k = 0; k < this->fe_seq.size(); k++){
        vector<float> next_features;
        this->fe_seq[k]->extract(in, i, next_features, instance);
        out.insert(out.end(), next_features.begin(), next_features.end());
    }
}

void merge_single_frame_fe::read(const rapidjson::Value& json)
{
    //TODO
}

void merge_single_frame_fe::write(rapidjson::Value& json,
                                  rapidjson::Document& d)
{
    //TODO
}

void merge_single_frame_fe::get_names(vector<string>& names)
{
    names.clear();
    for (uint k = 0; k < this->fe_seq.size(); k++){
        vector<string> next_names;
        this->fe_seq[k]->get_names(next_names);
        names.insert(names.end(), next_names.begin(), next_names.end());
    }
}

/*****************************************************************************
 *                  Selective HSV-Channel Feature Extractor                  *
 *****************************************************************************/

selective_hsv_channel_histogram_fe::selective_hsv_channel_histogram_fe()
{
    this->bindw = 10;
    this->normalize = true;
    this->use[0] = this->use[1] = this->use[2] = 0;
    this->name = "";
    this->initialize();
}

selective_hsv_channel_histogram_fe::selective_hsv_channel_histogram_fe(
                                                    int bindw, bool normalize,
                                                    int use_ch[3],
                                                    string name)
{
    this->bindw = bindw;
    this->normalize = normalize;
    this->use[0] = use_ch[0];
    this->use[1] = use_ch[1];
    this->use[2] = use_ch[2];
    this->name = name;
    this->initialize();
}

void selective_hsv_channel_histogram_fe::initialize()
{
    this->hist_range[0] = 180;
    this->hist_range[1] = 256;
    this->hist_range[2] = 256;
    
    this->hist_size[0] = 180 / this->bindw + (180 % this->bindw != 0 ? 1 : 0);
    this->hist_size[1] = 256 / this->bindw + (256 % this->bindw != 0 ? 1 : 0);
    this->hist_size[2] = 256 / this->bindw + (256 % this->bindw != 0 ? 1 : 0);
    
    this->hist_shift[0] = 0;
    this->hist_shift[1] = 0;
    this->hist_shift[2] = 0;
    
    this->total = 0;
    
    for (int ch=0; ch<3; ch++){
        this->hist_size[ch] *= this->use[ch];
        
        if ( ch > 0 ){
            this->hist_shift[ch] = this->hist_shift[ch - 1] + 
                                   this->hist_size[ch - 1];
        }
        
        this->total += this->hist_size[ch];
    }
}

float euclidean_d2(int x0, int y0, int x1, int y1)
{
    float diffx = x1 - x0;
    float diffy = y1 - y0;
    
    return diffx * diffx + diffy * diffy;
}

void draw_mask(Mat& src, Mat& dst, float ratio)
{
    int cr = src.rows / 2;
    int cc = src.cols / 2;
    float threshold = ratio * min(cr, cc);
    threshold *= threshold;

    dst = Mat::zeros(src.rows, src.cols, CV_8UC3);
    for ( int r = 0; r < src.rows; r++ ){
        for ( int c = 0; c < dst.cols; c++ ){

            if ( euclidean_d2(cr, cc, r, c) > threshold ){
                continue;
            }
            
            dst.at<Vec3b>(r, c) = Vec3b(255, 255, 255);
        }
    }
    
    dst = cv::min(src, dst);
}

void selective_hsv_channel_histogram_fe::extract(vector<Mat>& in, int i,
                                                 vector<float>& out,
                                                 anonadado::instance* instance)
{
    float ratio = 0.75;
    int cr = in[i].rows / 2;
    int cc = in[i].cols / 2;
    float threshold = ratio * min(cr, cc);
    threshold *= threshold;
    
    Mat aux;
    vector<Mat> hsv_planes;
    
    cvtColor(in[i], aux, CV_BGR2HSV);
    split( aux, hsv_planes );
    
    out.resize(total);
    fill(out.begin(), out.end(), 0.0);
    /*
    Mat mask;
    draw_mask(in[i], mask, 0.75);
    imshow("src", in[i]);
    imshow("mask", mask);
    waitKey(0);
    */
    for (int ch=0; ch<3; ch++){
        if ( this->use[ch] ){
            Mat dst;
            hsv_planes[ch].copyTo(dst);
            //medianBlur(dst, dst, 3);
            
            for ( int r = 0; r < in[i].rows; r++ ){
                for ( int c = 0; c < in[i].cols; c++ ){
                    if ( euclidean_d2(cr, cc, r, c) > threshold ){
                        continue;
                    }
                    
                    int bh = dst.at<uchar>(r, c) / this->bindw;
                    out[this->hist_shift[ch] + bh] += 1.0;
                }
            }
            
            if ( this->normalize ){
                float norm_total = 0.0;
                for (uint k = this->hist_shift[ch];
                     k < this->hist_shift[ch] + this->hist_size[ch];
                     k++)
                {
                    /*
                    if ( out[k] < (1.0 / this->hist_range[ch]) * 
                                   in[i].rows * in[i].cols ){
                        out[k] = 0.0;
                    }
                    */
                    norm_total += out[k];
                }

                for (uint k = this->hist_shift[ch];
                     k < this->hist_shift[ch] + this->hist_size[ch];
                     k++)
                {
                    out[k] /= norm_total;
                }
            }
        }
    }
    /*
    cout << out.size() << endl;
    Mat hist;
    plot_histogram(out, hist);
    imshow("hist", hist);
    waitKey(0);
    */
}

void selective_hsv_channel_histogram_fe::read(const rapidjson::Value& json)
{
    //TODO
}

void selective_hsv_channel_histogram_fe::write(rapidjson::Value& json,
                                               rapidjson::Document& d)
{
    //TODO
}

void selective_hsv_channel_histogram_fe::get_names(vector<string>& names)
{
    names.clear();
    names.resize(this->total);
    
    for (uint k = 0; k < names.size(); k++){
        stringstream ss;
        string n;
        ss << this->name << k;
        ss >> n;
        names[k] = n;
    }
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

void motion_fe::extract_by_inst(anonadado::instance& inst, int i,
                                vector<float>& out)
{
    out.resize(this->width * this->height);
    fill(out.begin(), out.end(), 0.0);
    
    Mat current_image;
    inst.get_frame(i, current_image);
    GaussianBlur(current_image, current_image, Size(3,3), 0.3);
    
    for ( int k = max(0, i - this->w - 1);
          k < min(i + this->w + 1, (int)inst.num_frames());
          k++ )
    {
        if ( k == i )
            continue;
        
        Mat next_image;
        inst.get_frame(k, next_image);
        GaussianBlur(next_image, next_image, Size(3,3), 0.3);
        
        for ( int r = 0; r < next_image.rows; r++ ){
            int fr = r / (next_image.rows / this->height);
            for ( int c = 0; c < next_image.cols; c++ ){
                int fc = c / (next_image.cols / this->width);
                
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

void motion_fe::get_names(vector<string>& names)
{
    names.clear();
    names.resize(this->width * this->height);
    
    for (uint k = 0; k < names.size(); k++){
        stringstream ss;
        string n;
        ss << "motion_" << k;
        ss >> n;
        names.push_back(n);
    }
}

/*****************************************************************************
 *                          Focus Feature Extractor                          *
 *****************************************************************************/

focus_fe::focus_fe()
{
    //TODO
}

void focus_fe::extract_by_inst(anonadado::instance& inst, int i,
                               vector<float>& out)
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

void focus_fe::get_names(vector<string>& names)
{
    names.clear();
    //TODO
}

/*****************************************************************************
 *                   Specular Reflection Feature Extractor                   *
 *****************************************************************************/

specular_reflection_fe::specular_reflection_fe()
{
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
    
    if ( img.at<uchar>(r, c) == 0 ){
        return ret;
    }

    img.at<uchar>(r, c) = 0;

    while( !q.empty() ){
        pair<int, int> next = q.front();
        q.pop();
        
        int r = next.first;
        int c = next.second;
        
        ret++;
        
        for (int dr=-1; dr<2; dr++){
            for (int dc=-1; dc<2; dc++){
                int nr = r + dr;
                int nc = c + dc;

                if ( 0 <= nr && nr < img.rows && 0 <= nc && nc < img.cols &&
                     img.at<uchar>(nr, nc) != 0 )
                {
                    img.at<uchar>(nr, nc) = 0;
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
    
    if ( this->detection ){
        this->detection->detect(in[i], sr);
    } else {
        sr = Mat::zeros(in[i].rows, in[i].cols, CV_8U);
    }

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

void specular_reflection_fe::get_names(vector<string>& names)
{
    names.clear();
    names.push_back("sr::num_blobs");
    names.push_back("sr::ratio");
    names.push_back("sr::largest");
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
    this->features_width = 180 / this->nfeatures + 1;
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

void color_cascade_fe::get_names(vector<string>& names)
{
    names.clear();
    names.resize(this->nfeatures);
    
    for (uint k = 0; k < names.size(); k++){
        stringstream ss;
        string n;
        ss << "color_cascade_" << k;
        ss >> n;
        names[k] = n;
    }
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

void hsv_fe::get_names(vector<string>& names)
{
    names.clear();
    names.push_back("hsv::h_mean");
    names.push_back("hsv::h_stddev");
    names.push_back("hsv::s_mean");
    names.push_back("hsv::s_stddev");
    names.push_back("hsv::v_mean");
    names.push_back("hsv::v_stddev");
}

/*****************************************************************************
 *                   Closest Transition Feature Extractor                    *
 *****************************************************************************/

closest_transition_fe::closest_transition_fe()
{
}

void closest_transition_fe::extract_by_inst(anonadado::instance& inst, int i,
                                            vector<float>& out)
{
    vector<int> step_index;
    int num_frames = inst.num_frames();
    int closest_left = 0;
    int closest_right = num_frames;
    
    inst.get_annotations("diagnosis_step", step_index);
    anonadado::annotation* a = inst.get_active_annotation(step_index[0], i);
    
    if ( !a ){
        out.clear();
        out.push_back(1.0);
        out.push_back(1.0);
        out.push_back(1.0);
        return;
    }
    
    anonadado::choice_feature* current_step =
                            (anonadado::choice_feature*)a->get_feature("step");
    
    // Left side
    for ( int f = i - 1; f >= 0; f-- ){
        anonadado::annotation* a = 
            inst.get_active_annotation(step_index[0], f);
        
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
        anonadado::annotation* a = inst.get_active_annotation(step_index[0], 
                                                              f);
        
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

void closest_transition_fe::extract(vector<Mat>& in, int i, vector<float>& out,
                                    anonadado::instance* instance)
{
    this->extract_by_inst(*instance, i, out);
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

void closest_transition_fe::get_names(vector<string>& names)
{
    names.clear();
    names.push_back("transition::left");
    names.push_back("transition::right");
    names.push_back("transition::closest");
}

/*****************************************************************************
 *                    Edges Summations Feature Extractor                     *
 *****************************************************************************/

edges_summations_fe::edges_summations_fe(
                                specular_reflection_detection* sr_detection,
                                img_inpaint* inpainting)
{
    this->sr_detection = sr_detection;
    this->inpainting = inpainting;
}
        
void edges_summations_fe::extract(vector<Mat>& in, int i, vector<float>& out,
                                  anonadado::instance* instance)
{
    Mat aux, gray, edges;

    if ( this->sr_detection != 0 && this->inpainting != 0 )
    {
        Mat sr;
        this->sr_detection->detect(in[i], sr);
        this->inpainting->fill(in[i], sr, aux);
    } else {
        in[i].copyTo(aux);
    }

    Mat aux_roi = aux(Rect(10, 10, aux.cols - 20, aux.rows - 20));

    cvtColor(aux_roi, gray, CV_BGR2GRAY);
    GaussianBlur(gray, gray, Size(5, 5), 3.0);
    Canny(gray, edges, 50, 100, 3);

    out.clear();
    out.push_back(sum(edges)[0]);
}

void edges_summations_fe::read(const rapidjson::Value& json)
{
    //TODO
}

void edges_summations_fe::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

void edges_summations_fe::get_names(vector<string>& names)
{
    names.clear();
    names.push_back("edges");
}
