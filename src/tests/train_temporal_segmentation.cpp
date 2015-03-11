#include "train_temporal_segmentation.hpp"

using namespace colposcopy;

int bin_width = 1;
int ph_mod_rate = 1;
int num_neighbors = 5;
int num_indexed_frames = 16;
int rows = 64;
int cols = 64;

void get_videos(const char* filename, vector<string>& videos)
{
    ifstream fin(filename);
    string f;
    videos.clear();
    while ( getline(fin, f) ){
        videos.push_back(f);
    }
}

int main(int argc, const char* argv[])
{
    argc--;
    argv++;
    
    vector<string> training_videos;
    vector<Mat> training_images, images;
    
    get_videos(argv[0], training_videos);
    cout << "Videos' names loaded..." << endl;

    vector< diagnosis_phase_detector::phase> training_labels, labels;
            
    // Get equidistant frames from each training video
    diagnosis_phase_detector::get_equidistant_frames(training_videos, 
                                                     training_images,
                                                     training_labels,
                                                     num_indexed_frames,
                                                     rows, cols);
    cout << "Equidistant frames loaded..." << endl;

    // KNN model
    knn knn_manhattan(num_neighbors);

    // Feature extractor
    hue_sat_histogram_fe f(bin_width, true);

    // Distances
    manhattan_distance hue_manhattan, sat_manhattan;
    vector<int> hue_sat_sizes;
    hue_sat_sizes.push_back(180 / bin_width + (180 % bin_width != 0 ? 1 : 0));
    hue_sat_sizes.push_back(256 / bin_width + (256 % bin_width != 0 ? 1 : 0));
    vector<v_distance*> manhattan_distances;
    manhattan_distances.push_back(&hue_manhattan);
    manhattan_distances.push_back(&sat_manhattan);
    merge_distances manhattan(manhattan_distances, hue_sat_sizes);

    // KNN model configuration
    knn_manhattan.set_feature_extractor(&f);
    knn_manhattan.set_distance(&manhattan);

    // Classifier
    classifier_dpd hd_manhattan;
    hd_manhattan.set_classifier(&knn_manhattan);
    w_dpd whd_manhattan(&hd_manhattan, 3);
    threshold_cl thrs;
    motion_fe f_cl(7, 1, 1);
    thrs.set_feature_extractor(&f_cl);
    thrs.set_threshold(1000000);
    classifier_dpd motion_cl;
    motion_cl.set_classifier(&thrs);
    final_dpd dpd_manhattan(&motion_cl, &hd_manhattan, ph_mod_rate);
    temporal_dpd temp_manhattan(&dpd_manhattan);

    temp_manhattan.train(training_images, training_labels);
    cout << "Model trained..." << endl;
    
    knn_manhattan.write(argv[1]);

    return 0;
}