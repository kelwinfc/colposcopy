#include "utils.hpp"

int num_chars(int n)
{
    std::stringstream ss;
    std::string s;
    
    ss << n;
    ss >> s;
    
    return s.size();
}

std::string spaced_d(int d, int n)
{
    std::stringstream ss;
    std::string ret, shift="";
    
    ss << d;
    ss >> ret;

    n -= ret.size();
    while ( n-- > 0 ){
        shift += " ";
    }

    return shift + ret;
}

/* Returns the amount of milliseconds elapsed since the UNIX epoch. Works on both
 * windows and linux. */

int64 GetTimeMs64()
{
    #ifdef WIN32
        /* Windows */
        FILETIME ft;
        LARGE_INTEGER li;

        /* Get the amount of 100 nano seconds intervals elapsed since January 1,
         * 1601 (UTC) and copy it
         * to a LARGE_INTEGER structure.
         */
        GetSystemTimeAsFileTime(&ft);
        li.LowPart = ft.dwLowDateTime;
        li.HighPart = ft.dwHighDateTime;

        uint64 ret = li.QuadPart;
        ret -= 116444736000000000LL; /* Convert from file time to UNIX
                                      * epoch time.
                                      */
        ret /= 10000; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3)
                       * intervals
                       */

        return ret;
    #else
        /* Linux */
        struct timeval tv;

        gettimeofday(&tv, NULL);

        uint64 ret = tv.tv_usec;
        /* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
        ret /= 1000;

        /* Adds the seconds (10^0) after converting them to milliseconds
         * (10^-3) */
        ret += (tv.tv_sec * 1000);

        return ret;
    #endif
}

void all_but_k(std::vector<std::string>& src,
               std::vector<std::string>& dst, int k)
{
    dst.clear();
    std::vector<std::string>::iterator it=src.begin(), end=src.end();
    size_t i = 0;
    
    for ( ; it != end; ++it ){
        if ( i++ != k ){
            dst.push_back(*it);
        }
    }
}

void plot_histogram(std::vector<float>& h, Mat& dst)
{
    int hist_w = 512; int hist_h = 400;
    int histSize = h.size();
    
    int bin_w = cvRound( (double) hist_w / histSize );

    dst = Mat(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
    
    /// Draw for each channel
    for( int i = 1; i < histSize; i++ )
    {
      line(dst, Point(bin_w*(i-1), hist_h - cvRound(h[i-1] * dst.rows)),
                Point(bin_w*(i), hist_h - cvRound(h[i] * dst.rows)),
                Scalar(255, 0, 0), 2, 8, 0);
    }
}
