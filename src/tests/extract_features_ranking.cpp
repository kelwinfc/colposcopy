#include "extract_features_ranking.hpp"

#include <ctime>

using namespace rank_learning;

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
        if ( img.data ){
            diagnosis_phase_detector::phase step = 
               diagnosis_phase_detector::string_to_phase(
                   step_feature->get_value());
            
            sequence.push_back(img);
            steps.push_back(step);
        }
    }
}

void get_samples_from_frames(vector<db_frame>& frames, feature_extractor* fe,
                             vector<sample>& samples)
{
    for (size_t i = 0; i < frames.size(); i++){
        sample out;
        anonadado::instance instance;
        instance.read(frames[i].video_filename);
        fe->extract_by_inst(instance, frames[i].frame, out);
        samples.push_back(out);
    }
}

feature_extractor* get_feature_extractor()
{
    threshold_srd* thrs = new threshold_srd(155, 2);
    navier_stokes_ip * t_ip = new navier_stokes_ip(10.0);
    
    specular_reflection_fe* sr = new specular_reflection_fe(thrs);
    color_cascade_fe* cc = new color_cascade_fe(4, true);
    hsv_fe* hsv = new hsv_fe(thrs);
    closest_transition_fe* ct = new closest_transition_fe();
    edges_summations_fe* es = new edges_summations_fe(thrs, t_ip);
    
    vector<feature_extractor*> single_fes, fes;
    single_fes.push_back(sr);
    single_fes.push_back(cc);
    single_fes.push_back(hsv);
    single_fes.push_back(es);
    merge_single_frame_fe* msf = new merge_single_frame_fe(single_fes);

    fes.push_back(msf);
    fes.push_back(ct);
    merge_fe* mg = new merge_fe(fes);

    add_inverse_fe* i = new add_inverse_fe(mg);
    /*
    vector<string> names;
    i->get_names(names);
    for ( size_t k = 0; k < names.size(); k++ ){
        cout << "(" << k << ":" << names[k] << ") ";
    }
    cout << endl;
    */
    return i;
}

void leave_one_patient_out(vector<db_frame>& frames, int video_index,
                           vector< pair<int, int> >& in,
                           vector< pair<int, int> >& tr,
                           vector< pair<int, int> >& ts
                          )
{
    tr.clear();
    ts.clear();
    
    size_t i = 0;
    for ( ; i < in.size(); i++ ){
        int vf = frames[in[i].first].video_index;
        int vs = frames[in[i].second].video_index;

        if ( vf == video_index || vs == video_index ){
            ts.push_back(in[i]);
        } else {
            tr.push_back(in[i]);
        }
    }
}

void resample_feedback(vector< pair<int, int> >& in,
                       vector< pair<int, int> >& tr,
                       vector< pair<int, int> >& ts,
                       float rate
                      )
{
    tr.clear();
    ts.clear();
    
    random_shuffle(in.begin(), in.end());

    size_t i = 0;
    for ( ; i < rate * in.size(); i++ ){
        tr.push_back(in[i]);
    }
    for ( ; i < in.size(); i++ ){
        ts.push_back(in[i]);
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

    diagnosis_phase_detector::phase ph = 
        diagnosis_phase_detector::diagnosis_unknown;

    if ( argc > 0 ){
        int v;
        sscanf(argv[0], "%d", &v);
        ph = (diagnosis_phase_detector::phase)v;
    }

    // Get frames and feedback
    db.get_videos(videos);
    db.get_annotated_frames(frames, videos, frame_index);
    db.get_feedback(frame_index, feedback, ph);
    
    cout << "Frames and feedback retrieved..." << endl;
    cout << videos.size() << " videos, " << frames.size() << " frames, "
         << feedback.size() << " annotations" << endl;
    
    {
        map<int, string>::iterator it=videos.begin(), it_end=videos.end();
        vector<int> nframes;
        
        for (; it != it_end; ++it)
        {
            int v = it->first;
            nframes.push_back(db.num_annotated_frames(v));
        }
        sort(nframes.begin(), nframes.end());
        for (size_t i = 0; i < nframes.size(); i++){
            cout << nframes[i] << " ";
        }
        cout << endl;
    }
    
    // Extract the features
    feature_extractor* fe = get_feature_extractor();
    get_samples_from_frames(frames, fe, samples);
    cout << "Features extracted..." << endl;

    map<int, string>::iterator it=videos.begin(), end=videos.end();
    size_t v = 0;

    float avg_tr = 0.0;
    float avg_ts = 0.0;

    for(; it != end; ++it){
        // Split feedback in training and test set
        vector< pair<int, int> > tr_feedback, ts_feedback;
        leave_one_patient_out(frames, it->first, feedback, tr_feedback,
                              ts_feedback);

        if ( tr_feedback.size() == 0 || ts_feedback.size() == 0 ){
            continue;
        }

        // Inject frames and artificial feedback
        // TODO

        // Train the model
        ranking r(0.3, 100);
        r.train(samples, tr_feedback);
        
        float tr_acc = r.accuracy(samples, tr_feedback);
        float ts_acc = r.accuracy(samples, ts_feedback);
        
        avg_tr += tr_acc;
        avg_ts += ts_acc;
        v++;
        
        cout << v << " "
             << tr_feedback.size() << " "
             << tr_acc << " "
             << ts_feedback.size() << " "
             << ts_acc << " | "
             << avg_tr / v << " " << avg_ts / v
             << endl;
    }
    
    cout << "Bye!\n";
    
    return 0;
}