#ifndef __COLPOSCOPY_UTILS
#define __COLPOSCOPY_UTILS

#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/video.hpp"
#include "ml.h"

#ifdef WIN32
    #include <Windows.h>
#else
    #include <sys/time.h>
    #include <ctime>
#endif

#define __COLPOSCOPY_VERBOSE 0

using namespace cv;

typedef long long int64;
typedef unsigned long long uint64;

int num_chars(int n);

std::string spaced_d(int d, int n);

int64 GetTimeMs64();

void all_but_k(std::vector<std::string>& src,
               std::vector<std::string>& dst, int k);

void plot_histogram(std::vector<float>& h, Mat& dst);

std::pair<int, int> get_center(Mat& src);

float pair_distance(std::pair<int, int>& a, std::pair<int, int>& b);

#endif
