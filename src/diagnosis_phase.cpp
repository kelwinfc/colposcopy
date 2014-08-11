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
        if ( in[i] == diagnosis_unknown ){
            out[i] = UNKNOWN;
        } else {
            out[i] = (label)in[i];
        }
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
                           diagnosis_phase_detector::phase replace_by,
                           bool seen_before
                          )
{
    this->pre = pre;
    this->post = post;
    this->replace_by = replace_by;
    this->seen_before = seen_before;
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

bool context_rule::is_before_rule() const
{
    return this->seen_before;
}

context_dpd::context_dpd()
{
    this->underlying_detector = 0;
}

context_dpd::context_dpd(diagnosis_phase_detector* d)
{
    this->underlying_detector = d;

    this->rules.insert( context_rule(diagnosis_plain, diagnosis_hinselmann,
                                     diagnosis_plain, true) );
    this->rules.insert( context_rule(diagnosis_green, diagnosis_schiller,
                                     diagnosis_plain, true) );
    this->rules.insert( context_rule(diagnosis_green, diagnosis_hinselmann,
                                     diagnosis_plain, true) );
    
    this->rules.insert( context_rule(diagnosis_green, diagnosis_plain,
                                     diagnosis_hinselmann, false) );
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
    // noop
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
            if ( it->is_before_rule() &&
                 seen.find(it->get_pre()) == seen.end() )
            {
                return it->get_replacement();
            } else if ( !it->is_before_rule() &&
                        seen.find(it->get_pre()) != seen.end() )
            {
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
    //TODO
}

float binary_dpd::eval(vector<Mat>& src, vector<phase>& labels)
{
    //TODO
}

void binary_dpd::detect(vector<Mat>& src, vector<phase>& dst)
{
    //TODO
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
