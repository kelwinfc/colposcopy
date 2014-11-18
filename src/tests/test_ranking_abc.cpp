#include "test_ranking_abc.hpp"

using namespace rank_learning;

int blank_char = 0;

void get_words(const char* filename, vector<string>& words ){
    ifstream fin(filename);
    string next_word;
    
    words.clear();
    while( fin >> next_word ){
        words.push_back(next_word);
    }
    
    fin.close();
}

void words_to_features(vector<string>& words, vector<sample>& samples)
{
    samples.clear();
    size_t max_length = 0;
    size_t n = words.size();
    
    for ( size_t i = 0; i < n; i++ ){
        max_length = max(max_length, words[i].size());
    }
    
    for ( size_t i = 0; i < n; i++ ){
        sample next_sample;
        next_sample.resize(max_length);
        fill(next_sample.begin(), next_sample.end(), blank_char);
        
        for ( size_t j = 0; j < words[i].size(); j++ ){
            next_sample[j] = words[i][j] - 'a' + 1;
        }
        
        samples.push_back(next_sample);
    }
    
    cout << "Max length: " << max_length << endl;
}

void features_to_words(vector<sample>& samples, vector<string>& words)
{
    size_t n = samples.size();
    
    for ( size_t i = 0; i < n; i++ ){
        string w = "";
        for ( size_t j = 0; j < samples[i].size() && 
              samples[i][j] != blank_char; j++ )
        {
            w += samples[i][j] + 'a' - 1;
        }
        words.push_back(w);
    }
}

void get_order(vector<string>& words, vector< pair<int, int> >& feedback)
{
    size_t n = words.size();
    
    sort(words.begin(), words.end());
    for ( size_t i = 0; i < n; i++ ){
        for ( size_t j = 0; j < i; j++ ){
            feedback.push_back(make_pair(i,j));
        }
    }
}

void resample_feedback(vector< pair<int, int> >& in,
                       vector< pair<int, int> >& out,
                       float rate
                      )
{
    out.clear();
    random_shuffle(in.begin(), in.end());
    
    for ( size_t i = 0; i < rate * in.size(); i++ ){
        out.push_back(in[i]);
    }
}

int distance(vector<string>& words, vector<string>& output, int k)
{
    for ( int i = 0; i < (int)words.size(); i++ ){
        if ( words[i] == output[k] )
        {
            return abs(i - k);
        }
    }
    
    return max(k, (int)output.size() - k);
}

int main(int argc, const char* argv[])
{
    argc--;
    argv++;
    srand(time(NULL));

    vector<string> words;
    vector<sample> samples;
    vector< pair<int, int> > feedback_aux, feedback;

    get_words(argv[0], words);
    get_order(words, feedback_aux);
    resample_feedback(feedback_aux, feedback, 0.6);

    words_to_features(words, samples);

    ranking r(0.3, 1000);

    r.train(samples, feedback);
    r.rank(samples);

    vector<string> output;
    features_to_words(samples, output);

    int d = 0;
    for ( size_t i = 0; i < output.size(); i++ ){
        int next_d = distance(words, output, i);
        d += next_d;
        
        cout << next_d << " " << words[i] << " " << output[i] << endl;
    }

    cout << (float)d / (float)output.size() << endl;
    cout << "Bye!\n";

    return 0;
}
