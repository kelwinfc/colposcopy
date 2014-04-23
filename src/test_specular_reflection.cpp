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
    
    Mat mask, dst;
    
    detect_specular_reflection_das(src, mask, 150);
    fill_with_avg(src, mask, dst);

    string s, m, d;
    s = argv[1];
    s += "_src.jpg";
    m = argv[1];
    m += "_mask.jpg";
    d = argv[1];
    d += "_dst.jpg";
    
    imwrite(s.c_str(), src);
    imwrite(m.c_str(), mask);
    imwrite(d.c_str(), dst);
    
    cout << "Bye!" << endl;
    return 0;
}