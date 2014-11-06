#ifndef __COLPOSCOPY_SPECULAR_REFLECTION
#define __COLPOSCOPY_SPECULAR_REFLECTION

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/photo/photo.hpp"
#include "ml.h"

#include <vector>
#include <queue>
#include <map>

#include "utils.hpp"

using namespace std;
using namespace cv;

class specular_reflection_detection {
    public:
        specular_reflection_detection();
        virtual void detect(Mat& img, Mat& dst);
};

class threshold_srd : public specular_reflection_detection {
    private:
        uchar thrs;
        uchar erosion_size;

    public:
        threshold_srd(uchar thr=255, uchar erosion_size=2);
        virtual void detect(Mat& img, Mat& dst);
        void set_threshold(uchar thr);
    
    protected:
        Mat dilate_mask(Mat& src);
};

class img_inpaint {
    private:
        
    public:
        img_inpaint();
        virtual void fill(Mat& img, Mat& holes, Mat& dst);
};

class best_first_ip : public img_inpaint {
    public:
        best_first_ip();
        virtual void fill(Mat& img, Mat& holes, Mat& dst);
    
    protected:
        int num_neighbors_with_information(Mat& mask, int x, int y);
        void fill_with_avg(Mat& src, Mat& mask, Mat& dst);
        void get_neighbors_avg(Mat& src, Mat& mask, int x, int y, Vec3b& ret);
};

class telea_ip : public img_inpaint {
    private:
        float r;

    public:
        telea_ip(float r);
        virtual void fill(Mat& img, Mat& holes, Mat& dst);
};

class navier_stokes_ip : public img_inpaint {
    private:
        float r;

    public:
        navier_stokes_ip(float r);
        virtual void fill(Mat& img, Mat& holes, Mat& dst);
};
 
#endif