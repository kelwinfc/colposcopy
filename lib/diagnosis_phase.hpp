#ifndef __COLPOSCOPY_DIAGNOSIS_PHASE
#define __COLPOSCOPY_DIAGNOSIS_PHASE

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/video.hpp"
#include "ml.h"

#include <vector>
#include <queue>
#include <algorithm>

using namespace std;
using namespace cv;

class diagnosis_phase_detector {
    private:

    public:
        enum phase {
            diagnosis_plain,
            diagnosis_green,
            diagnosis_hinselmann,
            diagnosis_schiller,
            diagnosis_transition,
            diagnosis_unknown
        };
        
        diagnosis_phase_detector();
        virtual void read(string filename);
        virtual void train(vector<Mat>& src, vector<phase>& labels);
        virtual float eval(vector<Mat>& src, vector<phase>& labels);
        virtual void detect(vector<Mat>& src, vector<phase>& dst);

        float get_confussion_matrix(vector<Mat>& src, vector<phase>& labels,
                                    map< pair<phase, phase>, int>& matrix);
        static phase string_to_phase(string s);
};

#define HBDP_HISTOGRAM vector<float>
#define HBDP_SAMPLE HBDP_HISTOGRAM

class histogram_based_dp_detector : public diagnosis_phase_detector {
    private:
        vector<HBDP_SAMPLE> index_histogram;
        vector<phase> index_phase;
        vector<float> index_threshold;
        vector<float> index_reliability;
        float max_error;
        int bindw;
    
    public:
        histogram_based_dp_detector();
        virtual void read(string filename);
        virtual void train(vector<Mat>& src, vector<phase>& labels);
        virtual float eval(vector<Mat>& src, vector<phase>& labels);
        virtual void detect(vector<Mat>& src, vector<phase>& dst);

    
    private:
        void get_histogram(Mat& a, vector<float>& h);
        float distance(Mat& a, Mat& b);
        float distance(vector<float>& ha, vector<float>& hb);

        float eval(vector< vector<float> >& src, vector<phase>& labels);
        void detect(vector< vector<float> >& src, vector<phase>& dst);
        
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
        
};

#endif
