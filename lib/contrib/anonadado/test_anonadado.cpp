#include "test_anonadado.hpp"

int main(int argc, char* argv[])
{
    anonadado::instance inst;
    inst.read(argv[1]);
    int frame = 0;
    int annotation_index = 0;

    if ( argc > 2 ){
        sscanf(argv[2], "%d", &frame);
    }
    
    vector<int> annotations;
    inst.get_active_annotations(0, annotations);
    cout << annotations.size() << " active annotations\n";
    
    anonadado::annotation* a =
            inst.get_active_annotation(annotations[annotation_index], frame);

    vector<string> features;
    a->get_features(features);
    
    cout << a->get_name() << ":" << endl;
    
    for ( uint i=0; i<features.size(); i++ ){
        cout << "   " << features[i] << ":";
        
        anonadado::choice_feature* f = (anonadado::choice_feature*)a->get_feature(features[i]);
        cout << f->get_value() << endl;
    }

    Mat img;
    inst.get_frame(frame, img);
    
    cout << "Bye!\n";
    
    return 0;
}
