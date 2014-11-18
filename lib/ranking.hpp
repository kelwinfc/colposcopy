#ifndef __MODELS_RANKING
#define __MODELS_RANKING

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
#include <iostream>
#include <cstdlib>
#include <ctime>

#include "utils.hpp"

using namespace std;

namespace rank_learning {

    typedef std::vector<float> sample;

    class ranking {
        protected:
            std::vector<float> w;
            float beta;
            size_t num_iterations;
        
        public:
            ranking();
            ranking(float B=0.5, size_t num_iterations=100);
            
            ~ranking();

            virtual void train(std::vector<sample>& samples,
                               std::vector< pair<int, int> >& feedback);
            virtual float predict(sample& a, sample& b);
            virtual void rank(std::vector<sample>& samples);
            virtual void rank(std::vector<sample>& samples,
                              std::vector<int>& positions);
            
            virtual float accuracy(std::vector<sample>& samples,
                                   std::vector< pair<int, int> >& feedback);
        protected:
            float R(sample& a, sample& b, int i);
            float Z();
            void normalize_weights();
            
            void loss(std::vector<sample>& samples,
                      std::vector< pair<int, int> >& feedback,
                      std::vector<float>& loss
                     );
            void update_weights(vector<float>& loss);
            
            void greedy_order(std::vector<sample>& samples,
                              std::vector<int>& total_order);
            
            float compute_greedy_V(vector<sample>& samples,
                                   int v, vector<bool>& taken);
            void update_V(vector<sample>& samples, vector<float>& V,
                          vector<bool>& taken);
    };
};

#endif
