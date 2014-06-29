#ifndef __MODELS_NEIGHBORS_PHASE_DISTANCE
#define __MODELS_NEIGHBORS_PHASE_DISTANCE

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

class v_distance {
    public:
        v_distance();
        virtual float d(vector<float>& a, vector<float>& b);

        virtual void read(string filename);
        virtual void read(const rapidjson::Value& json);
        virtual void write(string filename);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
};

class lk_distance : public v_distance {
    protected:
        int k;
        float k_inv;

    public:
        lk_distance();
        lk_distance(int k);
        virtual float d(vector<float>& a, vector<float>& b);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
};

class manhattan_distance : public lk_distance {
    public:
        manhattan_distance();

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
};

class euclidean_distance : public lk_distance {
    public:
        euclidean_distance();

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
};

class hi_distance : public v_distance {
    public:
        hi_distance();
        virtual float d(vector<float>& a, vector<float>& b);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
};

class earth_movers_distance : public v_distance {
    public:
        earth_movers_distance();
        
        virtual float d(vector<float>& a, vector<float>& b);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
    
    protected:
        float shifted_d(vector<float>& a, vector<float>& b, size_t shift);
};

class circular_emd : public earth_movers_distance {
    public:
        circular_emd();
        
        virtual float d(vector<float>& a, vector<float>& b);

        virtual void read(const rapidjson::Value& json);
        virtual void write(rapidjson::Value& json, rapidjson::Document& d);
};

#endif
