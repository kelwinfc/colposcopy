#ifndef __CERVIX_SEGMENTATION
#define __CERVIX_SEGMENTATION

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

#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <queue>
#include <set>

#include "utils.hpp"

using namespace std;
using namespace cv;

class cervix_segmentation {
    public:
        cervix_segmentation();
        ~cervix_segmentation();

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual void segment(vector<Mat>& src, int i, Mat& dst);
        virtual float segment(Mat& src, Mat& dst);

};

class watershed_cs : public cervix_segmentation {
    protected:
        uchar markers_threshold_min;
        uchar markers_threshold_max;
        uchar markers_bg_threshold_min;
        uchar markers_bg_threshold_max;
    
    public:
        watershed_cs(uchar markers_threshold_min=100,
                     uchar markers_threshold_max=255,
                     uchar markers_bg_threshold_min=1,
                     uchar markers_bg_threshold_max=128);
        ~watershed_cs();

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual float segment(Mat& src, Mat& dst);
};

class blobs_cs : public cervix_segmentation {
    protected:
        float min_size;
        int num_max_blobs;
        cervix_segmentation* underlying;
    
    public:
        blobs_cs(cervix_segmentation* a=0, float min_size=0.0,
                 int num_max_blobs=-1);
        ~blobs_cs();

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual float segment(Mat& src, Mat& dst);
    
    protected:
        void filter_blobs(Mat& src, Mat& dst);
};

class convex_hull_cs : public cervix_segmentation {
    protected:
        cervix_segmentation* underlying;
    
    public:
        convex_hull_cs(cervix_segmentation* a=0);
        ~convex_hull_cs();

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
        
        virtual float segment(Mat& src, Mat& dst);
    
    protected:
        void get_convex_hull(Mat& src, Mat& dst);
};

#endif
