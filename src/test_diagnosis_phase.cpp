#include "test_diagnosis_phase.hpp"
#include "/home/kelwinfc/Desktop/anonadado/src/api/cpp/anonadado.hpp"
#include "/home/kelwinfc/Desktop/anonadado/src/api/cpp/anonadado.cpp"
#include "/home/kelwinfc/Desktop/anonadado/src/api/cpp/utils.hpp"
#include "/home/kelwinfc/Desktop/anonadado/src/api/cpp/utils.cpp"

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

    vector<Mat> images, test_images;
    vector<diagnosis_phase_detector::phase> labels, test_labels;
    
    get_sequence(argv[0], images, labels, 1);
    
    histogram_based_dpd hd;
    hd.train(images, labels);
    hd.write("dp.json");

    w_dpd whd(&hd, 10);
    context_dpd cwhd(&whd);
    
    hd.visualize(images, labels, "results/phase_timeline/0_histogram.jpg");
    whd.visualize(images, labels, "results/phase_timeline/1_w.jpg");
    cwhd.visualize(images, labels, "results/phase_timeline/2_context.jpg");
    
    get_sequence(argv[1], test_images, test_labels, 1);
    
    
    float error = cwhd.print_confussion_matrix(test_images, test_labels);
    cout << "Test error: " << error << endl;
    
    hd.visualize(test_images, test_labels,
                 "results/phase_timeline/test_0_histogram.jpg");

    whd.visualize(test_images, test_labels,
                  "results/phase_timeline/test_1_w.jpg");
    cwhd.visualize(test_images, test_labels,
                   "results/phase_timeline/test_2_context.jpg");

}
