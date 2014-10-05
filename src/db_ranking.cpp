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
