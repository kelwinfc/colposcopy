#ifndef __MODELS_NEIGHBORS
#define __MODELS_NEIGHBORS

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

class neighborhood_based_classifier {
    protected:
        feature_extractor* extractor;
        v_distance* distance;

    public:
        neighborhood_based_classifier();
        ~neighborhood_based_classifier();

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        virtual void train(vector<Mat>& src, vector<label>& labels);
        virtual float eval(vector<Mat>& src, vector<label>& labels);
        virtual void detect(vector<Mat>& src, vector<label>& dst);
};

class incremental_nbc : public neighborhood_based_classifier {
    protected:
        vector<sample> index_features;
        vector<label> index_label;
        vector<float> index_threshold;
        vector<float> index_reliability;
        
        float max_error;
        int bindw;
        int max_samples;

    public:
        incremental_nbc();

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        virtual void train(vector<Mat>& src, vector<label>& labels);
        virtual float eval(vector<Mat>& src, vector<label>& labels);
        virtual void detect(vector<Mat>& src, vector<label>& dst);

    protected:
        float mdistance(Mat& a, Mat& b);

        virtual float eval(vector< vector<float> >& src, vector<label>& labels);
        virtual void detect(vector< vector<float> >& src, vector<label>& dst);

        pair<int,float> best_frame(vector<Mat>& src, vector<label>& labels,
                                   vector<bool>& indexed,
                                   vector<vector<float> >& hists,
                                   vector<float>& threshold,
                                   vector<float>& reliability);

        void extract_features(vector<Mat>& src, vector< vector<float> >& h);
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

#endif
