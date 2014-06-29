#include "test_classifiers.hpp"

void get_tokens(string line, vector<string>& tokens)
{
    stringstream tokenizer;
    string token;
    
    tokens.clear();
    tokenizer << line;
    
    while( getline(tokenizer, token, ',') ){
        tokens.push_back(token);
    }
}

void parse_dataset(string filename, vector<Mat>& input, vector<label>& labels)
{
    ifstream fin(filename.c_str());
    string line;
    
    while ( getline(fin, line) ){
        vector<string> tokens;
        get_tokens(line, tokens);
        
        Mat sample;
        sample.create(1, tokens.size() - 1, CV_32F);
        int next_label = UNKNOWN;
        
        for ( size_t i = 0; i + 1 < tokens.size(); i++ ){
            sscanf(tokens[i].c_str(), "%f", &sample.at<float>(0, i));
        }
        
        sscanf(tokens.back().c_str(), "%d", &next_label);
        
        input.push_back(sample);
        labels.push_back(next_label);
    }
}

void normalize_data(vector<Mat>& input)
{
    if (input.size() == 0 ){
        return;
    }
    
    vector<float> max_values;
    vector<float> min_values;
    
    for ( int c = 0; c < input[0].cols; c++ ){
        max_values.push_back(input[0].at<float>(0, c));
        min_values.push_back(input[0].at<float>(0, c));
    }
    
    for ( size_t i = 1; i < input.size(); i++ ){
        for ( int c = 0; c < input[i].cols; c++ ){
            max_values[c] = max(max_values[c], input[i].at<float>(0, c));
            min_values[c] = min(min_values[c], input[i].at<float>(0, c));
        }
    }
    
    for ( size_t i = 1; i < input.size(); i++ ){
        for ( int c = 0; c < input[i].cols; c++ ){
            
            float diff_c = max_values[c] - min_values[c];
            
            if ( diff_c == 0.0 ){
                continue;
            }
            
            input[i].at<float>(0, c) = 
                (input[i].at<float>(0, c) - min_values[c]) / diff_c;
            
            //cout << input[i].at<float>(0, c) << " ";
        }
        //cout << endl;
    }
}

void split_data(int k, int K,
                vector< pair<Mat, label> >& dataset,
                vector<Mat>& training_data, vector<label>& training_labels,
                vector<Mat>& test_data, vector<label>& test_labels)
{
    training_data.clear();
    training_labels.clear();
    test_data.clear();
    test_labels.clear();
    
    map<label, int> seen;
    
    for ( size_t i = 0; i < dataset.size(); i++ ){
        
        label nlabel = dataset[i].second;
        
        if ( seen.find(nlabel) == seen.end() ){
            seen[nlabel] = 0;
        }
        
        if ( seen[nlabel] % K == k ){
            test_data.push_back(dataset[i].first);
            test_labels.push_back(dataset[i].second);
        } else {
            training_data.push_back(dataset[i].first);
            training_labels.push_back(dataset[i].second);
        }
        
        seen[nlabel]++;
    }
}

map<label, map<label, size_t> > get_confusion_matrix(vector<label>& labels)
{
    map<label, map<label, size_t> > ret;
    set<label> ll;
    
    for ( size_t i = 0; i < labels.size(); i++ ){
        ll.insert(labels[i]);
    }
    ll.insert(UNKNOWN);
    
    for ( set<label>::iterator it = ll.begin(); it != ll.end(); ++it ){
        label i = *it;
        map<label, size_t> next_m;
        
        for ( set<label>::iterator it2 = ll.begin(); it2 != ll.end(); ++it2 ){
            label j = *it2;
            
            next_m[j] = 0;
        }
        
        ret[i] = next_m;
    }
    
    return ret;
}

void accumulate_confusion_matrix(map<label, map<label, size_t> >& matrix,
                                 vector<label>& predicted_labels,
                                 vector<label>& test_labels)
{
    for ( size_t i = 0; i < predicted_labels.size(); i++ ){
        matrix[test_labels[i]][predicted_labels[i]]++;
    }
}

void print_confusion_matrix(map<label, map<label, size_t> >& matrix)
{
    map<label, map<label, size_t> >::iterator it;
    vector<label> labels;
    
    for ( it = matrix.begin(); it != matrix.end(); ++it ){
        labels.push_back(it->first);
    }
    
    sort(labels.begin(), labels.end());
    
    size_t p=0, t=0;
    
    for ( size_t i = 0; i < labels.size(); i++ ){
        printf("%3d:", labels[i]);
        
        for ( size_t j = 0; j < labels.size(); j++ ){
            printf(" %3d", matrix[labels[i]][labels[j]]);
            t += matrix[labels[i]][labels[j]];
        }
        
        p += matrix[labels[i]][labels[i]];
        
        cout << endl;
    }
    cout << "Accuracy: " <<  ((float)p)/((float)t) << endl;
}

void k_fold_cross_validation(incremental_nbc& cl,
                             int K,
                             vector<Mat>& input,
                             vector<label>& labels)
{
    vector< pair<Mat, label> > dataset;
    
    for ( size_t i = 0; i < input.size(); i++ ){
        dataset.push_back( make_pair(input[i], labels[i]) );
    }
    
    random_shuffle(dataset.begin(), dataset.end());
    
    map<label, map<label, size_t> > confusion_matrix = 
                                                get_confusion_matrix(labels);
    
    for ( int k = 0; k < K; k++ ){
        vector<Mat> training_data, test_data;
        vector<label> training_labels, test_labels, predicted_labels;
        
        split_data(k, K, dataset,
                   training_data, training_labels,
                   test_data, test_labels);
        
        cl.untrain();
        cl.train(training_data, training_labels);
        cout << cl.index_size() << " ";
        flush(cout);
        
        cl.detect(test_data, predicted_labels);
        accumulate_confusion_matrix(confusion_matrix,
                                    predicted_labels,
                                    test_labels);
    }
    cout << endl;
    
    print_confusion_matrix(confusion_matrix);
}

int main(int argc, const char* argv[])
{
    argc--;
    argv++;
    
    vector<Mat> input;
    vector<label> labels;
    parse_dataset(argv[0], input, labels);
    normalize_data(input);
    int k = 5;
    
    cout << input.size() << endl;
    
    incremental_nbc incr_eucl;
    knn knn_eucl(10);
    
    identity_fe f;
    euclidean_distance eucl_d;
    
    incr_eucl.set_feature_extractor(&f);
    incr_eucl.set_distance(&eucl_d);
    
    knn_eucl.set_feature_extractor(&f);
    knn_eucl.set_distance(&eucl_d);
    
    
    cout << endl << "===== KNN - " << "Euclidean" << " =====" << endl;
    k_fold_cross_validation(knn_eucl, k, input, labels);
    
    cout << endl << "===== Incremental - " << "Euclidean" << " =====" << endl;
    k_fold_cross_validation(incr_eucl, k, input, labels);
    
    cout << "Bye!\n";
    return 0;
}
