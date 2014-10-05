#ifndef __COLPOSCOPY_RANKING_DB
#define __COLPOSCOPY_RANKING_DB

#include <fstream>
#include <algorithm>
#include <vector>
#include <queue>
#include <set>

#include <boost/thread/thread.hpp>
#include "mongo/client/dbclient.h"

#include "diagnosis_phase.hpp"

using namespace colposcopy;
using namespace std;

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

    protected:
        string get_key(diagnosis_phase_detector::phase phase,
                       int va, int fa, int vb, int fb);
        
        void insert_annotation_aux(diagnosis_phase_detector::phase phase,
                                   int va, int fa, int vb, int fb,
                                   int label);
};

#endif