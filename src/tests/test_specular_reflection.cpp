#include "test_specular_reflection.hpp"

int main(int argc, const char* argv[])
{
    argc--;
    argv++;
    if ( argc == 0 ){
        return -1;
    }

    Mat src = imread(argv[0]);
    
    if ( src.data == 0 ){
        return -1;
    }

    {
        Mat aux;
        resize(src, aux, Size(200, 200));
        aux.copyTo(src);
    }
    
    Mat mask, dst_avg, dst_d2;
    
    detect_specular_reflection_das(src, mask, 150);
    fill_with_avg(src, mask, dst_avg);
    //fill_with_d2(src, mask, dst_d2);
    
    string s, m, davg, dd2;
    s = argv[1];
    s += "_src.jpg";
    m = argv[1];
    m += "_mask.jpg";
    davg = argv[1];
    davg += "_dst_avg.jpg";
    //dd2 = argv[1];
    //dd2 += "_dst_d2.jpg";
    
    imwrite(s.c_str(), src);
    imwrite(m.c_str(), mask);
    imwrite(davg.c_str(), dst_avg);
    //imwrite(dd2.c_str(), dst_d2);
    
    cout << "Bye!" << endl;
    return 0;
}