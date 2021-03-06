#include "test_motion.hpp"

using namespace colposcopy;

int rows = 48;
int cols = 48;

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
        anonadado::annotation* a = 
            inst.get_active_annotation(step_index[0], f);
        anonadado::annotation* roi = 
            inst.get_active_annotation(roi_index[0], f);
        
        if ( !a ){
            continue;
        }
        
        anonadado::choice_feature* step_feature =
                            (anonadado::choice_feature*)a->get_feature("step");
        
        Mat img, aux;
        inst.get_frame(f, img);
        if ( !img.data ){
            continue;
        }
        if ( step_feature->get_value() == "transition" ){
            labels.push_back(diagnosis_phase_detector::diagnosis_transition);
        } else {
            labels.push_back(diagnosis_phase_detector::diagnosis_unknown);
        }
        
        anonadado::bbox_feature* roi_feature =
                            (anonadado::bbox_feature*)roi->get_feature("roi");
        BBOX roi_value = roi_feature->get_value();
        
        Rect region_of_interest =
            Rect((int)(img.cols / 400. * roi_value.first.first),
                 (int)(img.cols / 400. * roi_value.first.second),
                 (int)(img.rows / 600. * roi_value.second.first),
                 (int)(img.rows / 600. * roi_value.second.second));
        
        img = img(region_of_interest);
        resize(img, aux, Size(rows, cols));
        images.push_back(aux);
    }
}

float max_value = 0.0;

void show_motion(vector<float>& m, int mrows, int mcols,
                 Mat& dst, int dst_rows, int dst_cols)
{
    dst = Mat::zeros(dst_rows, dst_cols, CV_8U);
    
    float sum = 0.0;
    for (size_t i = 0; i < m.size(); i++ ){
        max_value = max(max_value, m[i]);
        sum += m[i];
    }
    cout << sum << endl;
    
    for (int r = 0; r < dst_rows; r++ ){
        for( int c = 0; c < dst_rows; c++ ){
            
            int fr = r / (dst_rows / mrows );
            int fc = c / (dst_cols / mcols );
            
            float value = m[fr * mcols + fc];
            value /= max_value + 1e-6;
            
            dst.at<uchar>(r, c) = (uchar)round(255.0 * value);
        }
    }
}

void normalize_confusion_matrix(map< pair<label, label>, int >& src,
                                map< pair<label, label>, float >& dst)
{
    map< pair<label, label>, int >::iterator it, end=src.end();
    
    int total = 0;
    for ( it = src.begin(); it != end; ++it ){
        total += it->second;
    }
    
    for ( it = src.begin() ; it != end; ++it ){
        if ( dst.find(it->first) == dst.end() ){
            dst[it->first] = 0.0;
        }
        dst[it->first] += (float)it->second / (float)total;
    }
}

void merge_confusion_matrix(map< pair<label, label>, float >& base_matrix,
                            map< pair<label, label>, int >& new_matrix,
                            int test_case)
{
    map< pair<label, label>, int >::iterator it, end=new_matrix.end();
    map< pair<label, label>, float >::iterator it_base,
                                               end_base=base_matrix.end();
    
    int total = 0;
    for ( it = new_matrix.begin(); it != end; ++it ){
        total += it->second;
    }
    
    if ( total == 0 ){
        total = 1;
    }
    
    for ( it_base = base_matrix.begin(); it_base != end_base; ++it_base )
    {
        base_matrix[it_base->first] *= test_case;
    }
    
    for ( it = new_matrix.begin() ; it != end; ++it ){
        if ( base_matrix.find(it->first) == base_matrix.end() ){
            base_matrix[it->first] = 0.0;
        }
        
        base_matrix[it->first] += (float)it->second / (float)total;
    }
    
    for ( it_base = base_matrix.begin(); it_base != end_base; ++it_base )
    {
        base_matrix[it_base->first] /= test_case + 1;
    }
}

void print_confusion_matrix(map< pair<label, label>, float>& matrix)
{
    int num_steps = 2;
    
    for ( int i=0; i<num_steps; i++ ){
        printf("   %d: ", i);
        for ( int j=0; j<num_steps; j++ ){
            printf("%0.4f ", matrix[make_pair(i, j)]);
        }
        printf("\n");
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

    ifstream fin(argv[0]);
    string line;
    int test_case = 0;
    
    vector<threshold_cl*> thrs;
    vector<classifier_dpd*> cls;
    vector<int> neighborhood_size;
    
    for (int k = 0; k < 10; k++){
        // Neighborhood size
        neighborhood_size.push_back(k);
        
        // Thresholding algorithm
        motion_fe* f_cl = new motion_fe(k, 1, 1);
        threshold_cl* next_thrs = new threshold_cl();
        next_thrs->set_feature_extractor(f_cl);
        next_thrs->set_threshold(1000000);
        thrs.push_back(next_thrs);
        
        // Classifier
        classifier_dpd* next_cl = new classifier_dpd();
        next_cl->set_classifier(thrs[thrs.size() - 1]);
        cls.push_back(next_cl);
    }

    while ( getline(fin, line) )
    {
        cout << "Test " << test_case << ": " << line << endl;

        vector<Mat> images;
        vector<diagnosis_phase_detector::phase> labels;

        // Load sequence
        get_sequence(line.c_str(), images, labels, mod_rate);

        // Read labels
        vector<Mat>::iterator it, end;
        vector<label> cl_labels;
        for (size_t i = 0; i < labels.size(); i++ ){
            if ( labels[i] == diagnosis_phase_detector::diagnosis_transition ){
                cl_labels.push_back(1);
            } else {
                cl_labels.push_back(0);
            }
        }

        for (size_t t = 0; t < thrs.size(); t++){
            thrs[t]->log_values(images, cl_labels);

            /*
            Mat histogram;
            thrs[t]->plot_histogram(histogram, 50);
            imshow("histogram", histogram);
            */

            stringstream ss;
            ss << "results/phase_timeline/motion/"
               << neighborhood_size[t] << "_" << test_case << ".jpg";
            string filename;
            ss >> filename;

            cls[t]->visualize(images, labels, filename.c_str());
            
            map< pair<label, label>, int> next_matrix;
            map< pair<int, int>, float> normalized_matrix;
            
            thrs[t]->get_confusion_matrix(images, cl_labels, next_matrix);
            normalize_confusion_matrix(next_matrix, normalized_matrix);
            
            float acc = thrs[t]->eval(images, cl_labels);

            cout << "Accuracy (" << neighborhood_size[t] << "): "
                 << acc << endl;

            print_confusion_matrix(normalized_matrix);
        }

        cout << "=========================" << endl;
        test_case++;
        /*
        {
            vector<label> predictions;
            thrs.detect(images, predictions);
            
            for (size_t i = 0; i < images.size(); i++ ){
                vector<float> features;
                f.extract(images, i, features);
                
                vector<float> feature_total;
                f_cl.extract(images, i, feature_total);
                cout << feature_total[0] << endl;
                
                Mat dst;
                show_motion(features, nr, nc, dst, images[i].rows,
                            images[i].cols);
                
                vector<Mat> channels;
                Mat z = Mat::zeros(dst.rows, dst.cols, CV_8U);
                
                if ( cl_labels[i] == predictions[i] ){
                    channels.push_back(z);
                    channels.push_back(dst);
                    channels.push_back(z);
                } else {
                    channels.push_back(z);
                    channels.push_back(z);
                    channels.push_back(dst);
                }
                
                merge(channels, dst);
                cout << cl_labels[i] << " " << predictions[i] << endl;
                
                imshow("img", images[i]);
                imshow("motion", dst);
                waitKey(0);
            }
        }
        */
    }
    
    waitKey(0);
}
