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

#include <fstream>
#include <algorithm>
#include <vector>
#include <queue>
#include <set>

#include "utils.hpp"

using namespace std;
using namespace cv;

class feature_extractor {
    public:
        feature_extractor();
        virtual void extract(vector<Mat>& in, int i, vector<float>& out);

        virtual void read(string filename);
        virtual void write(string filename);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
};

class identity_fe : public feature_extractor {
    public:
        identity_fe();
        virtual void extract(vector<Mat>& in, int i, vector<float>& out);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
};

class hue_histogram_fe : public feature_extractor {
    protected:
        int bindw;
        bool normalize;

    public:
        hue_histogram_fe();
        hue_histogram_fe(int bindw, bool normalize);
        virtual void extract(vector<Mat>& in, int i, vector<float>& out);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
};

#endif
