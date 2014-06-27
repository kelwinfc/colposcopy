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

#include "neighbors.hpp"

#include "utils.hpp"
#include "feature_extractor.hpp"
#include "distance.hpp"

#define HBDP_HISTOGRAM vector<float>
#define HBDP_SAMPLE HBDP_HISTOGRAM

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
};

class histogram_based_dpd : public diagnosis_phase_detector {
    protected:
        vector<HBDP_SAMPLE> index_histogram;
        vector<phase> index_phase;
        vector<float> index_threshold;
        vector<float> index_reliability;
        float max_error;
        int bindw;
        int max_samples;
        feature_extractor* extractor;
        v_distance* dist;
    
    public:
        histogram_based_dpd();
        ~histogram_based_dpd();
        
        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void train(vector<Mat>& src, vector<phase>& labels);
        virtual float eval(vector<Mat>& src, vector<phase>& labels);
        virtual void detect(vector<Mat>& src, vector<phase>& dst);
    
    protected:
        float distance(Mat& a, Mat& b);
        
        virtual float eval(vector< vector<float> >& src, vector<phase>& labels);
        virtual void detect(vector< vector<float> >& src, vector<phase>& dst);
        
        pair<int,float> best_frame(vector<Mat>& src, vector<phase>& labels,
                                   vector<bool>& indexed,
                                   vector<vector<float> >& hists,
                                   vector<float>& threshold,
                                   vector<float>& reliability);
        
        void compute_histograms(vector<Mat>& src, vector< vector<float> >& h);
        float compute_threshold(vector<Mat>& src, vector<phase>& labels,
                                vector< vector<float> >& hists, int i);
        void compute_thresholds(vector<Mat>& src, vector<phase>& labels,
                                vector<vector<float> >& h,
                                vector<float>& threshold);
        void compute_reliability(vector<Mat>& src, vector<phase>& labels,
                                 vector< vector<float> >& h,
                                 vector<float>& threshold,
                                 vector<float>& reliability);

        void add_to_index(vector<float>& hist, phase label, float threshold,
                          float reliability);
        void remove_last();
        
        void get_target_frames(vector<Mat>& src,
                               vector<phase>& labels,
                               vector<Mat>& src_train,
                               vector<phase>& labels_train);
};

class knn_dpd : public histogram_based_dpd {
    protected:
        int k;

    public:
        knn_dpd();
        
        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void train(vector<Mat>& src, vector<phase>& labels);
        virtual float eval(vector<Mat>& src, vector<phase>& labels);
        virtual void detect(vector<Mat>& src, vector<phase>& dst);

    protected:
        virtual void detect(vector< vector<float> >& src, vector<phase>& dst);
};

class w_dpd : public diagnosis_phase_detector {
    private:
        diagnosis_phase_detector* underlying_detector;
        int w;
    
    public:
        w_dpd();
        w_dpd(diagnosis_phase_detector* d, int w);

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

};

#endif
