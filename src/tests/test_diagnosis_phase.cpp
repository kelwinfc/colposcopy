#include "test_diagnosis_phase.hpp"

using namespace colposcopy;

int rows = 64;
int cols = 64;

void get_sequence(const char* filename,
                  vector<Mat>& images,
                  vector<diagnosis_phase_detector::phase>& labels,
                  int mod_rate
                 )
{
    anonadado::instance inst;
    inst.read(filename);
    vector<int> step_index;
    vector<int> roi_index;
    
    inst.get_annotations("diagnosis_step", step_index);
    inst.get_annotations("roi", roi_index);
    
    int num_frames = inst.num_frames();
    
    images.clear();
    labels.clear();
    
    for ( int f = 0; f < num_frames; f++ ){
        if ( f % mod_rate != 0 ){
            continue;
        }
        
        annotation* a = inst.get_active_annotation(step_index[0], f);
        annotation* roi = inst.get_active_annotation(roi_index[0], f);
        
        if ( !a )
            continue;
        
        anonadado::choice_feature* step_feature =
                            (anonadado::choice_feature*)a->get_feature("step");
        /*
        if ( diagnosis_phase_detector::string_to_phase(
                                            step_feature->get_value()) ==
             diagnosis_phase_detector::diagnosis_transition
           )
        {
            continue;
        }
        */
        Mat img, aux;
        inst.get_frame(f, img);
        
        if ( !img.data ){
            continue;
        }
        
        labels.push_back(diagnosis_phase_detector::string_to_phase(
                                                   step_feature->get_value()));
        
        
        anonadado::bbox_feature* roi_feature =
                            (anonadado::bbox_feature*)roi->get_feature("roi");
        BBOX roi_value = roi_feature->get_value();
        
        
        Rect region_of_interest =
            Rect(roi_value.first.first,
                 roi_value.first.second,
                 roi_value.second.first,
                 roi_value.second.second);
        
        img = img(region_of_interest);
        resize(img, aux, Size(rows, cols));
        images.push_back(aux);
    }
}

void get_videos(const char* filename, vector<string>& videos)
{
    ifstream fin(filename);
    string f;
    
    videos.clear();
    while ( getline(fin, f) ){
        videos.push_back(f);
    }
}

void all_but_one(vector< vector<Mat> >& images,
                 vector< vector<diagnosis_phase_detector::phase> >& labels,
                 vector<Mat>& training_images,
                 vector<diagnosis_phase_detector::phase>& training_labels,
                 size_t k)
{
    training_images.clear();
    training_labels.clear();
    
    for ( size_t i = 0; i < images.size(); i++ ){
        if ( i == k ){
            continue;
        }
        
        training_images.insert(training_images.end(),
                               images[i].begin(), images[i].end());
        training_labels.insert(training_labels.end(),
                               labels[i].begin(), labels[i].end());
    }
}

int main(int argc, const char* argv[])
{
    cout << "Hi!\n";
    
    argc--;
    argv++;
    if ( argc == 0 ){
        return -1;
    }

    int mod_rate = 1;
    
    vector<string> videos;
    get_videos(argv[0], videos);
    
    vector< vector<Mat> > images;
    vector< vector< diagnosis_phase_detector::phase> > labels;
    
    cout << "Loading videos... " << endl;
    for ( size_t i = 0; i < videos.size(); i++ ){
        vector<Mat> next_images;
        vector<diagnosis_phase_detector::phase> next_labels;
        
        get_sequence(videos[i].c_str(), next_images, next_labels, mod_rate);
        
        images.push_back(next_images);
        labels.push_back(next_labels);
        
        cout << i+1 << "/" << videos.size() << endl;
    }
    
    for ( size_t i = 0; i < videos.size(); i++ ){
        cout << "Testing with video (" << i << ") " << videos[i] << endl;
        
        vector<Mat> training_images;
        vector<diagnosis_phase_detector::phase> training_labels;
        
        all_but_one(images, labels, training_images, training_labels, i);
        
        //incremental_nbc incr_eucl;
        knn incr_eucl;
        hue_histogram_fe f;
        circular_emd d;
        //euclidean_distance d;
        //hi_distance d;
        incr_eucl.set_feature_extractor(&f);
        incr_eucl.set_distance(&d);
        
        classifier_dpd hd;
        hd.set_classifier(&incr_eucl);
        
        //hd.train(training_images, training_labels);
        
        w_dpd whd(&hd, 3);
        context_dpd cwhd(&whd);
        
        threshold_cl thrs;
        motion_fe f_cl(5, 1, 1);
        thrs.set_feature_extractor(&f_cl);
        thrs.set_threshold(500000);
        classifier_dpd motion_cl;
        motion_cl.set_classifier(&thrs);
        
        final_dpd dpd(&motion_cl, &cwhd, 3);
        dpd.train(training_images, training_labels);
        
        stringstream ss;
        string name;
        ss << i;
        ss >> name;
        
        cout << "Training done\n";
        /*
        hd.visualize(images[i], labels[i], "results/phase_timeline/" + name + "_0_histogram.jpg");
        whd.visualize(images[i], labels[i], "results/phase_timeline/" + name + "_1_w.jpg");
        cwhd.visualize(images[i], labels[i], "results/phase_timeline/" + name + "_2_context.jpg");
        */
        dpd.visualize(images[i], labels[i], "results/phase_timeline/" + name + "_final.jpg");
        
        float error = dpd.print_confussion_matrix(images[i], labels[i]);
        cout << "Test error: " << error << endl;
    }
    /*
    //knn_dpd hd;
    
    get_sequence(argv[1], test_images, test_labels, 1);
    
    hd.visualize(test_images, test_labels,
                 "results/phase_timeline/test_0_histogram.jpg");
    whd.visualize(test_images, test_labels,
                  "results/phase_timeline/test_1_w.jpg");
    cwhd.visualize(test_images, test_labels,
                   "results/phase_timeline/test_2_context.jpg");
    ucwhd.visualize(test_images, test_labels,
                   "results/phase_timeline/test_3_unknown.jpg");
    */
}
