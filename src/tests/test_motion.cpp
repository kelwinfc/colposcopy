#include "test_motion.hpp"

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

int main(int argc, const char* argv[])
{
    cout << "Hi!\n";
    
    argc--;
    argv++;
    if ( argc == 0 ){
        return -1;
    }

    int mod_rate = 2;
    
    vector<Mat> images;
    vector<diagnosis_phase_detector::phase> labels;
        
    get_sequence(argv[0], images, labels, mod_rate);
    
    int nr=8, nc=8;
    motion_fe f(5, nr, nc);
    
    vector<Mat>::iterator it, end;
    
    for (size_t i = 0; i < images.size(); i++ ){
        vector<float> features;
        f.extract(images, i, features);
        Mat dst;
        show_motion(features, nr, nc, dst, images[i].rows, images[i].cols);
        
        imshow("img", images[i]);
        imshow("motion", dst);
        waitKey(0);
    }
}
