#ifndef __COLPOSCOPY_RANKING_DB
#define __COLPOSCOPY_RANKING_DB

#include <fstream>
#include <algorithm>
#include <vector>
#include <queue>
#include <set>
#include <sstream>

#include <boost/thread/thread.hpp>
#include "mongo/client/dbclient.h"

#include "anonadado.hpp"
#include "anonadado_utils.hpp"

#include "diagnosis_phase.hpp"

using namespace colposcopy;
using namespace std;

class db_frame {
    public:
        int video_index;
        string video_filename;
        int frame;
        Mat img;
        
        db_frame(){}
};

class db_ranking {
    private:
        mongo::DBClientConnection db;
    
    public:
        db_ranking();
        bool exists(diagnosis_phase_detector::phase phase,
                    int va, int fa, int vb, int fb);
        void insert_annotation(diagnosis_phase_detector::phase phase,
                               int va, int fa, int vb, int fb,
                               int label);
        int get_video_index(string filename);
        
        void get_videos(map<int, string>& videos);
        void get_annotated_frames(vector<db_frame>& frames,
                                  map<int, string>& videos,
                                  map<string, int>& index);
        void get_feedback(map<string, int>& index,
                          vector< pair<int, int> >& feedback);
    protected:
        string get_key(diagnosis_phase_detector::phase phase,
                       int va, int fa, int vb, int fb);
        
        void insert_annotation_aux(diagnosis_phase_detector::phase phase,
                                   int va, int fa, int vb, int fb,
                                   int label);
        
        string get_frame_key(int video, int frame);
};

#endif