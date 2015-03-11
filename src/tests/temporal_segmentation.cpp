#include "temporal_segmentation.hpp"

using namespace colposcopy;

int bin_width = 1;
int ph_mod_rate = 1;
int num_neighbors = 5;
int num_indexed_frames = 16;
int rows = 64;
int cols = 64;

void get_sequence(const char* path, int num_frames, vector<Mat>& images)
{
    for ( int f = 0; f < num_frames; f++ ){
        string filename;
        stringstream ss;
        ss << path << f << ".jpg";
        ss >> filename;
        
        Mat img, aux;
        img = imread(filename);

        if ( !img.data ){
            continue;
        }

        resize(img, aux, Size(rows, cols));
        images.push_back(aux);
    }
}

int main(int argc, const char* argv[])
{
    argc--;
    argv++;

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
    knn_manhattan.read((string)"src/demos/temporal/knn.model");

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

    int num_images;
    const char* path;
    vector<Mat> images;
    vector<diagnosis_phase_detector::phase> labels;

    sscanf(argv[1], "%d", &num_images);
    path = argv[0];

    get_sequence(path, num_images, images);
    temp_manhattan.detect(images, labels);

    for(size_t i = 0; i < labels.size(); i++){
        cout << labels[i] << endl;
    }

    return 0;
}