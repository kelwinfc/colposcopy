#include "classifier.hpp"

/*****************************************************************************
 *                                Classifier                                 *
 *****************************************************************************/

classifier::classifier()
{
    this->extractor = 0;
}

classifier::~classifier()
{
    
}

void classifier::read(const rapidjson::Value& json)
{

}

void classifier::write(rapidjson::Value& json, rapidjson::Document& d)
{

}

void classifier::train(vector<Mat>& src, vector<label>& labels)
{

}

float classifier::eval(vector<Mat>& src, vector<label>& labels)
{
    return 1.0;
}

void classifier::detect(vector<Mat>& src, vector<label>& dst)
{
    dst.resize(src.size());
    fill(dst.begin(), dst.end(), UNKNOWN);
}

/*****************************************************************************
 *                       Neighborhood-based Classifier                       *
 *****************************************************************************/

neighborhood_based_classifier::neighborhood_based_classifier()
{
    this->extractor = 0;
    this->distance = 0;
}

neighborhood_based_classifier::~neighborhood_based_classifier()
{

}

void neighborhood_based_classifier::read(const rapidjson::Value& json)
{

}

void neighborhood_based_classifier::write(rapidjson::Value& json,
                                          rapidjson::Document& d)
{

}

void neighborhood_based_classifier::train(vector<Mat>& src,
                                          vector<label>& labels)
{

}

float neighborhood_based_classifier::eval(vector<Mat>& src,
                                          vector<label>& labels)
{
    return 1.0;
}

void neighborhood_based_classifier::detect(vector<Mat>& src,
                                           vector<label>& dst)
{
    dst.resize(src.size());
    fill(dst.begin(), dst.end(), UNKNOWN);
}

/*****************************************************************************
 *                          Incremental Classifier                           *
 *****************************************************************************/

incremental_nbc::incremental_nbc()
{
    this->max_error = 0.01;
    this->bindw = 5;
    this->max_samples = 20;
    this->extractor = 0;
    this->distance = 0;
}

void incremental_nbc::read(const rapidjson::Value& json)
{
    
}

void incremental_nbc::write(rapidjson::Value& json, rapidjson::Document& d)
{

}

float incremental_nbc::eval(vector<Mat>& src, vector<label>& labels)
{
    vector< vector<float> > hists;
    this->extract_features(src, hists);
    return this->eval(hists, labels);
}

void incremental_nbc::detect(vector<Mat>& src, vector<label>& dst)
{
    vector< vector<float> > f;
    this->extract_features(src, f);
    this->detect(f, dst);
}

float incremental_nbc::mdistance(Mat& a, Mat& b)
{
    vector<float> ha, hb;
    vector<Mat> va, vb;
    va.push_back(a);
    vb.push_back(b);

    this->extractor->extract(va, 0, ha);
    this->extractor->extract(vb, 0, hb);

    return this->distance->d(ha, hb);
}

float incremental_nbc::compute_threshold(vector<Mat>& src,
                                         vector<label>& labels,
                                         vector< vector<float> >& h,
                                         int index)
{
    vector< pair<float, int> > q;
    uint n = labels.size();
    uint misclassified = 0;

    for ( uint i = 0; i < n; i++ ){
        if ( labels[i] != UNKNOWN ){
            misclassified++;
        }
    }

    for ( uint i = 0; i < n; i++ ){
        q.push_back( make_pair(this->distance->d(h[index], h[i]), i) );
    }
    sort(q.begin(), q.end());

    int best_threshold = 0;
    float min_error = (float)misclassified / (float)n;

    for ( uint i = 0; i < n; i++ ){
        if ( labels[q[i].second] != labels[index] ){
            misclassified++;
        } else {
            misclassified--;
        }

        float next_error = (float)misclassified / (float)n;
        if ( next_error < min_error ){
            best_threshold = i;
            min_error = next_error;
        }
    }

    return q[best_threshold].first;
}

void incremental_nbc::compute_thresholds(vector<Mat>& src,
                                         vector<label>& labels,
                                         vector< vector<float> >& h,
                                         vector<float>& threshold)
{
    uint n = labels.size();

    threshold.resize(n);
    for ( uint i = 0; i < n; i++ ){
        threshold[i] = this->compute_threshold(src, labels, h, i);
    }
}

void incremental_nbc::compute_reliability(vector<Mat>& src,
                                          vector<label>& labels,
                                          vector< vector<float> >& h,
                                          vector<float>& threshold,
                                          vector<float>& reliability)
{
    int t=0,f=0;
    uint n = labels.size();

    reliability.resize(n);
    fill(reliability.begin(), reliability.end(), 0.0);

    for ( uint index=0; index<n; index++ ){
        for ( uint i=0; i<n; i++ ){
            label predicted = UNKNOWN;
            if ( this->distance->d(h[index], h[i]) <= threshold[index] ){
                predicted = labels[index];
            }

            if ( predicted == labels[i] ){
                t++;
            } else {
                f++;
            }
        }

        reliability[index] = ((float)t) / ((float)(t + f));
    }
}

void incremental_nbc::add_to_index(vector<float>& hist,
                                   label label,
                                   float threshold,
                                   float reliability)
{
    this->index_features.push_back(hist);
    this->index_label.push_back(label);
    this->index_threshold.push_back(threshold);
    this->index_reliability.push_back(reliability);
}

void incremental_nbc::remove_last()
{
    this->index_features.pop_back();
    this->index_label.pop_back();
    this->index_threshold.pop_back();
    this->index_reliability.pop_back();
}

float incremental_nbc::eval(vector< vector<float> >& src,
                            vector<label>& labels)
{
    int n = labels.size();
    uint f = 0;

    if ( n == 0 ){
        return 0.0;
    }

    vector<label> output;
    this->detect(src, output);

    for ( int i = 0; i < n; i++ ){
        if ( output[i] != labels[i] ){
            f++;
        }
    }

    return ((float)f) / labels.size();
}

void incremental_nbc::detect(vector< vector<float> >& src,
                             vector<label>& dst)
{
    uint n = src.size();
    for ( uint i = 0; i < n; i++ ){

        map<label, float> r;
        
        bool has = false;
        for ( uint k = 0; k < this->index_features.size(); k++ ){
            float next_distance = this->distance->d(this->index_features[k],
                                                src[i]);
            if ( next_distance <= this->index_threshold[k] ){

                if ( r.find(this->index_label[k]) == r.end() ){
                    r[this->index_label[k]] = 0.0;
                }
                
                r[this->index_label[k]] += this->index_reliability[k];
                has = true;
            }
        }

        if ( has ){
            label best_label = UNKNOWN;
            float best_r = 0.0;
            map<label, float>::iterator it;
            for ( it = r.begin(); it != r.end(); ++it ){
                if ( it->second > best_r ){
                    best_label = it->first;
                    best_r = it->second;
                }
            }
            dst.push_back(best_label);
        } else {
            dst.push_back(UNKNOWN);
        }
    }
}

pair<int, float> incremental_nbc::best_frame(vector<Mat>& src,
                                             vector<label>& labels,
                                             vector<bool>& indexed,
                                             vector<vector<float> >& hists,
                                             vector<float>& threshold,
                                             vector<float>& reliability)
{
    float min_error = 1.0;
    int ret = 0;
    uint n = labels.size();

    for ( uint i = 0; i < n; i++ ){

        if ( indexed[i] || labels[i] == UNKNOWN )
        {
            continue;
        }

        this->add_to_index(hists[i], labels[i], threshold[i], reliability[i]);
        float next_error = this->eval(hists, labels);

        if ( next_error <= min_error ){
            min_error = next_error;
            ret = i;
        }

        this->remove_last();
    }

    return make_pair(ret, min_error);
}

void incremental_nbc::extract_features(vector<Mat>& src,
                                       vector<vector<float> >& h)
{
    uint n = src.size();
    for ( uint i = 0; i < n; i++ ){
        vector<float> next_h;
        this->extractor->extract(src,i, next_h);
        h.push_back(next_h);
    }
}

void incremental_nbc::get_target_frames(vector<Mat>& src,
                                        vector<label>& labels,
                                        vector<Mat>& src_train,
                                        vector<label>& labels_train)
{
    src_train.clear();
    labels_train.clear();
    uint n = src.size();

    for ( uint i = 0; i < n; i++ ){
        if ( labels[i] != UNKNOWN )
        {
            src_train.push_back(src[i]);
            labels_train.push_back(labels[i]);
        }
    }
}

void incremental_nbc::train(vector<Mat>& src, vector<label>& labels)
{
    #if __COLPOSCOPY_VERBOSE
        cout << "Training\n";
    #endif

    vector<Mat> src_train;
    vector<label> labels_train;

    this->get_target_frames(src, labels, src_train, labels_train);

    vector<bool> indexed;
    vector<float> threshold, reliability;
    vector< vector<float> > hists;

    int n = labels_train.size();
    float current_error = 1.0;
    float previous_error = -1.0;

    indexed.resize(n);
    fill(indexed.begin(), indexed.end(), false);

    this->extract_features(src_train, hists);

    #if __COLPOSCOPY_VERBOSE
        cout << "Histograms computed\n";
    #endif

    this->compute_thresholds(src_train, labels_train, hists, threshold);

    #if __COLPOSCOPY_VERBOSE
        cout << "Thresholds computed\n";
    #endif

    this->compute_reliability(src_train, labels_train, hists,
                              threshold, reliability);

    #if __COLPOSCOPY_VERBOSE
        cout << "Reliability computed\n";
        if ( system("rm results/label_index/*.jpg") ){
            fprintf(stderr,
                    "Error: unable to rm  results/label_index/*.jpg\n");
        }
    #endif

    while ( current_error > this->max_error &&
            this->index_features.size() < src_train.size() &&
            ( previous_error < 0.0 || previous_error > current_error ) &&
            (this->max_samples < 0 || (int)this->index_features.size() <
                                           this->max_samples)
          )
    {
        previous_error = current_error;

        pair<int, float> best = this->best_frame(src_train, labels_train,
                                                 indexed, hists, threshold,
                                                 reliability);

        if ( best.second < 0 ){
            break;
        }

        this->add_to_index(hists[best.first], labels_train[best.first],
                           threshold[best.first], reliability[best.first]);
        indexed[best.first] = true;
        current_error = best.second;

        #if __COLPOSCOPY_VERBOSE
            printf("Include (class %d) %s: %0.4f (total: %d)\n",
                   labels_train[best.first],
                   spaced_d(best.first, num_chars(src_train.size())).c_str(),
                   current_error,
                   (int)this->index_features.size()
                  );

            stringstream ss;
            string filename;
            ss <<  "results/label_index/"
               << this->index_features.size() << ".jpg";
            ss >> filename;
            imwrite(filename, src_train[best.first]);
        #endif
    }

    #if __COLPOSCOPY_VERBOSE
        cout << "Done\n";
    #endif
}