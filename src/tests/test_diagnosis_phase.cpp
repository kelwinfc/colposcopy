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
    
    for ( int f = 0; f < num_frames; f++ ){
        if ( f % mod_rate != 0 ){
            continue;
        }
        
        anonadado::annotation* a = 
            inst.get_active_annotation(step_index[0], f);
        anonadado::annotation* roi = 
            inst.get_active_annotation(roi_index[0], f);
        
        if ( !a || !roi )
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
    int num_neighbors = 1;
    int bin_width = 5;
    int ph_mod_rate = 10;
    
    vector<string> videos;
    get_videos(argv[0], videos);
    
    vector< vector<Mat> > images;
    vector< vector< diagnosis_phase_detector::phase> > labels;
    
    cout << "Loading videos... " << endl;
    for ( size_t i = 0; i < videos.size(); i++ ){
        int64 start_t = GetTimeMs64();
        
        vector<Mat> next_images;
        vector<diagnosis_phase_detector::phase> next_labels;
        images.push_back(next_images);
        labels.push_back(next_labels);
        
        get_sequence(videos[i].c_str(), images.back(), labels.back(), mod_rate);
        int64 end_t = GetTimeMs64();
        
        cout << i+1 << "/" << videos.size() << " "
             << (end_t - start_t) / 1000 << " sec"
             << endl;
    }
    
    for ( size_t i = 0; i < videos.size(); i++ ){
        cout << "Testing with video (" << i << ") " << videos[i] << endl;
        
        vector<Mat> training_images;
        vector<diagnosis_phase_detector::phase> training_labels;
        /*
        vector<Mat> empty_images;
        vector<diagnosis_phase_detector::phase> empty_labels;
        images.push_back(empty_images);
        labels.push_back(empty_labels);
        all_but_one(images, labels, training_images, training_labels, images.size()-1);
        */
        
        all_but_one(images, labels, training_images, training_labels, i);
        
        /* K-NN Classifiers */
        knn knn_euclidean(num_neighbors);
        knn knn_hi(num_neighbors);
        knn knn_emd(num_neighbors);
        knn knn_cemd(num_neighbors);
        
        /* Feature extractor and distances */
        hue_histogram_fe f(bin_width, true);
        
        euclidean_distance euclidean;
        hi_distance hi;
        earth_movers_distance emd;
        circular_emd cemd;
        
        knn_euclidean.set_feature_extractor(&f);
        knn_hi.set_feature_extractor(&f);
        knn_emd.set_feature_extractor(&f);
        knn_cemd.set_feature_extractor(&f);
        
        knn_euclidean.set_distance(&euclidean);
        knn_hi.set_distance(&hi);
        knn_emd.set_distance(&emd);
        knn_cemd.set_distance(&cemd);
        
        classifier_dpd hd_euclidean, hd_hi, hd_emd, hd_cemd;
        hd_euclidean.set_classifier(&knn_euclidean);
        hd_hi.set_classifier(&knn_hi);
        hd_emd.set_classifier(&knn_emd);
        hd_cemd.set_classifier(&knn_cemd);
        
        /* Windowed Classifier */
        w_dpd whd_euclidean(&hd_euclidean, 3);
        w_dpd whd_hi(&hd_hi, 3);
        w_dpd whd_emd(&hd_emd, 3);
        w_dpd whd_cemd(&hd_cemd, 3);
        
        /* Context Classifier */
        
        context_dpd cwhd_euclidean(&whd_euclidean);
        context_dpd cwhd_hi(&whd_hi);
        context_dpd cwhd_emd(&whd_emd);
        context_dpd cwhd_cemd(&whd_cemd);
        
        /* Motion classifier */
        threshold_cl thrs;
        motion_fe f_cl(5, 1, 1);
        thrs.set_feature_extractor(&f_cl);
        thrs.set_threshold(1000000);
        classifier_dpd motion_cl;
        motion_cl.set_classifier(&thrs);
        
        /* Final classifier */
        final_dpd dpd_euclidean(&motion_cl, &cwhd_euclidean, ph_mod_rate);
        final_dpd dpd_hi(&motion_cl, &cwhd_hi, ph_mod_rate);
        final_dpd dpd_emd(&motion_cl, &cwhd_emd, ph_mod_rate);
        final_dpd dpd_cemd(&motion_cl, &cwhd_cemd, ph_mod_rate);
        
        dpd_euclidean.train(training_images, training_labels);
        dpd_hi.train(training_images, training_labels);
        dpd_emd.train(training_images, training_labels);
        dpd_cemd.train(training_images, training_labels);
        
        map<label, Scalar> colors;
        colors[2] = Scalar(0, 0, 255);
        colors[3] = Scalar(0, 255, 0);
        colors[4] = Scalar(255, 0, 0);
        colors[5] = Scalar(0, 64, 128);
        
        knn_cemd.plot_histograms(colors);
        
        stringstream ss;
        string name;
        ss << i;
        ss >> name;
        
        cout << "Training done\n";
        /*
        dpd_euclidean.visualize(images[i], labels[i], "results/phase_timeline/" + name + "_euclidean.jpg");
        dpd_hi.visualize(images[i], labels[i], "results/phase_timeline/" + name + "_hi.jpg");
        dpd_emd.visualize(images[i], labels[i], "results/phase_timeline/" + name + "_emd.jpg");
        dpd_cemd.visualize(images[i], labels[i], "results/phase_timeline/" + name + "_cemd.jpg");
        */
        float error;
        
        int64 start_t, end_t;
        
        start_t = GetTimeMs64();
        error = dpd_euclidean.print_confussion_matrix(images[i], labels[i]);
        end_t = GetTimeMs64();
        cout << (end_t - start_t) / 1000 << " sec" << endl;
        cout << "Euclidean: " << error << endl;
        
        start_t = GetTimeMs64();
        error = dpd_hi.print_confussion_matrix(images[i], labels[i]);
        end_t = GetTimeMs64();
        cout << (end_t - start_t) / 1000 << " sec" << endl;
        cout << "HI: " << error << endl;
        
        start_t = GetTimeMs64();
        error = dpd_emd.print_confussion_matrix(images[i], labels[i]);
        end_t = GetTimeMs64();
        cout << (end_t - start_t) / 1000 << " sec" << endl;
        cout << "EMD: " << error << endl;
        
        start_t = GetTimeMs64();
        error = dpd_cemd.print_confussion_matrix(images[i], labels[i]);
        end_t = GetTimeMs64();
        cout << (end_t - start_t) / 1000 << " sec" << endl;
        cout << "CEMD: " << error << endl;
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
