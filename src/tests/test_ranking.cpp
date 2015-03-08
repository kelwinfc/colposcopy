#include "test_ranking.hpp"

using namespace rank_learning;

void remove_from_feedback(vector< pair<int, int> >& feedback,
                          vector<db_frame>& frames,
                          int video_index,
                          vector< pair<int, int> >& training,
                          vector< pair<int, int> >& test
                         )
{
    training.clear();
    test.clear();
    
    for ( size_t i = 0; i < feedback.size(); i++ ){
        int f1 = feedback[i].first;
        int f2 = feedback[i].second;
        
        if ( frames[f1].video_index == video_index ||
             frames[f2].video_index == video_index )
        {
            test.push_back(feedback[i]);
        } else {
            training.push_back(feedback[i]);
        }
    }
}

void get_video(string instance_filename,
               vector<Mat>& sequence,
               vector<diagnosis_phase_detector::phase>& steps
              )
{
    anonadado::instance inst;
    inst.read(instance_filename);

    vector<int> step_index;
    vector<int> roi_index;

    inst.get_annotations("diagnosis_step", step_index);
    inst.get_annotations("roi", roi_index);

    int num_frames = inst.num_frames();

    sequence.clear();
    
    for ( int f = 0; f < num_frames; f++ ){
        
        anonadado::annotation* a = inst.get_active_annotation(step_index[0],
                                                              f);
        
        if ( !a )
            continue;
        
        anonadado::choice_feature* step_feature =
                            (anonadado::choice_feature*)a->get_feature("step");
        
        Mat img, aux;
        inst.get_frame(f, img);

        if ( !img.data ){
            continue;
        }

        diagnosis_phase_detector::phase step = 
           diagnosis_phase_detector::string_to_phase(step_feature->get_value());
        
        sequence.push_back(img);
        steps.push_back(step);
    }
}

void print_arff(string filename, vector<sample>& samples,
                vector< pair<int, int> >& feedback)
{
    ofstream fout(filename.c_str());
    fout << "@RELATION colposcopy" << endl;
    for (int j=0; j<samples[0].size(); j++){
        fout << "@ATTRIBUTE A" << j << " REAL" << endl;
    }
    fout << "@ATTRIBUTE class {0, 1}" << endl;
    fout << "@DATA" << endl;

    for(size_t i=0; i<feedback.size(); i++){
        int f1 = feedback[i].first;
        int f2 = feedback[i].second;
        
        if ( i % 2 == 0 ){
            for (int j=0; j<samples[f1].size(); j++){
                if ( j > 0 ){
                    fout << ",";
                }
                fout << (samples[f1][j] < samples[f2][j] ? 1.0 : 0.0);
            }
            fout << "," << 1 << endl;
        } else {
            for (int j=0; j<samples[f2].size(); j++){
                if ( j > 0 ){
                    fout << ",";
                }
                fout << (samples[f2][j] < samples[f1][j] ? 1.0 : 0.0);
            }
            fout << "," << 0 << endl;
        }
    }
    fout.close();
}

void print_data(string filename, vector<sample>& samples,
                vector<db_frame>& frames,
                vector< pair<int, int> >& feedback)
{
    ofstream fout(filename.c_str());
    fout << samples.size() << endl;
    
    for (size_t i = 0; i <samples.size(); i++){
        fout << frames[i].video_index;
        for (int j=0; j<samples[i].size(); j++){
            fout << "," << samples[i][j];
        }
        fout << endl;
    }
    
    fout << feedback.size() << endl;
    for(size_t i=0; i<feedback.size(); i++){
        int f1 = feedback[i].first;
        int f2 = feedback[i].second;
        fout << f1 << " " << f2 << endl;
    }
    fout.close();
}

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

    /*
    edges_summations_fe
    */

    threshold_srd thrs(155, 2);
    specular_reflection_fe sr(&thrs);
    
    hsv_fe hsv(&thrs);
    closest_transition_fe ct;
    color_cascade_fe cc(5, true);

    watershed_cs cervix_ws;
    blobs_cs cervix_blob(&cervix_ws, 0.2, 2);
    find_hole_cs epithelium_ws(&cervix_blob);
    cervix_region_fe cr(&cervix_ws, &epithelium_ws);

    vector<feature_extractor*> fes;
    fes.push_back(&cr);     // 3
    fes.push_back(&sr);     // 3
    fes.push_back(&ct);     // 3
    fes.push_back(&hsv);    // 6
    fes.push_back(&cc);     // 62
    merge_fe mg(fes);

    add_inverse_fe ife(&mg);

    vector<string> names;
    ife.get_names(names);
    for ( size_t i = 0; i < names.size(); i++ ){
        cout << i << " " << names[i] << endl;
    }

    for (size_t i = 0; i < frames.size(); i++){
        string video_filename = videos[frames[i].video_index];
        int frame_index = frames[i].frame;
        
        anonadado::instance instance;
        instance.read(video_filename);
        
        vector<float> out;
        ife.extract_by_inst(instance, frames[i].frame, out);
        samples.push_back(out);
    }
    cout << "Features extracted\n";
    
    print_data("ranking.data", samples, frames, feedback);
    
    cout << videos.size() << " videos" << endl;
    cout << frames.size() << " frames" << endl;
    cout << feedback.size() << " annotations" << endl;
    
    map<int, string>::iterator it;
    for (it = videos.begin(); it != videos.end(); ++it){
        vector< pair<int, int> > training, test;
        remove_from_feedback(feedback, frames, it->first, training, test);
        
        cout << training.size() << " " << test.size() << endl;
        
        l2r_ranking r(0.3, 100);
        r.train(samples, training);
        cout << r.accuracy(samples, test) << endl;
    }
    cout << "Bye!\n";
    //r.rank(samples);
    return 0;
}