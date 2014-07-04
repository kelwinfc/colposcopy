#ifndef __MODELS_CLASSIFIER
#define __MODELS_CLASSIFIER

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

#define NB_sample vector<float>
#define UNKNOWN -1

typedef int label;
typedef vector<float> sample;

class classifier {
    protected:
        feature_extractor* extractor;

    public:
        classifier();
        ~classifier();

        void set_feature_extractor(feature_extractor* extractor);
        
        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        virtual void train(vector<Mat>& src, vector<label>& labels);
        virtual void untrain();
        
        virtual float eval(vector<Mat>& src, vector<label>& labels);
        virtual void detect(vector<Mat>& src, vector<label>& dst);
        virtual label predict(Mat& src);
    
    protected:
        void extract_features(vector<Mat>& src, vector< vector<float> >& h);
};

class neighborhood_based_classifier : public classifier {
    protected:
        v_distance* distance;
    
    public:
        neighborhood_based_classifier();
        ~neighborhood_based_classifier();

        void set_distance(v_distance* d);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        virtual void train(vector<Mat>& src, vector<label>& labels);
        virtual void untrain();
        
        virtual float eval(vector<Mat>& src, vector<label>& labels);
        virtual void detect(vector<Mat>& src, vector<label>& dst);
        virtual label predict(Mat& src);
};

class incremental_nbc : public neighborhood_based_classifier {
    protected:
        vector<sample> index_features;
        vector<label> index_label;
        vector<float> index_threshold;
        vector<float> index_reliability;

        float max_error;
        int max_samples;
        float min_convergence;
    
    public:
        incremental_nbc();
        
        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        virtual void train(vector<Mat>& src, vector<label>& labels);
        virtual void untrain();
        
        virtual float eval(vector<Mat>& src, vector<label>& labels);
        virtual void detect(vector<Mat>& src, vector<label>& dst);
        virtual label predict(Mat& src);
        
        uint index_size();
    protected:
        float mdistance(Mat& a, Mat& b);

        virtual float eval(vector< vector<float> >& src, vector<label>& labels);
        virtual void detect(vector< vector<float> >& src, vector<label>& dst);

        pair<int,float> best_frame(vector<Mat>& src, vector<label>& labels,
                                   vector<bool>& indexed,
                                   vector<vector<float> >& hists,
                                   vector<float>& threshold,
                                   vector<float>& reliability);
        
        float compute_threshold(vector<Mat>& src, vector<label>& labels,
                                vector< vector<float> >& hists, int i);
        void compute_thresholds(vector<Mat>& src, vector<label>& labels,
                                vector<vector<float> >& h,
                                vector<float>& threshold);
        void compute_reliability(vector<Mat>& src, vector<label>& labels,
                                 vector< vector<float> >& h,
                                 vector<float>& threshold,
                                 vector<float>& reliability);

        void add_to_index(vector<float>& hist, label label, float threshold,
                          float reliability);
        void remove_last();

        void get_target_frames(vector<Mat>& src,
                               vector<label>& labels,
                               vector<Mat>& src_train,
                               vector<label>& labels_train);
};

class knn : public incremental_nbc {
    protected:
        int k;
        map<label, float> weight;
        
    public:
        knn();
        knn(int k);
        
        void set_k(int k);
        
        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        virtual void train(vector<Mat>& src, vector<label>& labels);
        
    protected:
        void detect(vector< vector<float> >& src, vector<label>& dst);
};

class threshold_cl : public classifier {
    protected:
        float k;
    
    public:
        threshold_cl();
        ~threshold_cl();
        
        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        virtual void train(vector<Mat>& src, vector<label>& labels);
        virtual void untrain();
        
        virtual float eval(vector<Mat>& src, vector<label>& labels);
        virtual void detect(vector<Mat>& src, vector<label>& dst);
        virtual label predict(Mat& src);
        
        void set_threshold(int k);
};

#endif
