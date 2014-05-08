#include "diagnosis_phase.hpp"

/*****************************************************************************
 *                         Diagnosis Phase Detector                          *
 *****************************************************************************/

diagnosis_phase_detector::phase diagnosis_phase_detector::string_to_phase(
                                                                    string s)
{
    if ( s == "plain" ){
        return diagnosis_plain;
    } else if ( s == "green" ){
        return diagnosis_green;
    } else if ( s == "hinselmann" ){
        return diagnosis_hinselmann;
    } else if ( s == "schiller" ){
        return diagnosis_schiller;
    } else if ( s == "transition" ){
        return diagnosis_transition;
    } else {
        return diagnosis_unknown;
    }
}

diagnosis_phase_detector::diagnosis_phase_detector()
{
    
}

void diagnosis_phase_detector::read(string filename)
{
    
}

void diagnosis_phase_detector::write(string filename)
{
    
}

void diagnosis_phase_detector::train(vector<Mat>& src, vector<phase>& labels)
{
    
}

float diagnosis_phase_detector::eval(vector<Mat>& src, vector<phase>& labels)
{
    int n = labels.size();
    uint f = 0;

    if ( n == 0 ){
        return 0.0;
    }

    vector<phase> output;
    this->detect(src, output);

    for ( int i = 0; i < n; i++ ){
        if ( output[i] != labels[i] ){
            f++;
        }
    }
    
    return ((float)f) / labels.size();
}

void diagnosis_phase_detector::detect(vector<Mat>& src, vector<phase>& dst)
{
    vector<Mat>::iterator it, end;

    dst.clear();
    it = src.begin();
    end = src.end();
    
    for ( ; it != end; ++it ){
        dst.push_back(diagnosis_unknown);
    }
}

float diagnosis_phase_detector::get_confussion_matrix(vector<Mat>& src,
                 vector<phase>& labels, map< pair<phase, phase>, int>& matrix)
{
    phase steps[] = {
                     diagnosis_green,
                     diagnosis_hinselmann,
                     diagnosis_plain,
                     diagnosis_schiller,
                     diagnosis_transition,
                     diagnosis_unknown
                    };
    int num_steps = 6;

    matrix.clear();
    for ( int i=0; i<num_steps; i++ ){
        for ( int j=0; j<num_steps; j++ ){
            matrix[ make_pair(steps[i],steps[j]) ] = 0;
        }
    }

    vector<phase> output;
    this->detect(src, output);

    int f = 0;
    for ( uint i=0; i<src.size(); i++ ){
        pair<phase, phase> p = make_pair( labels[i], output[i] );
        matrix[p] = matrix[p] + 1;
        if ( labels[i] != output[i] ){
            f++;
        }
    }

    return ((float)f) / ((float)output.size());
}

float diagnosis_phase_detector::print_confussion_matrix(vector<Mat>& src,
                                                        vector<phase>& labels)
{
    map< pair<diagnosis_phase_detector::phase,
              diagnosis_phase_detector::phase>, int> matrix;
    
    phase steps[] = {
         diagnosis_unknown,
         diagnosis_transition,
         diagnosis_plain,
         diagnosis_green,
         diagnosis_hinselmann,
         diagnosis_schiller
    };
    
    const char* names[] =
        {
            "unknown   ",
            "transition",
            "plain     ",
            "green     ",
            "hinselmann",
            "schiller  "
        };

    int num_steps = 6;
    
    float ret = this->get_confussion_matrix(src, labels, matrix);

    int max_v = 0;
    for ( int i=0; i<num_steps; i++ ){
        for ( int j=0; j<num_steps; j++ ){
            max_v = max(max_v, matrix[make_pair(steps[i], steps[j])]);
        }
    }
    int n_chars = num_chars(max_v);
    
    for ( int i=0; i<num_steps; i++ ){
        printf("%s ", names[i]);
        for ( int j=0; j<num_steps; j++ ){
            string frame_s = spaced_d(matrix[make_pair(steps[i], steps[j])],
                                      n_chars);
            cout << frame_s << " ";
        }
        printf("\n");
    }

    return ret;
}

Vec3b color_by_phase(diagnosis_phase_detector::phase p)
{
    if ( p == diagnosis_phase_detector::diagnosis_green ){
        return Vec3b(0, 255, 0);
    } else if ( p == diagnosis_phase_detector::diagnosis_hinselmann ){
        return Vec3b(255, 255, 255);
    } else if ( p == diagnosis_phase_detector::diagnosis_plain ){
        return Vec3b(0, 0, 255);
    } else if ( p == diagnosis_phase_detector::diagnosis_schiller ){
        return Vec3b(0, 64, 128);
    } else if ( p == diagnosis_phase_detector::diagnosis_transition ){
        return Vec3b(128, 128, 128);
    } else if ( p == diagnosis_phase_detector::diagnosis_unknown ){
        return Vec3b(0, 0, 0);
    }
    return Vec3b(0, 0, 0);
}

void diagnosis_phase_detector::visualize(vector<Mat>& src,
                                         vector<phase>& labels,
                                         Mat& dst,
                                         int rows_by_frame, int cols_by_frame
                                        )
{
    vector<phase> output;
    uint n = src.size();
    
    this->detect(src, output);
    
    dst = Mat::zeros(rows_by_frame * 3, n * cols_by_frame, CV_8UC3);
    
    for ( uint i = 0; i < n; i++ ){
        for ( int r = 0; r < rows_by_frame; r++ ){
            for ( int c = 0; c < cols_by_frame; c++ ){
                dst.at<Vec3b>(r, i * cols_by_frame + c) =
                    color_by_phase(labels[i]);
                dst.at<Vec3b>(rows_by_frame + r, i * cols_by_frame + c) =
                    color_by_phase(output[i]);

                if ( labels[i] != output[i] ){
                    dst.at<Vec3b>(2 * rows_by_frame + rows_by_frame/2 + r/2,
                                  i * cols_by_frame + c) = Vec3b(0, 0, 255);
                }
            }
        }
    }
}

void diagnosis_phase_detector::visualize(vector<Mat>& src,
                                         vector<phase>& labels,
                                         string filename,
                                         int rows_by_frame, int cols_by_frame)
{
    Mat dst;
    this->visualize(src, labels, dst, rows_by_frame, cols_by_frame);
    imwrite(filename.c_str(), dst);
}

/*****************************************************************************
 *                 Histogram-Based Diagnosis Phase Detector                  *
 *****************************************************************************/

histogram_based_dpd::histogram_based_dpd()
{
    this->max_error = 0.01;
    this->bindw = 5;
    this->max_samples = 20;
}

void histogram_based_dpd::read(string filename)
{
    FILE * pFile = fopen (filename.c_str(), "r");
    rapidjson::FileStream is(pFile);
    rapidjson::Document d;
    d.ParseStream<0>(is);

    if ( !d.HasMember("type") || !d["type"].IsString() ||
         strcmp(d["type"].GetString(), "histogram_based") != 0 ||
         !d.HasMember("config") || !d["config"].IsObject()
       )
    {
        return;
    }

    const rapidjson::Value& config = d["config"];

    if ( config.HasMember("max_error") && config["max_error"].IsDouble() ){
        this->max_error = (float)config["max_error"].GetDouble();
    }
    
    if ( config.HasMember("bindw") && config["bindw"].IsDouble() ){
        this->bindw = (float)config["bindw"].GetDouble();
    }

    if ( config.HasMember("histogram") &&
         config["histogram"].IsArray() )
    {
        const rapidjson::Value& h = config["histogram"];

        for (rapidjson::SizeType i = 0; i < h.Size(); i++){
            const rapidjson::Value& nh = h[i];

            vector<float> next_h;
            if ( nh.IsArray() ){
                for (rapidjson::SizeType j = 0; j < nh.Size(); j++){
                    const rapidjson::Value& nv = nh[j];
                    next_h.push_back((float)nv.GetDouble());
                }
            }
            this->index_histogram.push_back(next_h);
        }
    }
    
    if ( config.HasMember("phase") &&
         config["phase"].IsArray() )
    {
        const rapidjson::Value& ph = config["phase"];
        
        for (rapidjson::SizeType i = 0; i < ph.Size(); i++){
            const rapidjson::Value& next_value = ph[i];
            this->index_phase.push_back((phase)next_value.GetInt());
        }
    }
    
    if ( config.HasMember("reliability") &&
         config["reliability"].IsArray() )
    {
        const rapidjson::Value& r = config["reliability"];

        for (rapidjson::SizeType i = 0; i < r.Size(); i++){
            const rapidjson::Value& next = r[i];
            this->index_reliability.push_back((float)next.GetDouble());
        }
    }

    if ( config.HasMember("threshold") &&
         config["threshold"].IsArray() )
    {
        const rapidjson::Value& t = config["threshold"];

        for (rapidjson::SizeType i = 0; i < t.Size(); i++){
            const rapidjson::Value& next = t[i];
            this->index_threshold.push_back((float)next.GetDouble());
        }
    }
    
    fclose(pFile);
}

void histogram_based_dpd::write(string filename)
{
    FILE * pFile = fopen(filename.c_str(), "w");
    rapidjson::Document d;
    d.SetObject();

    d.AddMember("type", "histogram_based", d.GetAllocator());

    rapidjson::Value config(rapidjson::kObjectType);

    config.AddMember("max_error", this->max_error, d.GetAllocator());
    config.AddMember("bindw", this->bindw, d.GetAllocator());

    rapidjson::Value phase_json(rapidjson::kArrayType);
    rapidjson::Value rel_json(rapidjson::kArrayType);
    rapidjson::Value thrs_json(rapidjson::kArrayType);
    rapidjson::Value h_json(rapidjson::kArrayType);
    
    for ( uint i = 0; i < this->index_histogram.size(); i++ ){
        rel_json.PushBack(this->index_reliability[i], d.GetAllocator());
        phase_json.PushBack(this->index_phase[i], d.GetAllocator());
        thrs_json.PushBack(this->index_threshold[i], d.GetAllocator());

        rapidjson::Value next_hist(rapidjson::kArrayType);
        for ( uint j = 0; j < this->index_histogram[i].size(); j++ ){
            next_hist.PushBack(this->index_histogram[i][j], d.GetAllocator());
        }
        
        h_json.PushBack(next_hist, d.GetAllocator());
    }
    
    config.AddMember("histogram", h_json, d.GetAllocator());
    config.AddMember("phase", phase_json, d.GetAllocator());
    config.AddMember("reliability", rel_json, d.GetAllocator());
    config.AddMember("threshold", thrs_json, d.GetAllocator());
    
    d.AddMember("config", config, d.GetAllocator());
    
    rapidjson::FileStream f(pFile);
    rapidjson::Writer<rapidjson::FileStream> writer(f);
    d.Accept(writer);

    fclose(pFile);
}
    
void histogram_based_dpd::detect(vector<Mat>& src, vector<phase>& dst)
{
    vector< vector<float> > hists;
    this->compute_histograms(src, hists);
    this->detect(hists, dst);
}

float histogram_based_dpd::eval(vector<Mat>& src, vector<phase>& labels)
{
    vector< vector<float> > hists;
    this->compute_histograms(src, hists);
    return this->eval(hists, labels);
}

void histogram_based_dpd::get_histogram(Mat& a, vector<float>& h)
{
    Mat aux;
    float max_vh = 0, max_vs;
    vector<Mat> hsv_planes;
    vector<float> hue, sat;
    
    cvtColor(a, aux, CV_BGR2HSV);
    split( aux, hsv_planes );
    
    hue.resize( 256 / this->bindw );
    fill(hue.begin(), hue.end(), 0.0);

    sat.resize( 256 / this->bindw );
    fill(sat.begin(), sat.end(), 0.0);
    
    for ( int r = 0; r < a.rows; r++ ){
        for ( int c = 0; c < a.cols; c++ ){
            int bh = hsv_planes[0].at<uchar>(r, c) / this->bindw;
            hue[bh] += 1.0;
            max_vh = max(max_vh, hue[bh]);

            int bs = hsv_planes[1].at<uchar>(r, c) / this->bindw;
            sat[bs] += 1.0;
            max_vs = max(max_vs, sat[bs]);
        }
    }
    
    for ( uint i = 0; i < hue.size(); i++ ){
        hue[i] /= max_vh;
        sat[i] /= max_vs;
    }
    
    h.clear();
    h.reserve( hue.size() + sat.size() );
    h.insert( h.end(), hue.begin(), hue.end() );
    //h.insert( h.end(), sat.begin(), sat.end() );
}

float histogram_based_dpd::distance(Mat& a, Mat& b)
{
    vector<float> ha, hb;

    this->get_histogram(a, ha);
    this->get_histogram(b, hb);
    
    return this->distance(ha, hb);
}

float histogram_based_dpd::distance(vector<float>& ha,
                                    vector<float>& hb)
{
    float ret = 0.0;
    float total = 0.0;
    
    for ( uint i = 0; i < ha.size(); i++ ){
        ret += min(ha[i], hb[i]);
        total += max(ha[i], hb[i]);
    }

    return 1.0 - ret / total;
}

float histogram_based_dpd::compute_threshold(vector<Mat>& src,
                                             vector<phase>& labels,
                                             vector< vector<float> >& h,
                                             int index)
{
    vector< pair<float, int> > q;
    uint n = labels.size();
    uint misclassified = 0;
    
    for ( uint i = 0; i < n; i++ ){
        if ( labels[i] != diagnosis_unknown ){
            misclassified++;
        }
    }
    
    for ( uint i = 0; i < n; i++ ){
        q.push_back( make_pair(this->distance(h[index], h[i]), i) );
    }
    sort(q.begin(), q.end());
    
    int best_threshold = 0;
    float min_error = (float)misclassified / (float)n;
    
    for ( uint i = 0; i < n; i++ ){
        if ( labels[q[i].second] != labels[index] ){
            misclassified++;
        } else {
            misclassified--;
        }

        float next_error = (float)misclassified / (float)n;
        if ( next_error < min_error ){
            best_threshold = i;
            min_error = next_error;
        }
    }
    
    return q[best_threshold].first;
}

void histogram_based_dpd::compute_thresholds(vector<Mat>& src,
                                             vector<phase>& labels,
                                             vector< vector<float> >& h,
                                             vector<float>& threshold)
{
    uint n = labels.size();
    
    threshold.resize(n);
    for ( uint i = 0; i < n; i++ ){
        threshold[i] = this->compute_threshold(src, labels, h, i);
    }
}

void histogram_based_dpd::compute_reliability(vector<Mat>& src,
                                              vector<phase>& labels,
                                              vector< vector<float> >& h,
                                              vector<float>& threshold,
                                              vector<float>& reliability)
{
    int t=0,f=0;
    uint n = labels.size();

    reliability.resize(n);
    fill(reliability.begin(), reliability.end(), 0.0);
    
    for ( uint index=0; index<n; index++ ){
        for ( uint i=0; i<n; i++ ){
            phase predicted = diagnosis_unknown;
            if ( this->distance(h[index], h[i]) <= threshold[index] ){
                predicted = labels[index];
            }
            
            if ( predicted == labels[i] ){
                t++;
            } else {
                f++;
            }
        }

        reliability[index] = ((float)t) / ((float)(t + f));
    }
}

void histogram_based_dpd::add_to_index(vector<float>& hist,
                                       phase label,
                                       float threshold,
                                       float reliability)
{
    this->index_histogram.push_back(hist);
    this->index_phase.push_back(label);
    this->index_threshold.push_back(threshold);
    this->index_reliability.push_back(reliability);
}

void histogram_based_dpd::remove_last()
{
    this->index_histogram.pop_back();
    this->index_phase.pop_back();
    this->index_threshold.pop_back();
    this->index_reliability.pop_back();
}

float histogram_based_dpd::eval(vector< vector<float> >& src,
                                vector<phase>& labels)
{
    int n = labels.size();
    uint f = 0;

    if ( n == 0 ){
        return 0.0;
    }

    vector<phase> output;
    this->detect(src, output);

    for ( int i = 0; i < n; i++ ){
        if ( output[i] != labels[i] ){
            f++;
        }
    }

    return ((float)f) / labels.size();
}

void histogram_based_dpd::detect(vector< vector<float> >& src,
                                 vector<phase>& dst)
{
    uint n = src.size();
    for ( uint i = 0; i < n; i++ ){
        
        map<phase, float> r;
        
        r[diagnosis_plain] = 0.0;
        r[diagnosis_green] = 0.0;
        r[diagnosis_hinselmann] = 0.0;
        r[diagnosis_schiller] = 0.0;
        r[diagnosis_transition] = 0.0;
        
        bool has = false;
        for ( uint k = 0; k < this->index_histogram.size(); k++ ){
            float next_distance = distance(this->index_histogram[k], src[i]);
            if ( next_distance <= this->index_threshold[k] ){
                r[this->index_phase[k]] += this->index_reliability[k];
                has = true;
            }
        }
        
        if ( has ){
            phase best_phase = diagnosis_unknown;
            float best_r = 0.0;
            map<phase, float>::iterator it;
            for ( it = r.begin(); it != r.end(); ++it ){
                if ( it->second > best_r ){
                    best_phase = it->first;
                    best_r = it->second;
                }
            }
            dst.push_back(best_phase);
        } else {
            dst.push_back(diagnosis_unknown);
        }
    }
}

pair<int, float> histogram_based_dpd::best_frame(vector<Mat>& src,
                                                 vector<phase>& labels,
                                                 vector<bool>& indexed,
                                                 vector<vector<float> >& hists,
                                                 vector<float>& threshold,
                                                 vector<float>& reliability)
{
    float min_error = 1.0;
    int ret = 0;
    uint n = labels.size();

    for ( uint i = 0; i < n; i++ ){

        if ( indexed[i] ||
             (labels[i] == diagnosis_unknown &&
              labels[i] == diagnosis_transition )
           )
        {
            continue;
        }
        
        this->add_to_index(hists[i], labels[i], threshold[i], reliability[i]);
        float next_error = this->eval(hists, labels);
        
        if ( next_error <= min_error ){
            min_error = next_error;
            ret = i;
        }

        this->remove_last();
    }
    
    return make_pair(ret, min_error);
}

void histogram_based_dpd::compute_histograms(vector<Mat>& src,
                                             vector<vector<float> >& h)
{
    uint n = src.size();
    for ( uint i = 0; i < n; i++ ){
        vector<float> next_h;
        this->get_histogram(src[i], next_h);
        h.push_back(next_h);
    }
}

void histogram_based_dpd::get_target_frames(vector<Mat>& src,
                                            vector<phase>& labels,
                                            vector<Mat>& src_train,
                                            vector<phase>& labels_train)
{
    src_train.clear();
    labels_train.clear();
    uint n = src.size();

    for ( uint i = 0; i < n; i++ ){
        if ( labels[i] != diagnosis_unknown &&
             labels[i] != diagnosis_transition
           )
        {
            src_train.push_back(src[i]);
            labels_train.push_back(labels[i]);
        }
    }
}

void histogram_based_dpd::train(vector<Mat>& src, vector<phase>& labels)
{
    #if __COLPOSCOPY_VERBOSE
        cout << "Training\n";
    #endif
    
    vector<Mat> src_train;
    vector<phase> labels_train;

    this->get_target_frames(src, labels, src_train, labels_train);
    
    vector<bool> indexed;
    vector<float> threshold, reliability;
    vector< vector<float> > hists;
    
    int n = labels_train.size();
    float current_error = 1.0;
    float previous_error = -1.0;
    
    indexed.resize(n);
    fill(indexed.begin(), indexed.end(), false);
    
    this->compute_histograms(src_train, hists);

    #if __COLPOSCOPY_VERBOSE
        cout << "Histograms computed\n";
    #endif
    
    this->compute_thresholds(src_train, labels_train, hists, threshold);

    #if __COLPOSCOPY_VERBOSE
        cout << "Thresholds computed\n";
    #endif
    
    this->compute_reliability(src_train, labels_train, hists,
                              threshold, reliability);

    #if __COLPOSCOPY_VERBOSE
        cout << "Reliability computed\n";
        if ( system("rm results/phase_index/*.jpg") ){
            fprintf(stderr,
                    "Error: unable to rm  results/phase_index/*.jpg\n");
        }
    #endif
    
    while ( current_error > this->max_error &&
            this->index_histogram.size() < src_train.size() && 
            ( previous_error < 0.0 || previous_error > current_error ) &&
            (this->max_samples < 0 || (int)this->index_histogram.size() <
                                           this->max_samples)
          )
    {
        previous_error = current_error;
        
        pair<int, float> best = this->best_frame(src_train, labels_train,
                                                 indexed, hists, threshold,
                                                 reliability);

        if ( best.second < 0 ){
            break;
        }
        
        this->add_to_index(hists[best.first], labels_train[best.first],
                           threshold[best.first], reliability[best.first]);
        indexed[best.first] = true;
        current_error = best.second;

        #if __COLPOSCOPY_VERBOSE
            printf("Include (class %d) %s: %0.4f (total: %d)\n",
                   labels_train[best.first],
                   spaced_d(best.first, num_chars(src_train.size())).c_str(),
                   current_error,
                   (int)this->index_histogram.size()
                  );

            stringstream ss;
            string filename;
            ss <<  "results/phase_index/"
               << this->index_histogram.size() << ".jpg";
            ss >> filename;
            imwrite(filename, src_train[best.first]);
        #endif
    }

    #if __COLPOSCOPY_VERBOSE
        cout << "Done\n";
    #endif
}

/*****************************************************************************
 *                     Windowed Diagnosis Phase Detector                     *
 *****************************************************************************/

w_dpd::w_dpd()
{
    this->underlying_detector = 0;
    this->w = 0;
}

w_dpd::w_dpd(diagnosis_phase_detector* d, int w)
{
    this->underlying_detector = d;
    this->w = w;
}

void w_dpd::read(string filename)
{
    //TODO
}

void w_dpd::write(string filename)
{
    //TODO
}

void w_dpd::train(vector<Mat>& src, vector<phase>& labels)
{
    //TODO
}

float w_dpd::eval(vector<Mat>& src, vector<phase>& labels)
{
    return diagnosis_phase_detector::eval(src, labels);
}

void w_dpd::initialize_diagnosis_phase_map(map<phase, int>& h)
{
    h[diagnosis_green]      = 0;
    h[diagnosis_hinselmann] = 0;
    h[diagnosis_plain]      = 0;
    h[diagnosis_schiller]   = 0;
    h[diagnosis_unknown]    = 0;
}

diagnosis_phase_detector::phase get_highest(
                                map<diagnosis_phase_detector::phase, int>& h)
{
    diagnosis_phase_detector::phase ret =
        diagnosis_phase_detector::diagnosis_unknown;
    int max_ret = 0;
    
    map<diagnosis_phase_detector::phase, int>::iterator it;

    for ( it = h.begin(); it != h.end(); ++it ){
        if ( it->second > max_ret ){
            max_ret = it->second;
            ret = it->first;
        }
    }

    return ret;
}

void w_dpd::detect(vector<Mat>& src, vector<phase>& dst)
{
    if ( this->w == 0 || this->underlying_detector == 0 ){
        diagnosis_phase_detector::detect(src, dst);
    } else {
        uint n = src.size();
        dst.clear();
        
        map<diagnosis_phase_detector::phase, int> h;
        queue<diagnosis_phase_detector::phase> q;

        initialize_diagnosis_phase_map(h);

        vector<phase> dst_aux;
        this->underlying_detector->detect(src, dst_aux);
        
        for ( uint i = 0; i < n; i++ ){

            if ( q.size() > 0 && (int)q.size() >= this->w ){
                phase next = q.front();
                h[next]--;
                q.pop();
            }
            
            q.push(dst_aux[i]);
            h[dst_aux[i]]++;
            
            dst.push_back(get_highest(h));
        }
    }
}

/*****************************************************************************
 *                          Context Phase Detector                           *
 *****************************************************************************/

context_rule::context_rule(diagnosis_phase_detector::phase pre,
                           diagnosis_phase_detector::phase post,
                           diagnosis_phase_detector::phase replace_by)
{
    this->pre = pre;
    this->post = post;
    this->replace_by = replace_by;
}

diagnosis_phase_detector::phase context_rule::get_pre() const 
{
    return this->pre;
}

diagnosis_phase_detector::phase context_rule::get_post() const
{
    return this->post;
}

diagnosis_phase_detector::phase context_rule::get_replacement() const
{
    return this->replace_by;
}

bool operator<(context_rule const& a, context_rule const& b)
{
    return a.get_pre() < b.get_pre() ||
           (a.get_pre() == b.get_pre() && a.get_post() < b.get_post() );
}

context_dpd::context_dpd()
{
    this->underlying_detector = 0;
}

context_dpd::context_dpd(diagnosis_phase_detector* d)
{
    this->underlying_detector = d;

    this->rules.insert( context_rule(diagnosis_plain, diagnosis_hinselmann,
                                     diagnosis_plain) );
    this->rules.insert( context_rule(diagnosis_green, diagnosis_schiller,
                                     diagnosis_plain) );
    this->rules.insert( context_rule(diagnosis_green, diagnosis_hinselmann,
                                     diagnosis_plain) );
}

void context_dpd::read(string filename)
{
    //TODO
}

void context_dpd::write(string filename)
{
    //TODO
}

void context_dpd::train(vector<Mat>& src, vector<phase>& labels)
{
    
}

float context_dpd::eval(vector<Mat>& src, vector<phase>& labels)
{
    return diagnosis_phase_detector::eval(src, labels);
}

diagnosis_phase_detector::phase context_dpd::is_ok(set<phase>& seen, phase p)
{
    set<context_rule>::iterator it;
    for ( it = this->rules.begin(); it != this->rules.end(); ++it ){
        if ( it->get_post() == p ){
            if ( seen.find(it->get_pre()) == seen.end() ){
                return it->get_replacement();
            }
        }
    }

    return p;
}

void context_dpd::detect(vector<Mat>& src, vector<phase>& dst)
{
    if ( this->underlying_detector == 0 ){
        diagnosis_phase_detector::detect(src, dst);
    } else {
        uint n = src.size();
        dst.clear();
        
        vector<phase> dst_aux;
        this->underlying_detector->detect(src, dst_aux);

        set<phase> seen;
        
        for ( uint i = 0; i < n; i++ ){
            phase next = is_ok(seen, dst_aux[i]);
            dst.push_back(next);
            
            if ( next == dst_aux[i] ){
                seen.insert(next);
            }
        }
    }
}

/*****************************************************************************
 *                          Unknown Phase Detector                           *
 *****************************************************************************/

unknown_removal_dpd::unknown_removal_dpd()
{
    this->underlying_detector = 0;
}

unknown_removal_dpd::unknown_removal_dpd(diagnosis_phase_detector* d)
{
    this->underlying_detector = d;
}

void unknown_removal_dpd::read(string filename)
{
    //TODO
}

void unknown_removal_dpd::write(string filename)
{
    //TODO
}

void unknown_removal_dpd::train(vector<Mat>& src, vector<phase>& labels)
{

}

float unknown_removal_dpd::eval(vector<Mat>& src, vector<phase>& labels)
{
    return diagnosis_phase_detector::eval(src, labels);
}

void unknown_removal_dpd::detect(vector<Mat>& src, vector<phase>& dst)
{
    if ( this->underlying_detector == 0 ){
        diagnosis_phase_detector::detect(src, dst);
    } else {
        uint n = src.size();
        dst.clear();

        vector<phase> dst_aux;
        this->underlying_detector->detect(src, dst_aux);

        phase last = diagnosis_unknown;
        
        for ( uint i = 0; i < n; i++ ){
            phase next = dst_aux[i];
            if ( next == diagnosis_unknown ){
                dst.push_back(last);
            } else {
                dst.push_back(next);
                last = next;
            }
        }
    }
}
