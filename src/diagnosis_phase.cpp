#include "diagnosis_phase.hpp"

using namespace colposcopy;

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
    FILE * pFile = fopen (filename.c_str(), "r");
    rapidjson::FileStream is(pFile);
    rapidjson::Document d;
    d.ParseStream<0>(is);
    this->read(d);
    fclose(pFile);
}

void diagnosis_phase_detector::read(const rapidjson::Value& json)
{
    // noop
}

void diagnosis_phase_detector::write(string filename)
{
    FILE * pFile = fopen(filename.c_str(), "w");
    rapidjson::Document d;
    d.SetObject();

    this->write(d, d);
    
    rapidjson::FileStream f(pFile);
    rapidjson::Writer<rapidjson::FileStream> writer(f);
    d.Accept(writer);
    
    fclose(pFile);
}

void diagnosis_phase_detector::write(rapidjson::Value& json,
                                     rapidjson::Document& d)
{
    // noop
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

void diagnosis_phase_detector::get_equidistant_frames(
                                              vector<string>& videos,
                                              vector<Mat>& images,
                                              vector<phase>& labels,
                                              int frames_per_phase,
                                              int rows, int cols)
{
    images.clear();
    labels.clear();

    /* For each video get equidistant frames */
    for (size_t v = 0; v < videos.size(); v++){
        anonadado::instance inst;
        inst.read(videos[v]);
        vector<int> step_index;
        vector<int> roi_index;
        
        inst.get_annotations("diagnosis_step", step_index);
        inst.get_annotations("roi", roi_index);
        
        int num_frames = inst.num_frames();
        
        map< phase, vector<int> > frames;
        for ( int f = 0; f < num_frames; f++ ){
            anonadado::annotation* a = 
                inst.get_active_annotation(step_index[0], f);
            anonadado::annotation* roi = 
                inst.get_active_annotation(roi_index[0], f);
            
            if ( !a || !roi )
                continue;
            
            anonadado::choice_feature* step_feature =
                            (anonadado::choice_feature*)a->get_feature("step");
            phase next_phase = diagnosis_phase_detector::string_to_phase(
                                                    step_feature->get_value());
            
            if ( next_phase == diagnosis_phase_detector::diagnosis_transition )
                continue;
            
            if ( frames.find(next_phase) == frames.end() ){
                vector<int> n;
                frames[next_phase] = n;
            }
            
            frames[next_phase].push_back(f);
        }
        
        phase phs[] = {
                       diagnosis_phase_detector::diagnosis_plain,
                       diagnosis_phase_detector::diagnosis_green,
                       diagnosis_phase_detector::diagnosis_hinselmann,
                       diagnosis_phase_detector::diagnosis_schiller
                      };
        
        for(int ph_index = 0; ph_index < 4; ph_index++){
            phase next_ph = phs[ph_index];
            size_t frames_to_retrieve = min((size_t)frames_per_phase,
                                            frames[next_ph].size());

            float frames_step = ((float)frames[next_ph].size()) / 
                                ((float)(frames_to_retrieve + 1));

            for (size_t f = 0; f < frames_to_retrieve; f++){
                int next_frame = frames[next_ph][(int)((f + 1) * frames_step)];
                
                anonadado::annotation* a = 
                    inst.get_active_annotation(step_index[0], next_frame);
                anonadado::annotation* roi = 
                    inst.get_active_annotation(roi_index[0], next_frame);

                if ( !a || !roi ){
                    continue;
                }
                
                Mat img, aux;
                inst.get_frame(next_frame, img);
                
                if ( !img.data ){
                    continue;
                }
                
                
                anonadado::bbox_feature* roi_feature =
                            (anonadado::bbox_feature*)roi->get_feature("roi");
                BBOX roi_value = roi_feature->get_value();
                
                Rect region_of_interest =
                    Rect(roi_value.first.first,
                         roi_value.first.second,
                         roi_value.second.first - roi_value.first.first,
                         roi_value.second.second - roi_value.first.second);
                
                resize(img, aux, Size(600, 474));
                aux.copyTo(img);
                
                img = img(region_of_interest);
                
                resize(img, aux, Size(rows, cols));
                
                images.push_back(aux);
                labels.push_back(next_ph);
            }
        }
    }
}

/*****************************************************************************
 *                 Histogram-Based Diagnosis Phase Detector                  *
 *****************************************************************************/

classifier_dpd::classifier_dpd()
{
    this->c = 0;
    this->delete_classifier = false;
}

classifier_dpd::~classifier_dpd()
{
    if ( this->c && this->delete_classifier ){
        delete this->c;
    }
}

void classifier_dpd::read(const rapidjson::Value& json)
{
    if ( !json.HasMember("type") || !json["type"].IsString() ||
         strcmp(json["type"].GetString(), "classifier") != 0 ||
         !json.HasMember("config") || !json["config"].IsObject()
       )
    {
        return;
    }

    const rapidjson::Value& config = json["config"];
    
    if ( config.HasMember("classifier") || config["classifier"].IsObject() ){
        this->c = classifier::from_json(config["classifier"]);
        
        if ( this->c != 0){
            this->delete_classifier = true;
        }
    }
}

void classifier_dpd::write(rapidjson::Value& json, rapidjson::Document& d)
{
    json.SetObject();
    json.AddMember("type", "classifier", d.GetAllocator());
    
    //TODO: write classifier
}

void classifier_dpd::detect(vector<Mat>& src, vector<phase>& dst)
{
    vector<int> aux;
    this->c->detect(src, aux);

    this->label_to_phase(aux, dst);
}

void classifier_dpd::train(vector<Mat>& src, vector<phase>& labels)
{
    vector<label> aux;
    this->phase_to_label(labels, aux);
    this->c->untrain();
    this->c->train(src, aux);
}

float classifier_dpd::eval(vector<Mat>& src, vector<phase>& labels)
{
    return diagnosis_phase_detector::eval(src, labels);
}

void classifier_dpd::set_classifier(classifier* cl)
{
    if ( this->c != 0 && this->delete_classifier ){
        delete this->c;
    }
    
    this->c = cl;
    this->delete_classifier = 0;
}

void classifier_dpd::phase_to_label(vector<phase>& in, vector<label>& out)
{
    out.clear();
    out.resize(in.size());
    for ( size_t i = 0; i < in.size(); i++ ){
        out[i] = (label)in[i];
    }
}

void classifier_dpd::label_to_phase(vector<label>& in, vector<phase>& out)
{
    out.clear();
    out.resize(in.size());
    for ( size_t i = 0; i < in.size(); i++ ){
        if ( in[i] == UNKNOWN ){
            out[i] = diagnosis_unknown;
        } else {
            out[i] = (phase)in[i];
        }
    }
}

/*****************************************************************************
 *                     Windowed Diagnosis Phase Detector                     *
 *****************************************************************************/

w_dpd::w_dpd()
{
    this->underlying_detector = 0;
    this->w = 0;
    this->delete_detector = false;
}

w_dpd::w_dpd(diagnosis_phase_detector* d, int w)
{
    this->underlying_detector = d;
    this->w = w;
    this->delete_detector = false;
}

w_dpd::~w_dpd()
{
    if ( this->delete_detector && this->underlying_detector != 0 ){
        delete this->underlying_detector;
    }
}
        

void w_dpd::read(const rapidjson::Value& json)
{
    if ( !json.HasMember("type") || !json["type"].IsString() ||
         strcmp(json["type"].GetString(), "classifier") != 0 ||
         !json.HasMember("config") || !json["config"].IsObject()
       )
    {
        return;
    }
    
    const rapidjson::Value& config = json["config"];
    
    if ( config.HasMember("underlying") || config["underlying"].IsObject() ){
        this->delete_detector = true;
        
        this->underlying_detector = from_json(config["underlying"]);
        
        if ( this->underlying_detector != 0){
            this->delete_detector= true;
        }
    }
    
    if ( config.HasMember("w") || config["w"].IsInt() ){
        this->w = config["w"].GetInt();
    }
}

void w_dpd::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}
void w_dpd::train(vector<Mat>& src, vector<phase>& labels)
{
    this->underlying_detector->train(src, labels);
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

context_rule::context_rule()
{
    
}

context_rule::context_rule(diagnosis_phase_detector::phase pre,
                           diagnosis_phase_detector::phase post,
                           diagnosis_phase_detector::phase replace_by
                          )
{
    this->pre        = pre;
    this->post       = post;
    this->replace_by = replace_by;
}

diagnosis_phase_detector::phase context_rule::is_ok(
                                    set<diagnosis_phase_detector::phase>& seen,
                                    diagnosis_phase_detector::phase p)
{
    return p;
}

at_least_once_before_cr::at_least_once_before_cr(
                                diagnosis_phase_detector::phase pre,
                                diagnosis_phase_detector::phase post,
                                diagnosis_phase_detector::phase replace_by
                               )
{
    this->pre        = pre;
    this->post       = post;
    this->replace_by = replace_by;
}

diagnosis_phase_detector::phase at_least_once_before_cr::is_ok(
                                    set<diagnosis_phase_detector::phase>& seen,
                                    diagnosis_phase_detector::phase p)
{
    if ( seen.find(this->pre) == seen.end() )
    {
        return this->replace_by;
    }
    
    return p;
}

never_before_cr::never_before_cr(diagnosis_phase_detector::phase pre,
                                 diagnosis_phase_detector::phase post,
                                 diagnosis_phase_detector::phase replace_by)
{
    this->pre        = pre;
    this->post       = post;
    this->replace_by = replace_by;
}
        
diagnosis_phase_detector::phase never_before_cr::is_ok(
                                    set<diagnosis_phase_detector::phase>& seen,
                                    diagnosis_phase_detector::phase p)
{
    if ( seen.find(this->pre) != seen.end() )
    {
        return this->replace_by;
    }
    
    return p;
}

never_after_cr::never_after_cr(diagnosis_phase_detector::phase pre,
                               diagnosis_phase_detector::phase post,
                               diagnosis_phase_detector::phase replace_by)
{
    this->pre        = pre;
    this->post       = post;
    this->replace_by = replace_by;
}
        
diagnosis_phase_detector::phase never_after_cr::is_ok(
                                    set<diagnosis_phase_detector::phase>& seen,
                                    diagnosis_phase_detector::phase p)
{
    if ( seen.find(this->pre) != seen.end() )
    {
        return this->replace_by;
    }
    
    return p;
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

context_dpd::context_dpd()
{
    this->underlying_detector = 0;
}

context_dpd::context_dpd(diagnosis_phase_detector* d)
{
    this->underlying_detector = d;

    this->rules.insert(new at_least_once_before_cr(diagnosis_green,
                                                   diagnosis_hinselmann,
                                                   diagnosis_plain) );
    this->rules.insert(new at_least_once_before_cr(diagnosis_green,
                                                   diagnosis_schiller,
                                                   diagnosis_plain) );
    this->rules.insert(new at_least_once_before_cr(diagnosis_green,
                                                   diagnosis_hinselmann,
                                                   diagnosis_plain) );
    this->rules.insert(new never_after_cr(diagnosis_green,
                                          diagnosis_plain,
                                          diagnosis_hinselmann) );
    
    /*
    
    this->rules.insert( context_rule(diagnosis_green, diagnosis_plain,
                                     diagnosis_hinselmann, false) );*/
}

context_dpd::~context_dpd()
{
    set<context_rule*>::iterator it=this->rules.begin(), end=this->rules.end();
    
    for ( ; it != end; ++it ){
        context_rule* next = *it;
        delete next;
    }
}

void context_dpd::read(const rapidjson::Value& json)
{
    //TODO
}

void context_dpd::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

void context_dpd::train(vector<Mat>& src, vector<phase>& labels)
{
    this->underlying_detector->train(src, labels);
}

float context_dpd::eval(vector<Mat>& src, vector<phase>& labels)
{
    return diagnosis_phase_detector::eval(src, labels);
}

diagnosis_phase_detector::phase context_dpd::is_ok(set<phase>& seen, phase p)
{
    set<context_rule*>::iterator it;
    for ( it = this->rules.begin(); it != this->rules.end(); ++it ){
        context_rule* next = *it;
        if ( next->get_post() == p && next->is_ok(seen, p) != p ){
            return next->get_replacement();
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

void unknown_removal_dpd::read(const rapidjson::Value& json)
{
    //TODO
}

void unknown_removal_dpd::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

void unknown_removal_dpd::train(vector<Mat>& src, vector<phase>& labels)
{
    this->underlying_detector->train(src, labels);
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

/*****************************************************************************
 *                           Binary Phase Detector                           *
 *****************************************************************************/

binary_dpd::binary_dpd()
{
    this->left = this->right = 0;
}

binary_dpd::binary_dpd(diagnosis_phase_detector* l,
                       diagnosis_phase_detector* r)
{
    this->left = l;
    this->right = r;
}

void binary_dpd::read(const rapidjson::Value& json)
{
    //TODO
}

void binary_dpd::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

void binary_dpd::train(vector<Mat>& src, vector<phase>& labels)
{
    if ( this->left != 0 ){
        this->left->train(src, labels);
    }
    
    if ( this->right != 0 ){
        this->right->train(src, labels);
    }
}

float binary_dpd::eval(vector<Mat>& src, vector<phase>& labels)
{
    return diagnosis_phase_detector::eval(src, labels);
}

void binary_dpd::detect(vector<Mat>& src, vector<phase>& dst)
{
    vector<phase> dst_left, dst_right;
    
    if ( this->left != 0 ){
        this->left->detect(src, dst_left);
    } else {
        this->diagnosis_phase_detector::detect(src, dst_left);
    }
    
    if ( this->right != 0 ){
        this->right->detect(src, dst_right);
    } else {
        this->diagnosis_phase_detector::detect(src, dst_right);
    }
    
    dst.resize(src.size());
    for ( size_t i = 0; i < src.size(); i++ ){
        if ( dst_left[i] == diagnosis_unknown ){
            dst[i] = dst_right[i];
        } else {
            dst[i] = dst_left[i];
        }
    }
}

/*****************************************************************************
 *                           Final Phase Detector                            *
 *****************************************************************************/

final_dpd::final_dpd()
{
    this->dpd_transition = 0;
    this->dpd_phase      = 0;
    this->mod_rate       = 1;
}

final_dpd::final_dpd(diagnosis_phase_detector* tr, diagnosis_phase_detector* ph,
                     uint mod_rate)
{
    this->dpd_transition = tr;
    this->dpd_phase      = ph;
    this->mod_rate       = mod_rate;
}
        
void final_dpd::read(const rapidjson::Value& json)
{
    //TODO
}

void final_dpd::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

void final_dpd::get_transition_labels(vector<Mat>& images,
                                      vector<phase>& labels,
                                      vector<Mat>& tr_images,
                                      vector<phase>& tr_labels)
{
    tr_images.clear();
    tr_labels.clear();
    
    for ( size_t i = 0; i < images.size(); i++ ){
        tr_images.push_back(images[i]);
        if ( labels[i] == diagnosis_transition ){
            tr_labels.push_back(diagnosis_transition);
        } else {
            tr_labels.push_back(diagnosis_unknown);
        }
    }
}

void final_dpd::get_phase_labels(vector<Mat>& images,
                                 vector<phase>& labels,
                                 vector<Mat>& ph_images,
                                 vector<phase>& ph_labels)
{
    ph_images.clear();
    ph_labels.clear();
    
    for ( size_t i = 0; i < images.size(); i++ ){
        if ( labels[i] != diagnosis_transition && 
             labels[i] != diagnosis_unknown && (i % (size_t)mod_rate == 0) )
        {
            ph_images.push_back(images[i]);
            ph_labels.push_back(labels[i]);
        }
    }
}

void final_dpd::train(vector<Mat>& src, vector<phase>& labels)
{
    vector<Mat> src_transition, src_phase;
    vector<phase> labels_transition, labels_phase;
    
//     this->get_transition_labels(src, labels,
//                                 src_transition, labels_transition);
    this->get_phase_labels(src, labels,
                           src_phase, labels_phase);
    
    //this->dpd_transition->train(src_transition, labels_transition);
    this->dpd_phase->train(src_phase, labels_phase);
}

float final_dpd::eval(vector<Mat>& src, vector<phase>& labels)
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

void final_dpd::extract_non_transition(vector<Mat>& src, vector<Mat>& dst,
                                       vector<phase>& transitions,
                                       vector<int>& mapping)
{
    mapping.resize(src.size());
    fill(mapping.begin(), mapping.end(), -1);
    
    for ( size_t i = 0; i < src.size(); i++ ){
        if ( transitions[i] != diagnosis_transition ){
            mapping[i] = (int)dst.size();
            dst.push_back(src[i]);
        }
    }
}

void final_dpd::detect(vector<Mat>& src, vector<phase>& dst)
{
    vector<phase> transition_out;
    vector<phase> phase_out;
    vector<Mat> src_phase;
    vector<int> mapping;
    
    this->dpd_transition->detect(src, transition_out);
    
    extract_non_transition(src, src_phase, transition_out, mapping);
    
    this->dpd_phase->detect(src_phase, phase_out);
    
    dst.resize(src.size());
    fill(dst.begin(), dst.end(), diagnosis_unknown);
    
    for ( size_t i = 0; i < src.size(); i++ ){
        if ( transition_out[i] == diagnosis_transition ){
            dst[i] = diagnosis_transition;
        } else {
            dst[i] = phase_out[mapping[i]];
        }
    }
}

/*****************************************************************************
 *                           Final Phase Detector                            *
 *****************************************************************************/


temporal_dpd::temporal_dpd()
{
    this->underlying = 0;
}

temporal_dpd::temporal_dpd(diagnosis_phase_detector* u)
{
    this->underlying = u;
}

void temporal_dpd::read(const rapidjson::Value& json)
{
    // TODO
}

void temporal_dpd::write(rapidjson::Value& json, rapidjson::Document& d)
{
    // TODO
}

void temporal_dpd::train(vector<Mat>& src, vector<phase>& labels)
{
    if ( this->underlying != 0 ){
        this->underlying->train(src, labels);
    }
}

float temporal_dpd::eval(vector<Mat>& src, vector<phase>& labels)
{
    return diagnosis_phase_detector::eval(src, labels);
}

void temporal_dpd::detect(vector<Mat>& src, vector<phase>& dst)
{
    vector<phase> intermidiate;
    this->underlying->detect(src, intermidiate);
    map<pair<int, phase> , int> knowledge;
    
    int result = this->dp(intermidiate, 0, diagnosis_plain, knowledge);
    this->traverse(intermidiate, knowledge, result, 0, diagnosis_plain, dst);
}

int temporal_dpd::dp(vector<phase>& sequence, int i, phase step,
                     map<pair<int, phase>, int>& knowledge)
{
    if ( i == sequence.size() ){
        return 0;
    }
    pair<int, phase> key = make_pair(i, step);
    
    if ( knowledge.find(key) != knowledge.end() ){
        return knowledge[key];
    }
    
    int ret = 0;
    
    // Transition or match
    if ( sequence[i] == diagnosis_transition || 
         sequence[i] == step ||
         sequence[i] == diagnosis_unknown
       )
    {
        ret = this->dp(sequence, i + 1, step, knowledge);
    }
    // Special case wit Plain and Hinselmann
    else if ( step == diagnosis_plain && 
              sequence[i] == diagnosis_hinselmann
            )
    {
        ret = min(this->dp(sequence, i + 1, step, knowledge),
                  this->dp(sequence, i, sequence[i], knowledge));
    }
    // Special case wit Hinselmann and Plain
    else if ( step == diagnosis_hinselmann && sequence[i] == diagnosis_plain )
    {
        ret = this->dp(sequence, i + 1, step, knowledge);
    }
    // Has next step
    else if ( this->has_next(step) ){
        ret = min(this->dp(sequence, i, this->next_step(step), knowledge),
                  1 + this->dp(sequence, i + 1, step, knowledge));
    }
    // Doesn't have next step
    else {
        ret = 1 + this->dp(sequence, i + 1, step, knowledge);
    }
    
    return knowledge[key] = ret;
}

bool temporal_dpd::has_next(phase step)
{
    return step != diagnosis_schiller;
}

diagnosis_phase_detector::phase temporal_dpd::next_step(phase step)
{
    switch (step){
        case diagnosis_plain:
            return diagnosis_green;
        case diagnosis_green:
            return diagnosis_hinselmann;
        case diagnosis_hinselmann:
            return diagnosis_schiller;
        default:
            return diagnosis_transition;
    }
}

void temporal_dpd::traverse(vector<phase>& sequence,
                            map<pair<int, phase>, int>& knowledge,
                            int goal, int i, phase step, vector<phase>& dst)
{
    if ( i == sequence.size() ){
        return;
    }
    
    pair<int, phase> key = make_pair(i, step);
    
    // Transition or match
    if ( sequence[i] == diagnosis_transition ||
        sequence[i] == step ||
         sequence[i] == diagnosis_unknown
       )
    {
        dst.push_back(sequence[i]);
        this->traverse(sequence, knowledge, goal, i + 1, step, dst);
    }
    // Special case wit Plain and Hinselmann
    else if ( step == diagnosis_plain && sequence[i] == diagnosis_hinselmann)
    {
        if ( goal == knowledge[make_pair(i + 1, step)] ){
            dst.push_back(diagnosis_plain);
            this->traverse(sequence, knowledge, goal, i + 1, step, dst);
        } else {
            this->traverse(sequence, knowledge, goal, i, sequence[i],
                           dst);
        }
    }
    // Special case wit Hinselmann and Plain
    else if ( step == diagnosis_hinselmann && sequence[i] == diagnosis_plain )
    {
        dst.push_back(step);
        this->traverse(sequence, knowledge, goal, i + 1, step, dst);
    }
    // Has next step
    else if ( this->has_next(step) ){
        // Change step
        if ( goal != 1 + knowledge[make_pair(i + 1, step)] ){
            this->traverse(sequence, knowledge, goal, i,
                           this->next_step(step), dst);
        }
        // Keep current step
        else {
            dst.push_back(step);
            this->traverse(sequence, knowledge, goal - 1, i + 1, step, dst);
        }
    }
    // Doesn't have next step
    else {
        dst.push_back(step);
        this->traverse(sequence, knowledge, goal - 1, i + 1, step, dst);
    }
}

/*****************************************************************************
 *                                   Utils                                   *
 *****************************************************************************/

diagnosis_phase_detector* diagnosis_phase_detector::from_json(
                                                    const rapidjson::Value& j)
{
    diagnosis_phase_detector* ret = 0;
    
    if ( j.IsObject() && j.HasMember("type") && j.HasMember("config") )
    {
        const rapidjson::Value& type = j["type"];
        
        if ( type.IsString() ){
            const char* t = type.GetString();
            if ( strcmp(t, "classifier") == 0 ){
                ret = new classifier_dpd();
            } else if ( strcmp(t, "window") == 0 ){
                ret = new w_dpd();
            } else if ( strcmp(t, "context") == 0 ){
                ret = new context_dpd();
            } else if ( strcmp(t, "unknown") == 0 ){
                ret = new unknown_removal_dpd();
            }
            
            ret->read(j);
        }
    }
    
    return ret;
}

diagnosis_phase_detector* diagnosis_phase_detector::get(string filename)
{
    FILE * pFile = fopen (filename.c_str(), "r");
    rapidjson::FileStream is(pFile);
    rapidjson::Document d;
    d.ParseStream<0>(is);
    
    diagnosis_phase_detector* ret = diagnosis_phase_detector::from_json(d);
    
    fclose(pFile);
    
    return ret;
}
