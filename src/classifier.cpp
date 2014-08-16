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

void classifier::untrain()
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

label classifier::predict(Mat& src)
{
    return UNKNOWN;
}

void classifier::set_feature_extractor(feature_extractor* f)
{
    this->extractor = f;
}

void classifier::extract_features(vector<Mat>& src,
                                  vector<vector<float> >& h)
{
    uint n = src.size();
    for ( uint i = 0; i < n; i++ ){
        vector<float> next_h;
        this->extractor->extract(src,i, next_h);
        h.push_back(next_h);
    }
}

void classifier::get_confusion_matrix(vector<Mat>& src, vector<label>& labels,
                                      map< pair<label, label>, int>& matrix)
{
    vector<label> predictions;
    this->detect(src, predictions);
    
    set<label> keys;
    
    for ( size_t i = 0; i < labels.size(); i++ ){
        keys.insert(labels[i]);
    }
    for ( size_t i = 0; i < predictions.size(); i++ ){
        keys.insert(predictions[i]);
    }
    
    matrix.clear();
    
    for(set<label>::iterator it = keys.begin(); it != keys.end(); ++it){
        for(set<label>::iterator it2 = keys.begin(); it2 != keys.end(); ++it2){
            matrix[ make_pair(*it, *it2) ] = 0;
        }
    }
    
    for ( size_t i = 0; i < labels.size(); i++ ){
        matrix[make_pair(labels[i], predictions[i])]++;
    }
}

void classifier::print_confusion_matrix(vector<Mat>& src, vector<label>& labels)
{
    map< pair<label, label>, int> m;
    this->get_confusion_matrix(src, labels, m);
    
    set<label> keys_set;
    
    for(map<pair<label, label>, int>::iterator it = m.begin(); it != m.end();
        ++it)
    {
        keys_set.insert(it->first.first);
    }
    
    vector<label> keys;
    for(set<label>::iterator it = keys_set.begin(); it != keys_set.end(); ++it)
    {
        keys.push_back(*it);
    }
    
    sort(keys.begin(), keys.end());
    
    for ( size_t i = 0; i < keys.size(); i++ ){
        printf("%4d:", keys[i]);
        for ( size_t j = 0; j < keys.size(); j++ ){
            printf(" %5d", m[make_pair(keys[i], keys[j])]);
        }
        cout << endl;
    }
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

void neighborhood_based_classifier::untrain()
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

label neighborhood_based_classifier::predict(Mat& src)
{
    return UNKNOWN;
}

void neighborhood_based_classifier::set_distance(v_distance* d)
{
    this->distance = d;
}

/*****************************************************************************
 *                          Incremental Classifier                           *
 *****************************************************************************/

incremental_nbc::incremental_nbc()
{
    this->max_error = 0.01;
    this->max_samples = 15;
    this->extractor = 0;
    this->distance = 0;
    this->min_convergence = 0.005;
}

void incremental_nbc::read(const rapidjson::Value& json)
{
    
}

void incremental_nbc::write(rapidjson::Value& json, rapidjson::Document& d)
{

}

float incremental_nbc::eval(vector<Mat>& src, vector<label>& labels)
{
    vector< vector<float> > features;
    this->extract_features(src, features);
    return this->eval(features, labels);
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

void incremental_nbc::add_to_index(vector<float>& features,
                                   label label,
                                   float threshold,
                                   float reliability)
{
    this->index_features.push_back(features);
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
                
                r[this->index_label[k]] = max(r[this->index_label[k]],
                                              this->index_reliability[k]);
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
                                             vector<vector<float> >& features,
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

        this->add_to_index(features[i], labels[i], threshold[i],
                           reliability[i]);
        float next_error = this->eval(features, labels);

        if ( next_error <= min_error ){
            min_error = next_error;
            ret = i;
        }

        this->remove_last();
    }

    return make_pair(ret, min_error);
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

static int training_fold = 0;

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
    vector< vector<float> > features;

    int n = labels_train.size();
    float current_error = 1.0;
    float previous_error = -1.0;

    indexed.resize(n);
    fill(indexed.begin(), indexed.end(), false);

    this->extract_features(src_train, features);

    #if __COLPOSCOPY_VERBOSE
        cout << "Features computed\n";
    #endif

    this->compute_thresholds(src_train, labels_train, features, threshold);

    #if __COLPOSCOPY_VERBOSE
        cout << "Thresholds computed\n";
    #endif

    this->compute_reliability(src_train, labels_train, features,
                              threshold, reliability);

    #if __COLPOSCOPY_VERBOSE
        cout << "Reliability computed\n";
    #endif

    while ( current_error > this->max_error &&
            this->index_features.size() < src_train.size() &&
            ( previous_error < 0.0 || 
                (previous_error > current_error && 
                 abs(previous_error - current_error) > this->min_convergence
                )
            ) &&
            (this->max_samples < 0 || (int)this->index_features.size() <
                                           this->max_samples)
          )
    {
        previous_error = current_error;

        pair<int, float> best = this->best_frame(src_train, labels_train,
                                                 indexed, features, threshold,
                                                 reliability);

        if ( best.second < 0 ){
            break;
        }

        this->add_to_index(features[best.first], labels_train[best.first],
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
            ss << "results/label_index/"
               << training_fold << "_"
               << this->index_features.size() << ".jpg";
            ss >> filename;
            imwrite(filename, src_train[best.first]);
        #endif
    }
    
    training_fold++;
    
    #if __COLPOSCOPY_VERBOSE
        cout << "Done\n";
    #endif
}

void incremental_nbc::untrain()
{
    this->index_features.clear();
    this->index_label.clear();
    this->index_threshold.clear();
    this->index_reliability.clear();

}

label incremental_nbc::predict(Mat& src)
{
    vector<Mat> labels;
    vector<label> out;
    
    labels.push_back(src);
    this->detect(labels, out);
    
    return out[0];
}

uint incremental_nbc::index_size()
{
    return this->index_features.size();
}

/*****************************************************************************
 *                            K-Nearest Neighbors                            *
 *****************************************************************************/

knn::knn()
{
    this->k = 1;
}

knn::knn(int k)
{
    this->k = k;
}

void knn::set_k(int k)
{
    this->k = k;
}

void knn::read(const rapidjson::Value& json)
{
    //TODO
}

void knn::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

void knn::train(vector<Mat>& src, vector<label>& labels)
{
    #if __COLPOSCOPY_VERBOSE
        cout << "Training\n";
    #endif
    
    vector<Mat> src_train;
    vector<label> labels_train;
    
    this->get_target_frames(src, labels, src_train, labels_train);
    
    vector< vector<float> > f;
    
    int n = labels_train.size();
    
    this->extract_features(src_train, f);
    
    for ( int i = 0; i < n; i++ ){
        this->add_to_index(f[i], labels_train[i], 0.0, 1.0);
        
        if ( this->weight.find(labels_train[i]) == this->weight.end() ){
            this->weight[labels_train[i]] = 0.0;
        }
        
        this->weight[labels_train[i]] += 1.0;
    }
    
    float sum = 0.0;
    for ( map<label, float>::iterator it = this->weight.begin();
          it != this->weight.end(); ++it )
    {
         this->weight[it->first] = labels_train.size() - it->second;
         sum += this->weight[it->first];
    }
    
    #if __COLPOSCOPY_VERBOSE
        cout << "Weights: ";
    #endif
    
    for ( map<label, float>::iterator it = this->weight.begin();
          it != this->weight.end(); ++it )
    {
        this->weight[it->first] /= sum;
        
        #if __COLPOSCOPY_VERBOSE
            cout << "(" << it->first << "," << this->weight[it->first] << ") ";
        #endif  
    }
    
    #if __COLPOSCOPY_VERBOSE
        cout << endl;
        cout << "Done\n";
    #endif
}

void knn::detect(vector< vector<float> >& src, vector<label>& dst)
{
    uint n = src.size();
    for ( uint i = 0; i < n; i++ ){
        map<label, float> r;
        priority_queue< pair<float, label> > q;
        
        for ( uint j = 0; j < this->index_features.size(); j++ ){
            float next_distance = this->distance->d(this->index_features[j],
                                                    src[i]);
            q.push( make_pair(-next_distance, this->index_label[j]) );
        }
        
        int kk = this->k;
        while ( !q.empty() && kk-- > 0 ){
            
            if ( r.find(q.top().second) == r.end() ){
                r[q.top().second] = 0;
            }
            
            r[q.top().second] += this->weight[q.top().second];
            q.pop();
        }
        
        float max_occurrences = 0.0;
        dst.push_back(UNKNOWN);
        
        map<label, float>::iterator it;
        for ( it = r.begin(); it != r.end(); ++it ){
            if ( it->second > max_occurrences ){
                dst.back() = it->first;
                max_occurrences = it->second;
            }
        }
    }
}

/*****************************************************************************
 *                                 Threshold                                 *
 *****************************************************************************/

threshold_cl::threshold_cl()
{
    this->extractor = 0;
    this->k = 0;
}

threshold_cl::~threshold_cl()
{
    
}

void threshold_cl::read(const rapidjson::Value& json)
{
    //TODO
}

void threshold_cl::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

void threshold_cl::train(vector<Mat>& src, vector<label>& labels)
{
    #if __COLPOSCOPY_VERBOSE
        cout << "Training threshold\n";
    #endif
    
    if ( src.size() == 0 ){
        this->k = 0;
        return;
    }
    
    /*
     * [-inf..k) -> 0
     * [k..inf)  -> 1
     */
    
    vector< pair<float, label> > tp;
    size_t correctly_classified = 0;
    
    {
        vector< vector<float> > features;
        this->extract_features(src, features);
        
        for ( size_t i = 0; i < labels.size(); i++ ){
            tp.push_back( make_pair(features[i][0], labels[i]) );
            if ( labels[i] == 1 ){
                correctly_classified++;
            }
        }
    }
    
    sort(tp.begin(), tp.end());
    
    if ( tp.size() == 0 ){
        return;
    }
    
    this->k = tp[0].first;
    size_t best_cl = correctly_classified;
    
    for ( size_t i = 1; i < labels.size(); i++ ){
        
        if ( tp[i - 1].second == 0 ){
            correctly_classified++;
        } else {
            correctly_classified--;
        }
        
        if ( correctly_classified > best_cl ){
            this->k = tp[i].first;
            best_cl = correctly_classified;
        }
        
        //cout << i << " " << this->k << " " << correctly_classified << endl;
    }
    
    #if __COLPOSCOPY_VERBOSE
        cout << "Trained: " << this->k << " "
             << best_cl << "/" << labels.size() << endl;
    #endif
}

void threshold_cl::untrain()
{
    this->k = 0;
}

float threshold_cl::eval(vector<Mat>& src, vector<label>& labels)
{
    if ( labels.size() == 0 ){
        return 1.0;
    }
    
    map< pair<label, label>, int> matrix;
    map< pair<label, label>, int>::iterator it, end;
    
    get_confusion_matrix(src, labels, matrix);
    
    it = matrix.begin();
    end = matrix.end();
    
    int t = 0;
    int f = 0;
    
    for ( ; it != end; ++it ){
        pair<label, label> next = it->first;
        int count = it->second;
        
        if ( next.first == next.second ){
            t += count;
        } else {
            f += count;
        }
    }
    
    return (float)t / (float)(t + f);
}

void threshold_cl::detect(vector<Mat>& src, vector<label>& dst)
{
    vector< vector<float> > features;
    this->extract_features(src, features);
    
    dst.resize(src.size());
    
    for ( size_t i = 0; i < src.size(); i++ ){
        if ( features[i][0] >= this->k ){
            dst[i] = 1;
        } else {
            dst[i] = 0;
        }
    }
}

label threshold_cl::predict(Mat& src)
{
    vector<Mat> src_seq;
    src_seq.push_back(src);
    
    vector<label> dst_seq;
    this->detect(src_seq, dst_seq);
    
    return dst_seq[0];
}

void threshold_cl::set_threshold(int k)
{
    this->k = k;
}

void threshold_cl::log_values(vector<Mat>& src, vector<label>& labels)
{
    vector< vector<float> > features;
    this->extract_features(src, features);
    
    for ( size_t i = 0; i < src.size(); i++ ){
        this->log.push_back(make_pair(features[i][0], labels[i]));
    }
}

void threshold_cl::plot_histogram(Mat& img, int num_bins)
{
    if ( this->log.size() == 0 ){
        return;
    }
    
    float min_value = this->log[0].first;
    float max_value = this->log[0].first;
    size_t total = 0;
    
    for ( size_t i = 0; i < this->log.size(); i++ ){
        min_value = min(min_value, this->log[i].first);
        max_value = max(max_value, this->log[i].first);
        total += this->log[i].second;
    }
    
    int rows = 200;
    int cols = 400;
    img = Mat::zeros(rows + 20, cols + 20, CV_8UC3);
    
    for (int r=0; r<img.rows; r++){
        for (int c=0; c<img.cols; c++){
            img.at<Vec3b>(r, c) = Vec3b(255, 255, 255);
        }
    }
    
    vector<float> bins[2];
    
    bins[0].resize(num_bins);
    bins[1].resize(num_bins);
    fill(bins[0].begin(), bins[0].end(), 0);
    fill(bins[1].begin(), bins[1].end(), 0);
    
    sort(this->log.begin(), this->log.end());
    
    float max_height[2] = {0.0, 0.0};
    for ( size_t i = 0; i < this->log.size(); i++ ){
        float next_value = this->log[i].first;
        next_value = (next_value - min_value) / (max_value - min_value);
        
        bins[this->log[i].second][next_value * num_bins]++;
        max_height[0] = max(max_height[0],
                         bins[this->log[i].second][next_value * num_bins]);
    }
    
    Scalar colors[2] = {Scalar(255, 0, 0), Scalar(0, 0, 255)};
    
    Point this_point, prev_point;
    
    for ( int l=0; l<2; l++ ){
        for ( int b=0; b<num_bins; b++ ){
            prev_point = this_point;
            this_point = Point(10 + (float)b/(float)num_bins * cols,
                              rows + 10 - bins[l][b] / max_height[0] * rows);
            
            circle(img, this_point, 2, colors[l], -1, 8, 0);
            
            if ( b ){
                line(img, prev_point, this_point, colors[l], 1);
            }
        }
    }
}

/*****************************************************************************
 *                                   Utils                                   *
 *****************************************************************************/

classifier* classifier::from_json(const rapidjson::Value& j)
{
    //TODO
    classifier * ret = 0;
    
    return ret;
}

classifier* classifier::get(string filename)
{
    FILE * pFile = fopen (filename.c_str(), "r");
    rapidjson::FileStream is(pFile);
    rapidjson::Document d;
    d.ParseStream<0>(is);
    
    classifier * ret = classifier::from_json(d);
    
    fclose(pFile);
    
    return ret;
}