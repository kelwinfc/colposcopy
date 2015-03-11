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
                 roi_value.second.first - roi_value.first.first,
                 roi_value.second.second - roi_value.first.second);
        
        resize(img, aux, Size(600, 474));
        aux.copyTo(img);

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

void resize(Mat& src, int rows, int cols)
{
    Mat aux;
    resize(src, aux, Size(rows, cols));
    aux.copyTo(src);
}

string get_name(string alg, int num_frames, int video)
{
    stringstream ss;
    string ret;
    
    ss << "results/phase_timeline/visualization/" << num_frames << "_"
       << video << "_" << alg << ".jpg";
    ss >> ret;
    return ret;
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
    int num_neighbors = 5;
    int bin_width = 1;
    int ph_mod_rate = 1;

    vector<string> videos;
    get_videos(argv[0], videos);
    
    int num_frames[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
    size_t num_experiments = 11;
    
    for (size_t exp = 1; exp < num_experiments; exp++){
        
        cout << "==== Experiment "
             << exp << " - " << num_frames[exp]
             << " ====" << endl;
        
        for ( size_t i = 0; i < videos.size(); i++ ){
            cout << "Testing with video (" << i << ") " << videos[i] << endl;
            
            vector<string> training_videos;
            vector<Mat> training_images, images;
            vector< diagnosis_phase_detector::phase> training_labels, labels;
            
            // Get equidistant frames from each training video
            all_but_k(videos, training_videos, i);
            
            diagnosis_phase_detector::get_equidistant_frames(training_videos, 
                                                             training_images,
                                                             training_labels,
                                                             num_frames[exp],
                                                             rows, cols);
            // Train the models
            
            // K-NN Classifiers
            knn knn_manhattan(num_neighbors);
            knn knn_emd(num_neighbors);
            //knn knn_cemd(num_neighbors);
            
            // Feature extractor and distances
            hue_sat_histogram_fe f(bin_width, true);
            
            manhattan_distance hue_manhattan, sat_manhattan;
            earth_movers_distance hue_emd, sat_emd;
            //circular_emd hue_cemd, sat_cemd;
            
            vector<int> hue_sat_sizes;
            hue_sat_sizes.push_back(180 / bin_width + 
                                    (180 % bin_width != 0 ? 1 : 0));
            hue_sat_sizes.push_back(256 / bin_width + 
                                    (256 % bin_width != 0 ? 1 : 0));
            
            vector<v_distance*> manhattan_distances;
            vector<v_distance*> emd_distances;
            //vector<v_distance*> cemd_distances;
            
            manhattan_distances.push_back(&hue_manhattan);
            manhattan_distances.push_back(&sat_manhattan);
            emd_distances.push_back(&hue_emd);
            emd_distances.push_back(&sat_emd);
            //cemd_distances.push_back(&hue_cemd);
            //cemd_distances.push_back(&sat_cemd);
            
            merge_distances manhattan(manhattan_distances, hue_sat_sizes);
            merge_distances emd(emd_distances, hue_sat_sizes);
            //merge_distances cemd(cemd_distances, hue_sat_sizes);
            
            knn_manhattan.set_feature_extractor(&f);
            knn_emd.set_feature_extractor(&f);
            //knn_cemd.set_feature_extractor(&f);
            
            knn_manhattan.set_distance(&manhattan);
            knn_emd.set_distance(&emd);
            //knn_cemd.set_distance(&cemd);

            classifier_dpd hd_manhattan, hd_emd, hd_cemd;
            hd_manhattan.set_classifier(&knn_manhattan);
            hd_emd.set_classifier(&knn_emd);
            //hd_cemd.set_classifier(&knn_cemd);

            // Windowed Classifier
            w_dpd whd_manhattan(&hd_manhattan, 3);
            w_dpd whd_emd(&hd_emd, 3);
            //w_dpd whd_cemd(&hd_cemd, 3);

            /*
            // Context Classifier
            context_dpd cwhd_manhattan(&whd_manhattan);
            context_dpd cwhd_emd(&whd_emd);
            //context_dpd cwhd_cemd(&whd_cemd);
            */

            // Motion classifier
            threshold_cl thrs;
            motion_fe f_cl(7, 1, 1);
            thrs.set_feature_extractor(&f_cl);
            thrs.set_threshold(1000000);
            classifier_dpd motion_cl;
            motion_cl.set_classifier(&thrs);

            // Final classifier
            final_dpd dpd_manhattan(&motion_cl, &hd_manhattan, ph_mod_rate);
            final_dpd dpd_emd(&motion_cl, &hd_emd, ph_mod_rate);
            
            // Temporal Classifier
            temporal_dpd temp_manhattan(&dpd_manhattan);
            temporal_dpd temp_emd(&dpd_emd);
            
            temp_manhattan.train(training_images, training_labels);
            temp_emd.train(training_images, training_labels);
            
            map<label, Scalar> colors;
            colors[2] = Scalar(0, 0, 255);
            colors[3] = Scalar(0, 255, 0);
            colors[4] = Scalar(255, 0, 0);
            colors[5] = Scalar(0, 64, 128);

            //knn_cemd.plot_histograms(colors);

            stringstream ss;
            string name;
            ss << i;
            ss >> name;

            int64 start_t, end_t;
            start_t = GetTimeMs64();
            get_sequence(videos[i].c_str(), images, labels, 1);
            end_t = GetTimeMs64();

            float load_sequence_t = (end_t - start_t) / 1000.;

            float error, next_time;
            /*
            dpd_manhattan.visualize(images, labels,
                                    get_name("L1", num_frames[exp], i));
            dpd_emd.visualize(images, labels,
                              get_name("EMD", num_frames[exp], i));
            temp_manhattan.visualize(images, labels,
                                     get_name("L1_T", num_frames[exp], i));
            temp_emd.visualize(images, labels,
                               get_name("EMD_T", num_frames[exp], i));
            */
            cout << "Time loading " << images.size() << " frames: "
                 << load_sequence_t << " sec "
                 << ((float)images.size() / load_sequence_t)
                 << " fps" << endl;

            // Eval the models and print the results
            start_t = GetTimeMs64();
            error = dpd_manhattan.print_confussion_matrix(images, labels);
            end_t = GetTimeMs64();
            next_time = (end_t - start_t) / 1000.;
            cout << next_time << " sec "
                 << ((float)images.size()) / 
                    (load_sequence_t + next_time)
                 << " fps" << endl;
            cout << "L$_1$: " << error << endl;
            
            start_t = GetTimeMs64();
            error = temp_manhattan.print_confussion_matrix(images, labels);
            end_t = GetTimeMs64();
            next_time = (end_t - start_t) / 1000.;
            cout << next_time << " sec "
                 << ((float)images.size()) / 
                    (load_sequence_t + next_time)
                 << " fps" << endl;
            cout << "T-L$_1$: " << error << endl;
            
            start_t = GetTimeMs64();
            error = dpd_emd.print_confussion_matrix(images, labels);
            end_t = GetTimeMs64();
            next_time = (end_t - start_t) / 1000.;
            cout << next_time << " sec "
                 << ((float)images.size()) / 
                    (load_sequence_t + next_time)
                 << " fps" << endl;
            cout << "EMD: " << error << endl;
            
            start_t = GetTimeMs64();
            error = temp_emd.print_confussion_matrix(images, labels);
            end_t = GetTimeMs64();
            next_time = (end_t - start_t) / 1000.;
            cout << next_time << " sec "
                 << ((float)images.size()) / 
                    (load_sequence_t + next_time)
                 << " fps" << endl;
            cout << "T-EMD: " << error << endl;
            /*
            start_t = GetTimeMs64();
            error = dpd_cemd.print_confussion_matrix(images, labels);
            end_t = GetTimeMs64();
            cout << (end_t - start_t) / 1000 << " sec" << endl;
            cout << "Hue-CEMD: " << error << endl;
            */
        }
    }

    return 0;
}
