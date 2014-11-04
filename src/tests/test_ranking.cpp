#include "test_ranking.hpp"

using namespace rank_learning;

int main(int argc, const char* argv[])
{
    argc--;
    argv++;
    srand(time(NULL));

    db_ranking db;
    vector< pair<int, int> > feedback;  // Pairs of frames
    map<int, string> videos;            // Videos
    vector<db_frame> frames;            // Frame description
    map<string, int> frame_index;       // Frame index
    vector<sample> samples;             // Features extracted from frames

    db.get_videos(videos);
    db.get_annotated_frames(frames, videos, frame_index);
    db.get_feedback(frame_index, feedback);

    cout << frames.size() << " frames" << endl;
    cout << feedback.size() << " annotations" << endl;

    /*
    vector< pair<int, int> > feedback_aux, feedback;
    get_words(argv[0], words);
    get_order(words, feedback_aux);
    resample_feedback(feedback_aux, feedback, 0.6);
    
    words_to_features(words, samples);
    */
    ranking r(0.3, 1000);

    r.train(samples, feedback);
    r.rank(samples);
    /*
    vector<string> output;
    features_to_words(samples, output);
    int d = 0;
    for ( size_t i = 0; i < output.size(); i++ ){
        int next_d = distance(words, output, i);
        d += next_d;
        
        cout << next_d << " " << words[i] << " " << output[i] << endl;
    }
    
    cout << (float)d / (float)output.size() << endl;
    */
    cout << "Bye!\n";
    
    return 0;
}