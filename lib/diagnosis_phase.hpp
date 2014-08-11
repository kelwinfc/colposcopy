#ifndef __COLPOSCOPY_DIAGNOSIS_PHASE
#define __COLPOSCOPY_DIAGNOSIS_PHASE

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

#include <fstream>
#include <algorithm>
#include <vector>
#include <queue>
#include <set>

#include "utils.hpp"
#include "feature_extractor.hpp"
#include "distance.hpp"
#include "classifier.hpp"

using namespace std;
using namespace cv;

namespace colposcopy {

class diagnosis_phase_detector {
    private:

    public:
        enum phase {
            diagnosis_unknown,
            diagnosis_transition,
            diagnosis_plain,
            diagnosis_green,
            diagnosis_hinselmann,
            diagnosis_schiller
        };
        
        diagnosis_phase_detector();
        
        virtual void read(string filename);
        virtual void read(const rapidjson::Value& json);
        virtual void write(string filename);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void train(vector<Mat>& src, vector<phase>& labels);
        virtual float eval(vector<Mat>& src, vector<phase>& labels);
        virtual void detect(vector<Mat>& src, vector<phase>& dst);
        
        float get_confussion_matrix(vector<Mat>& src, vector<phase>& labels,
                                    map< pair<phase, phase>, int>& matrix);
        float print_confussion_matrix(vector<Mat>& src, vector<phase>& labels);
        
        static phase string_to_phase(string s);
        void visualize(vector<Mat>& src, vector<phase>& labels,
                       Mat& dst, int rows_by_frame=200, int cols_by_frame=2);
        void visualize(vector<Mat>& src, vector<phase>& labels,
                       string filename,
                       int rows_by_frame=200, int cols_by_frame=2);
        
        static diagnosis_phase_detector* from_json(const rapidjson::Value& j);
        static diagnosis_phase_detector* get(string filename);
};

class classifier_dpd : public diagnosis_phase_detector {
    protected:
        classifier* c;
        bool delete_classifier;
    
    public:
        classifier_dpd();
        ~classifier_dpd();
        
        void set_classifier(classifier* cl);
        
        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void train(vector<Mat>& src, vector<phase>& labels);
        virtual float eval(vector<Mat>& src, vector<phase>& labels);
        virtual void detect(vector<Mat>& src, vector<phase>& dst);
    
    protected:
        void phase_to_label(vector<phase>& in, vector<label>& out);
        void label_to_phase(vector<label>& in, vector<phase>& out);
};

class w_dpd : public diagnosis_phase_detector {
    private:
        diagnosis_phase_detector* underlying_detector;
        int w;
        bool delete_detector;
    
    public:
        w_dpd();
        w_dpd(diagnosis_phase_detector* d, int w);
        ~w_dpd();
        
        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void train(vector<Mat>& src, vector<phase>& labels);
        virtual float eval(vector<Mat>& src, vector<phase>& labels);
        virtual void detect(vector<Mat>& src, vector<phase>& dst);

    private:
        void initialize_diagnosis_phase_map(map<phase, int>& h);
};

class context_rule {
    private:
        diagnosis_phase_detector::phase pre;
        diagnosis_phase_detector::phase post;
        diagnosis_phase_detector::phase replace_by;
        bool seen_before;
    
    public:
        context_rule(diagnosis_phase_detector::phase pre,
                     diagnosis_phase_detector::phase post,
                     diagnosis_phase_detector::phase replace_by,
                     bool barkward
                    );

        friend bool operator<(context_rule const& a, context_rule const& b)
        {
            return a.get_pre() < b.get_pre() ||
                   (a.get_pre() == b.get_pre() &&
                    a.get_post() < b.get_post() );
        }
        
        diagnosis_phase_detector::phase get_pre() const;
        diagnosis_phase_detector::phase get_post() const;
        diagnosis_phase_detector::phase get_replacement() const;
        bool is_before_rule() const;
};

class context_dpd : public diagnosis_phase_detector {
    private:
        diagnosis_phase_detector* underlying_detector;
        set<context_rule> rules;
    
    public:
        context_dpd();
        context_dpd(diagnosis_phase_detector* d);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void train(vector<Mat>& src, vector<phase>& labels);
        virtual float eval(vector<Mat>& src, vector<phase>& labels);
        virtual void detect(vector<Mat>& src, vector<phase>& dst);

    private:
        diagnosis_phase_detector::phase is_ok(set<phase>& seen, phase p);
};

class unknown_removal_dpd : public diagnosis_phase_detector {
    private:
        diagnosis_phase_detector* underlying_detector;
    
    public:
        unknown_removal_dpd();
        unknown_removal_dpd(diagnosis_phase_detector* d);
        
        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void train(vector<Mat>& src, vector<phase>& labels);
        virtual float eval(vector<Mat>& src, vector<phase>& labels);
        virtual void detect(vector<Mat>& src, vector<phase>& dst);
};

class binary_dpd : public diagnosis_phase_detector {
    protected:
        diagnosis_phase_detector* left;
        diagnosis_phase_detector* right;
    
    public:
        binary_dpd();
        binary_dpd(diagnosis_phase_detector* l, diagnosis_phase_detector* r);
        
        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void train(vector<Mat>& src, vector<phase>& labels);
        virtual float eval(vector<Mat>& src, vector<phase>& labels);
        virtual void detect(vector<Mat>& src, vector<phase>& dst);
}

};

#endif
