#include "anonadado.hpp"

using namespace anonadado;

/*****************************************************************************
 *                                  Feature                                  *
 *****************************************************************************/

feature::feature()
{
    this->name = "";
    this->type = "";
}

feature::feature(feature& a)
{
    this->name = a.name;
    this->type = a.type;
}

void feature::read(const rapidjson::Value& v, bool just_value)
{
    if ( !just_value ){
        this->name = rapidjson_get_string(v, "name", "");
        this->type = rapidjson_get_string(v, "type", "");
    }
}

void feature::read(std::string filename, bool just_value)
{
    
}

std::string feature::get_type()
{
    return this->type;
}

std::string feature::get_name()
{
    return this->name;
}

/******************************* Bool Feature ********************************/

bool_feature::bool_feature()
{
    this->default_value = true;
    this->value = true;
    this->type = "bool";
}

bool_feature::bool_feature(bool_feature& f)
{
    this->default_value = f.default_value;
    this->value = f.value;
    this->type = "bool";
}

void bool_feature::read(const rapidjson::Value& v, bool just_value)
{
    if ( !just_value ){
        feature::read(v);
        this->default_value = rapidjson_get_bool(v, "default", true);
    }
    
    this->value = rapidjson_get_bool(v, "value", this->default_value);
}

bool bool_feature::get_value()
{
    return this->value;
}

/****************************** String Feature *******************************/

str_feature::str_feature()
{
    this->default_value = "";
    this->value = "";
    this->type = "string";
}

str_feature::str_feature(str_feature& f)
{
    this->default_value = f.default_value;
    this->value = f.value;
    this->type = "string";
}

void str_feature::read(const rapidjson::Value& v, bool just_value)
{
    if ( !just_value ){
        feature::read(v);
        this->default_value = rapidjson_get_string(v, "default", "");
    }
    
    this->value = rapidjson_get_string(v, "value", this->default_value);
}

std::string str_feature::get_value()
{
    return this->value;
}

/******************************* Float Feature *******************************/

float_feature::float_feature()
{
    this->default_value = 0.0;
    this->value = 0.0;
    this->type = "float";
}

float_feature::float_feature(float_feature& f)
{
    this->default_value = f.default_value;
    this->value = f.value;
    this->type = "float";
}

void float_feature::read(const rapidjson::Value& v, bool just_value)
{
    if ( !just_value){
        feature::read(v);
        this->default_value = rapidjson_get_float(v, "default", 0.0);
    }
    
    this->value = rapidjson_get_float(v, "value", this->default_value);
}

float float_feature::get_value()
{
    return this->value;
}

/******************************** Int Feature ********************************/

int_feature::int_feature()
{
    this->default_value = 0;
    this->value = 0;
    this->type = "int";
}

int_feature::int_feature(int_feature& f)
{
    this->default_value = f.default_value;
    this->value = f.value;
    this->type = "int";
}

void int_feature::read(const rapidjson::Value& v, bool just_value)
{
    if ( !just_value ){
        feature::read(v);
        this->default_value = rapidjson_get_int(v, "default", 0);
    }
    
    this->value = rapidjson_get_int(v, "value", this->default_value);
}

int int_feature::get_value()
{
    return this->value;
}

/****************************** Choice Feature *******************************/

choice_feature::choice_feature()
{
    this->default_value = 0;
    this->value = 0;
    this->type = "choice";
}

choice_feature::choice_feature(choice_feature& f)
{
    this->default_value = f.default_value;
    this->value = f.value;
    this->type = "choice";
    
    this->values = f.values;
}

void choice_feature::read(const rapidjson::Value& v, bool just_value)
{
    if ( !just_value ){
        feature::read(v);
        std::string dv = rapidjson_get_string(v, "default", "");
        std::string av = rapidjson_get_string(v, "value", "");
        bool has_value = false;
        
        this->values.clear();
        if ( v.HasMember("values") && v["values"].IsArray() ){
            const rapidjson::Value& values = v["values"];
            int index = 0;
            for (rapidjson::SizeType i = 0; i < values.Size(); i++){

                const rapidjson::Value& v_json = values[i];
                
                if ( v_json.IsString() ){
                    std::string vs = v_json.GetString();
                    this->values.push_back(vs);

                    if ( vs == dv ){
                        this->default_value = index;
                    }
                    if ( vs == av ){
                        this->value = index;
                    }

                    index++;
                }
            }
        }

        if ( !has_value ){
            this->value = this->default_value;
        }
    } else {
        std::string av = rapidjson_get_string(v, "value", "");
        for ( uint i=0; i<this->values.size(); i++ ){
            if ( this->values[i] == av ){
                this->value = i;
            }
        }
    }
}

std::string choice_feature::get_value()
{
    return this->values[this->value];
}

/******************************* Bbox Feature ********************************/

bbox_feature::bbox_feature()
{
    this->default_value = DEFAULT_BBOX;
    this->value = DEFAULT_BBOX;
    this->type = "bbox";
}

bbox_feature::bbox_feature(bbox_feature& f)
{
    this->default_value = f.default_value;
    this->value = f.value;
    this->type = "bbox";
}

void bbox_feature::read(const rapidjson::Value& v, bool just_value)
{
    if ( !just_value ){
        feature::read(v);
        this->default_value = rapidjson_get_bbox(v, "default", DEFAULT_BBOX);
    }
    
    this->value = rapidjson_get_bbox(v, "value", this->default_value);
}

BBOX bbox_feature::get_value()
{
    return this->value;
}

/****************************** Vector Feature *******************************/

vector_feature::vector_feature()
{
    this->default_value = DEFAULT_BBOX;
    this->value = DEFAULT_BBOX;
    this->type = "vector";
}

vector_feature::vector_feature(vector_feature& f)
{
    this->default_value = f.default_value;
    this->value = f.value;
    this->type = "vector";
}

/******************************* Point Feature *******************************/

point_feature::point_feature()
{
    this->default_value = DEFAULT_POINT;
    this->value = DEFAULT_POINT;
    this->type = "point";
}

point_feature::point_feature(point_feature& f)
{
    this->default_value = f.default_value;
    this->value = f.value;
    this->type = "point";
}

void point_feature::read(const rapidjson::Value& v, bool just_value)
{
    if ( !just_value ){
        feature::read(v);
        this->default_value = rapidjson_get_point(v, "default", DEFAULT_POINT);
    }
    
    this->value = rapidjson_get_point(v, "value", this->default_value);
}

POINT point_feature::get_value()
{
    return this->value;
}
/*****************************************************************************/

feature* get_feature_instance(const rapidjson::Value& v)
{
    feature* ret;
    
    std::string type = rapidjson_get_string(v, "type", "");
    
    if ( type == "bool" ){
        ret = new bool_feature();
    } else if ( type == "string" ){
        ret = new str_feature();
    } else if ( type == "float" ){
        ret = new float_feature();
    } else if ( type == "int" ){
        ret = new int_feature();
    } else if ( type == "choice" ){
        ret = new choice_feature();
    } else if ( type == "bbox" ){
        ret = new bbox_feature();
    } else if ( type == "vector" ){
        ret = new vector_feature();
    } else {
        ret = new feature();
    }
    
    ret->read(v);
    
    return ret;
}

/*****************************************************************************
 *                                Annotation                                 *
 *****************************************************************************/

annotation::annotation()
{
    this->frame = -1;
    this->is_unique = false;
    this->is_global = false;
    this->name = "";
}

feature* get_feature_instance(feature* f)
{
    feature* ret = 0;
    
    std::string type = f->get_type();
    
    if ( type == "bool" ){
        bool_feature* cf = (bool_feature*)f;
        ret = new bool_feature(*cf);
    } else if ( type == "string" ){
        str_feature* cf = (str_feature*)f;
        ret = new str_feature(*cf);
    } else if ( type == "float" ){
        float_feature* cf = (float_feature*)f;
        ret = new float_feature(*cf);
    } else if ( type == "int" ){
        int_feature* cf = (int_feature*)f;
        ret = new int_feature(*cf);
    } else if ( type == "choice" ){
        choice_feature* cf = (choice_feature*)f;
        ret = new choice_feature(*cf);
    } else if ( type == "bbox" ){
        bbox_feature* cf = (bbox_feature*)f;
        ret = new bbox_feature(*cf);
    } else if ( type == "vector" ){
        vector_feature* cf = (vector_feature*)f;
        ret = new vector_feature(*cf);
    } else {
        ret = new feature(*f);
    }
    
    return ret;
}

annotation::annotation(annotation& a)
{
    this->frame = a.frame;
    this->name  = a.name;
    this->is_unique = a.is_unique;
    this->is_global = a.is_global;
    
    this->clear_features();
    
    std::map<std::string, feature*>::iterator it;
    for ( it = a.features.begin(); it != a.features.end(); ++it ){
        if ( this->features.find(it->first) != this->features.end() )
            delete this->features[it->first];
        this->features[it->first] = get_feature_instance(it->second);
    }
}

annotation::~annotation()
{
    this->clear_features();
}

void annotation::read(const rapidjson::Value& v, bool just_value)
{
    if ( !just_value ){
        this->is_unique = rapidjson_get_bool(v, "is_unique", false);
        this->is_global = rapidjson_get_bool(v, "is_global", false);
        this->name = rapidjson_get_string(v, "name", "");
        
        this->clear_features();
    }
    
    this->frame = rapidjson_get_int(v, "frame", -1);

    if ( v.HasMember("features") && v["features"].IsArray() )
    {
        const rapidjson::Value& f = v["features"];
        int index = 0;
        
        for (rapidjson::SizeType i = 0; i < f.Size(); i++){
            const rapidjson::Value& f_json = f[i];
            feature* f = 0;
            
            if ( just_value )
            {
                f = this->features[rapidjson_get_string(f_json, "name", "")];
                f->read(f_json, true);
            } else {
                f = get_feature_instance(f_json);
                
                if (this->features.find(f->get_name()) != this->features.end())
                {
                    delete this->features[f->get_name()];
                }
                
                this->features[f->get_name()] = f;
            }
            
            index++;
        }
    }
}

void annotation::read(std::string filename)
{
    FILE * pFile = fopen (filename.c_str() , "r");
    rapidjson::FileStream is(pFile);
    rapidjson::Document document;
    document.ParseStream<0>(is);

    this->read(document);
    
    fclose(pFile);
}

std::string annotation::get_name()
{
    return this->name;
}

void annotation::clear_features()
{
    std::map<std::string, feature*>::iterator it;
    for ( it = this->features.begin(); it != this->features.end(); ++it){
        delete it->second;
    }
}

int annotation::get_frame()
{
    return this->frame;
}

feature* annotation::get_feature(std::string name)
{
    feature* ret = 0;
    
    if ( this->features.find(name) != this->features.end() ){
        ret = this->features[name];
    }
    
    return ret;
}

void annotation::get_features(std::vector<std::string>& f)
{
    std::map<std::string, feature*>::iterator it, end;
    
    f.clear();
    it = this->features.begin();
    end = this->features.end();

    for ( ; it != end; ++it ){
        f.push_back(it->first);
    }
}

/*****************************************************************************
 *                                  Domain                                   *
 *****************************************************************************/

domain::domain()
{
    this->name = "";
}

domain::~domain()
{
    this->clear_labels();
}

void domain::read(std::string filename)
{
    FILE * pFile = fopen (filename.c_str() , "r");
    rapidjson::FileStream is(pFile);
    rapidjson::Document document;
    document.ParseStream<0>(is);
    this->clear_labels();

    this->name = rapidjson_get_string(document, "name", "");
    if ( document.HasMember("labels") && document["labels"].IsArray() ){
        const rapidjson::Value& labels = document["labels"];
        for (rapidjson::SizeType i = 0; i < labels.Size(); i++){
            const rapidjson::Value& a_json = labels[i];
            annotation* a = new annotation();
            a->read(a_json);
            this->labels[a->get_name()] = a;
        }
    }
    fclose(pFile);
}

annotation* domain::get_descriptor(std::string label_name)
{
    annotation* ret = 0;

    if ( this->labels.find(label_name) != this->labels.end() ){
        ret = this->labels[label_name];
    }
    
    return ret;
}

annotation* domain::get_instance(std::string label_name)
{
    annotation* ret = 0;

    if ( this->labels.find(label_name) != this->labels.end() ){
        ret = new annotation(*this->labels[label_name]);
    }
    return ret;
}

void domain::clear_labels()
{
    std::map<std::string, annotation*>::iterator it;

    for ( it = this->labels.begin(); it != this->labels.end(); ++it ){
        delete it->second;
    }
    this->labels.clear();
}

/*****************************************************************************
 *                                 Instance                                  *
 *****************************************************************************/

instance::instance()
{
    this->d = 0;
}

instance::~instance()
{
    if ( this->d ){
        delete this->d;
    }
    
    this->clear_annotations();
}

void instance::read(std::string filename)
{
    FILE * pFile = fopen (filename.c_str() , "r");
    rapidjson::FileStream is(pFile);
    rapidjson::Document document;
    document.ParseStream<0>(is);
    
    this->sequence_filename =
        rapidjson_get_string(document, "sequence_filename");
    
    this->video_filename =
        rapidjson_get_string(document, "video_filename");
    
    this->instance_name = rapidjson_get_string(document, "name");
    this->instance_filename = filename;

    this->domain_filename = rapidjson_get_string(document, "domain");

    if ( this->d != 0 ){
        delete this->d;
    }
    
    this->d = new domain();
    this->d->read(this->domain_filename);

    this->clear_annotations();
    
    if ( document.HasMember("sequence") && document["sequence"].IsArray() )
    {
        const rapidjson::Value& sequence = document["sequence"];

        for (rapidjson::SizeType i = 0; i < sequence.Size(); i++){
            const rapidjson::Value& n_annotation = sequence[i];

            std::vector<annotation*> next;

            if ( n_annotation.IsArray() ){
                for (rapidjson::SizeType j = 0; j < n_annotation.Size(); j++){
                    const rapidjson::Value& nn_annotation = n_annotation[j];
                    
                    if ( nn_annotation.HasMember("name") && 
                         nn_annotation["name"].IsString() )
                    {
                        std::string label = nn_annotation["name"].GetString();
                        annotation* prev =  this->d->get_instance(label);

                        if ( prev != 0 ){
                            annotation* n = new annotation(*prev);
                            n->read(nn_annotation, true);
                            next.push_back(n);
                            delete prev;
                        }
                    }
                }
            }
            
            this->annotations.push_back(next);
        }
    }
    
    fclose(pFile);
}

void instance::clear_annotations()
{
    std::vector< std::vector<annotation*> >::iterator it;
    for ( it = this->annotations.begin(); it != this->annotations.end(); ++it){
        std::vector<annotation*>::iterator inner_it;
        for ( inner_it = it->begin(); inner_it != it->end(); ++inner_it){
            delete *inner_it;
        }
    }
}

void instance::get_active_annotations(int frame_number,
                                      std::vector<int>& annotation_index)
{
    annotation_index.clear();
    int index = 0;
    
    std::vector< std::vector<annotation*> >::iterator it;
    for ( it = this->annotations.begin(); it != this->annotations.end(); ++it )
    {
        
        std::vector<annotation*> next = *it;
        
        if ( next.size() > 0 && 
             next[0]->get_frame() <= frame_number &&
             frame_number <= next.back()->get_frame()
           )
        {
            annotation_index.push_back(index);
        }
        
        index++;
    }
}

annotation* instance::get_active_annotation(int index, int frame)
{
    annotation* ret = 0;
    int max_frame = -1;
    
    if (0 <= index && index < (int)this->annotations.size()){
        
        std::vector<annotation*>::iterator it, end;
        
        it = this->annotations[index].begin();
        end = this->annotations[index].end();
        
        for ( ; it != end; ++it ){
            annotation* n = *it;
            int n_frame = n->get_frame();
            if ( n_frame <= frame && n_frame >= max_frame ){
                max_frame = n_frame;
                ret = n;
            }
        }
    }
    
    return ret;
}

void instance::get_frame(int index, cv::Mat& dst)
{
    std::string dir;
    std::stringstream ss;

    ss << this->sequence_filename;
    
    if ( this->sequence_filename[this->sequence_filename.size() - 1] != '/' ){
        ss << "/";
    }

    ss << index << ".jpg";

    getline(ss, dir);

    dst = cv::imread(dir);
}

int instance::num_frames()
{
    std::string path, line;
    std::stringstream ss, ss_frames;
    int ret;
    
    ss << this->sequence_filename;

    if ( this->sequence_filename[this->sequence_filename.size() - 1] != '/' ){
        ss << "/";
    }

    ss << "anonadado.data";

    getline(ss, path);

    std::ifstream fin(path.c_str());

    getline(fin, line);
    getline(fin, line);
    
    ss_frames << line;
    ss_frames >> ret;
    
    return ret;
}

void instance::get_annotations(std::string name, std::vector<int>& out)
{
    std::vector<std::vector<annotation*> >::iterator it, end;
    int index = 0;
    
    it = this->annotations.begin();
    end = this->annotations.end();
    out.clear();
    
    for ( ; it != end; ++it ){
        if ( it->size() > 0 && (*it)[0]->get_name() == name ){
            out.push_back(index);
        }
        index++;
    }
}
