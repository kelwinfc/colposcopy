#include "db_ranking.hpp"

db_ranking::db_ranking()
{
    string errmsg;
    
    try {
        db.connect("localhost", errmsg);
    } catch( const mongo::DBException &e ) {
        cout << "Error: " << errmsg << endl;
        exit(-1);
    }
}

string db_ranking::get_key(diagnosis_phase_detector::phase phase,
                           int va, int fa, int vb, int fb)
{
    stringstream ss;
    string ret;
    ss << phase << "/" << va << "/" << fa << "/" << vb << "/" << fb;
    getline(ss, ret);
    return ret;
}

bool db_ranking::exists(diagnosis_phase_detector::phase phase,
                        int va, int fa, int vb, int fb)
{
    string key = get_key(phase, va, fa, vb, fb);
    auto_ptr<mongo::DBClientCursor> cursor = 
                this->db.query("colposcopy.ranking", QUERY( "_id" << key ));
    
    while ( cursor->more() ){
        mongo::BSONObj p = cursor->next();
        cout << "Exists! " << key << endl;
        return true;
    }
    
    return false;
}

void db_ranking::insert_annotation(diagnosis_phase_detector::phase phase,
                                   int va, int fa, int vb, int fb,
                                   int label)
{
    this->insert_annotation_aux(phase, va, fa, vb, fb, label);
    
    if ( label != -1 ){
        this->insert_annotation_aux(phase, vb, fb, va, fa, 1 - label);
    }
}

void db_ranking::insert_annotation_aux(diagnosis_phase_detector::phase phase,
                                       int va, int fa, int vb, int fb,
                                       int label)
{
    mongo::BSONObjBuilder b;
    b.append("_id", get_key(phase, va, fa, vb, fb));
    b.append("best_video", va);
    b.append("worst_video", vb);
    b.append("best_frame", fa);
    b.append("worst_frame", fb);
    b.append("rank", label);
    
    mongo::BSONObj r = b.obj();
    this->db.insert("colposcopy.ranking", r);
}

int db_ranking::get_video_index(string filename)
{
    // Verify if exists
    {
        string key = filename;
        auto_ptr<mongo::DBClientCursor> cursor = 
                    this->db.query("colposcopy.video", QUERY( "_id" << key ));
        
        if ( cursor->more() ){
            mongo::BSONObj p = cursor->next();
            return p.getIntField("index");
        }
    }
    
    // Find the max index
    int max_value = -1;
    
    {
        auto_ptr<mongo::DBClientCursor> cursor = 
                    this->db.query("colposcopy.video",
                                   mongo::Query().sort("index", -1));
        
        while ( cursor->more() ){
            mongo::BSONObj p = cursor->next();
            max_value = max(max_value, p.getIntField("index"));
        }
    }
    
    // Insert the new video
    mongo::BSONObjBuilder b;
    b.append("_id", filename);
    b.append("index", ++max_value);
    mongo::BSONObj r = b.obj();
    this->db.insert("colposcopy.video", r);
    
    return max_value;
}

void db_ranking::get_videos(map<int, string>& videos)
{
    mongo::auto_ptr<mongo::DBClientCursor> cursor = 
        this->db.query("colposcopy.video", mongo::BSONObj());

    videos.clear();
    while (cursor->more()){
        mongo::BSONObj p = cursor->next();
        videos[p.getIntField("index")] = p.getStringField("_id");
    }
}

string db_ranking::get_frame_key(int video, int frame)
{
    stringstream ss;
    string ret;
    
    ss << video << "_" << frame;
    ss >> ret;
    
    return ret;
}

void db_ranking::get_annotated_frames(vector<db_frame>& frames,
                                      map<int, string>& videos,
                                      map<string, int>& index)
{
    mongo::auto_ptr<mongo::DBClientCursor> cursor = 
        this->db.query("colposcopy.ranking", mongo::BSONObj());

    frames.clear();
    index.clear();

    while (cursor->more()){
        mongo::BSONObj p = cursor->next();

        db_frame next_frame;
        next_frame.video_index = p.getIntField("best_video");
        next_frame.frame = p.getIntField("best_frame");
        
        string key = this->get_frame_key(next_frame.video_index,
                                         next_frame.frame);
        
        if ( index.find(key) == index.end() ){
            next_frame.video_filename = videos[next_frame.video_index];

            anonadado::instance inst;
            inst.read(next_frame.video_filename);
            inst.get_frame(next_frame.frame, next_frame.img);

            index[key] = frames.size();
            frames.push_back(next_frame);
        }
    }
}

void db_ranking::get_feedback(map<string, int>& index,
                              vector< pair<int, int> >& feedback)
{
    feedback.clear();
    
    mongo::auto_ptr<mongo::DBClientCursor> cursor = 
        this->db.query("colposcopy.ranking", mongo::BSONObj());

    while (cursor->more()){
        mongo::BSONObj p = cursor->next();

        int best_video = p.getIntField("best_video");
        int best_frame = p.getIntField("best_frame");
        string best_key = this->get_frame_key(best_video, best_frame);

        int worst_video = p.getIntField("worst_video");
        int worst_frame = p.getIntField("worst_frame");
        string worst_key = this->get_frame_key(worst_video, worst_frame);
        
        if ( p.getIntField("rank") == 1 ){
            feedback.push_back(make_pair(index[best_key], index[worst_key]));
        }
    }
}
