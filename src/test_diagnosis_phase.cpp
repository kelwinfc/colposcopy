#include "test_diagnosis_phase.hpp"
#include "/home/kelwinfc/Desktop/anonadado/src/api/cpp/anonadado.hpp"
#include "/home/kelwinfc/Desktop/anonadado/src/api/cpp/anonadado.cpp"
#include "/home/kelwinfc/Desktop/anonadado/src/api/cpp/utils.hpp"
#include "/home/kelwinfc/Desktop/anonadado/src/api/cpp/utils.cpp"

int rows = 64;
int cols = 64;

void get_sequence(const char* filename,
                  vector<Mat>& images,
                  vector<diagnosis_phase_detector::phase>& labels
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
        if ( f % 5 != 0 ){
            continue;
        }
        
        annotation* a = inst.get_active_annotation(step_index[0], f);
        annotation* roi = inst.get_active_annotation(roi_index[0], f);
        
        if ( !a )
            continue;

        anonadado::choice_feature* step_feature =
                            (anonadado::choice_feature*)a->get_feature("step");
        labels.push_back(diagnosis_phase_detector::string_to_phase(
                                                    step_feature->get_value()));
        

        anonadado::bbox_feature* roi_feature =
                            (anonadado::bbox_feature*)roi->get_feature("roi");
        BBOX roi_value = roi_feature->get_value();

        Mat img, aux;
        inst.get_frame(f, img);
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

int main(int argc, const char* argv[])
{
    argc--;
    argv++;
    if ( argc == 0 ){
        return -1;
    }

    vector<Mat> images;
    vector<diagnosis_phase_detector::phase> labels;
    
    get_sequence(argv[0], images, labels);

    histogram_based_dp_detector hd;
    hd.train(images, labels);
    cout << labels.size() << endl;
}
