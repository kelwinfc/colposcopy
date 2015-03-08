#ifndef __MODELS_NEIGHBORS_FEATURE_EXTRACTOR
#define __MODELS_NEIGHBORS_FEATURE_EXTRACTOR

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/video.hpp"
#include "ml.h"

#include "contrib/rapidjson/rapidjson.h"
#include "contrib/rapidjson/document.h"
#include "contrib/rapidjson/prettywriter.h"
#include "contrib/rapidjson/filestream.h"
#include "contrib/rapidjson/stringbuffer.h"
#include "contrib/pugixml.hpp"

#include "anonadado.hpp"
#include "anonadado_utils.hpp"

#include <fstream>
#include <algorithm>
#include <vector>
#include <queue>
#include <stack>
#include <set>

#include "utils.hpp"
#include "specular_reflection.hpp"
#include "cervix_segmentation.hpp"

using namespace std;
using namespace cv;

class feature_extractor {
    public:
        feature_extractor();
        virtual void extract_by_inst(anonadado::instance& inst, int i,
                                     vector<float>& out);
        virtual void extract(vector<Mat>& in, int i, vector<float>& out,
                             anonadado::instance* instance=0);

        virtual void read(string filename);
        virtual void write(string filename);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void get_names(vector<string>& names);
};

class identity_fe : public feature_extractor {
    public:
        identity_fe();
        virtual void extract_by_inst(anonadado::instance& inst, int i,
                                     vector<float>& out);
        virtual void extract(vector<Mat>& in, int i, vector<float>& out,
                             anonadado::instance* instance=0);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void get_names(vector<string>& names);
};

class add_inverse_fe : public feature_extractor {
    protected:
        feature_extractor* underlying_fe;

    public:
        add_inverse_fe(feature_extractor* u);
        
        virtual void extract_by_inst(anonadado::instance& inst, int i,
                                     vector<float>& out);
        virtual void extract(vector<Mat>& in, int i, vector<float>& out,
                             anonadado::instance* instance=0);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void get_names(vector<string>& names);
    protected:
        void add_inverse(vector<float>& out);
};

class merge_fe : public feature_extractor {
    protected:
        vector<feature_extractor*> fe_seq;
    
    public:
        merge_fe(){}
        merge_fe(vector<feature_extractor*>& fe_seq);
        
        virtual void extract_by_inst(anonadado::instance& inst, int i,
                                     vector<float>& out);
        virtual void extract(vector<Mat>& in, int i, vector<float>& out,
                             anonadado::instance* instance=0);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void get_names(vector<string>& names);
};

class merge_single_frame_fe : public merge_fe {
    public:
        merge_single_frame_fe(vector<feature_extractor*>& fe_seq);
        
        virtual void extract_by_inst(anonadado::instance& inst, int i,
                                     vector<float>& out);
        virtual void extract(vector<Mat>& in, int i, vector<float>& out,
                             anonadado::instance* instance=0);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void get_names(vector<string>& names);
};

class selective_hsv_channel_histogram_fe : public feature_extractor {
    protected:
        int bindw;
        bool normalize;
        int use[3];
        string name;
        
        int hist_range[3];
        int hist_size[3];
        int hist_shift[3];
        int total;
    
    public:
        selective_hsv_channel_histogram_fe();
        selective_hsv_channel_histogram_fe(int bindw, bool normalize,
                                           int use_ch[3], string name);
        
        virtual void extract(vector<Mat>& in, int i, vector<float>& out,
                             anonadado::instance* instance=0);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void get_names(vector<string>& names);
    protected:
        void initialize();
};

static int hue_histogram_fe_usage[3] = {1, 0, 0};

class hue_histogram_fe : public selective_hsv_channel_histogram_fe {
    public:
        hue_histogram_fe() 
            : selective_hsv_channel_histogram_fe(10, true,
                                                 hue_histogram_fe_usage,
                                                 "sat"){}
        hue_histogram_fe(int bindw, bool normalize)
            : selective_hsv_channel_histogram_fe(bindw, normalize,
                                                 hue_histogram_fe_usage,
                                                 "hue"){}
};

static int saturation_histogram_fe_usage[3] = {0, 1, 0};

class saturation_histogram_fe : public selective_hsv_channel_histogram_fe {
    public:
        saturation_histogram_fe() : 
            selective_hsv_channel_histogram_fe(10, true,
                                               saturation_histogram_fe_usage,
                                               "sat"){}
        saturation_histogram_fe(int bindw, bool normalize)
            : selective_hsv_channel_histogram_fe(bindw, normalize,
                                                 saturation_histogram_fe_usage,
                                                 "sat"){}
};

static int hs_histogram_fe[3] = {1, 1, 0};

class hue_sat_histogram_fe : public selective_hsv_channel_histogram_fe {
    public:
        hue_sat_histogram_fe() : 
            selective_hsv_channel_histogram_fe(10, true,
                                               hs_histogram_fe,
                                               "hue_sat"){}
        hue_sat_histogram_fe(int bindw, bool normalize)
            : selective_hsv_channel_histogram_fe(bindw, normalize,
                                                 hs_histogram_fe,
                                                 "hue_sat"){}
};

static int full_hsv_histogram_fe[3] = {1, 1, 1};

class hsv_histogram_fe : public selective_hsv_channel_histogram_fe {
    public:
        hsv_histogram_fe() : 
            selective_hsv_channel_histogram_fe(10, true,
                                               full_hsv_histogram_fe,
                                               "hsv_hist"){}
        hsv_histogram_fe(int bindw, bool normalize)
            : selective_hsv_channel_histogram_fe(bindw, normalize,
                                                 full_hsv_histogram_fe,
                                                 "hsv_hist"){}
};

class motion_fe : public feature_extractor {
    protected:
        int w;
        int width;
        int height;
    
    public:
        motion_fe();
        motion_fe(int w, int width, int height);
        
        virtual void extract_by_inst(anonadado::instance& inst, int i,
                                     vector<float>& out);
        virtual void extract(vector<Mat>& in, int i, vector<float>& out,
                             anonadado::instance* instance=0);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void get_names(vector<string>& names);
};

class focus_fe : public feature_extractor {
    public:
        focus_fe();
        
        virtual void extract_by_inst(anonadado::instance& inst, int i,
                                     vector<float>& out);
        virtual void extract(vector<Mat>& in, int i, vector<float>& out,
                             anonadado::instance* instance=0);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void get_names(vector<string>& names);
};

class specular_reflection_fe : public feature_extractor {
    protected:
        specular_reflection_detection* detection;
    
    public:
        specular_reflection_fe();
        specular_reflection_fe(specular_reflection_detection* d);

        virtual void extract(vector<Mat>& in, int i, vector<float>& out,
                             anonadado::instance* instance=0);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void get_names(vector<string>& names);
    
    protected:
        int flood_fill(Mat& img, int r, int c);
};

class color_cascade_fe : public feature_extractor {
    protected:
        int levels;
        specular_reflection_detection* sr_detection;
        img_inpaint* inpainting;
        bool hard;
        int nfeatures;
        int features_width;

    public:
        color_cascade_fe();
        color_cascade_fe(int levels, bool hard=true,
                         specular_reflection_detection* sr_detection=0,
                         img_inpaint* inpainting=0
        );
        
        virtual void extract(vector<Mat>& in, int i, vector<float>& out,
                             anonadado::instance* instance=0);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void get_names(vector<string>& names);
    
    protected:
        int num_features();
};

class hsv_fe : public feature_extractor {
    protected:
        specular_reflection_detection* sr_detection;
    
    public:
        hsv_fe(specular_reflection_detection* sr_detection=0);
        
        virtual void extract(vector<Mat>& in, int i, vector<float>& out,
                             anonadado::instance* instance=0);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void get_names(vector<string>& names);
};

class closest_transition_fe : public feature_extractor {
    public:
        closest_transition_fe();
        
        virtual void extract_by_inst(anonadado::instance& inst, int i,
                                     vector<float>& out);
        virtual void extract(vector<Mat>& in, int i, vector<float>& out,
                             anonadado::instance* instance=0);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void get_names(vector<string>& names);
};

class edges_summations_fe : public feature_extractor {
    protected:
        specular_reflection_detection* sr_detection;
        img_inpaint* inpainting;

    public:
        edges_summations_fe(specular_reflection_detection* sr_detection=0,
                            img_inpaint* inpainting=0);
        
        virtual void extract(vector<Mat>& in, int i, vector<float>& out,
                             anonadado::instance* instance=0);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void get_names(vector<string>& names);
};

class cervix_region_fe : public feature_extractor {
    protected:
        cervix_segmentation* cervix;
        cervix_segmentation* epithelium;
        
    public:
        cervix_region_fe(cervix_segmentation* cervix=0,
                         cervix_segmentation* epithelium=0);
        
        virtual void extract(vector<Mat>& in, int i, vector<float>& out,
                             anonadado::instance* instance=0);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void get_names(vector<string>& names);
};

#endif
