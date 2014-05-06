#ifndef __COLPOSCOPY_SPECULAR_REFLECTION
#define __COLPOSCOPY_SPECULAR_REFLECTION

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/video.hpp"
#include "ml.h"

#include <vector>
#include <queue>

using namespace std;
using namespace cv;

void detect_specular_reflection_das(Mat& src, Mat& dst, uchar thr=230);
void fill_with_avg(Mat& src, Mat& mask, Mat& dst);
void fill_with_d2(Mat& src, Mat& mask, Mat& dst);

#endif