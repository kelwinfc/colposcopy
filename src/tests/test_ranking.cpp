#include "test_ranking.hpp"

// #include "contrib/anonadado/anonadado.hpp"
// #include "contrib/anonadado/anonadado.cpp"
// #include "contrib/anonadado/utils.hpp"
// #include "contrib/anonadado/utils.cpp"

using namespace rank_learning;

void get_video(string instance_filename,
               vector<Mat>& sequence,
               vector<diagnosis_phase_detector::phase>& steps
              )
{
    anonadado::instance inst;
    cout << instance_filename << endl;
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
    
    threshold_srd thrs(155, 2);
    specular_reflection_fe sr(&thrs);
    color_cascade_fe cc(3, false);
    hsv_fe hsv(&thrs);
    closest_transition_fe ct;
    
    map<int, string>::iterator it;
    for (it = videos.begin(); it != videos.end(); ++it){
        vector<Mat> sequence;
        vector<diagnosis_phase_detector::phase> steps;
        
        get_video(it->second, sequence, steps);
        anonadado::instance instance;
        instance.read(it->second);
        
        vector<float> out;
        cout << "Loading done\n";
        
        for ( int i=0; i<sequence.size(); i++){
            //sr.extract(sequence, i, out, &instance);
            //cc.extract(sequence, i, out, &instance);
            //hsv.extract(sequence, i, out, &instance);
            ct.extract(sequence, i, out, &instance);
            
            for (int j=0; j<out.size(); j++){
                cout << out[j] << " ";
            }
            cout << endl;
            
            imshow("img", sequence[i]);
            waitKey(0);
        }
    }
    
    cout << frames.size() << " frames" << endl;
    cout << feedback.size() << " annotations" << endl;

    ranking r(0.3, 1000);

    r.train(samples, feedback);
    r.rank(samples);

    cout << "Bye!\n";
    
    return 0;
}