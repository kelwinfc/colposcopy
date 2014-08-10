#include "generate_pairs_of_images_to_annotate.hpp"

int rows = 64;
int cols = 64;
int num_frames = 0;
int best = 1;

class pair_generator {
    
    public:
        vector< pair<int, int> > boundaries;
        vector< pair<int, int> > next_boundaries;
        
        pair_generator()
        {
            this->boundaries.push_back( make_pair(0, 0) );
        }
        
        pair_generator(int a, int b)
        {
            this->boundaries.push_back( make_pair(a, b) );
        }
        
        bool has_available(int i)
        {
            return this->boundaries[i].first < this->boundaries[i].second;
        }
        
        pair<int, int> get_next()
        {
            // Split
            if ( this->next_boundaries.size() == 0 ){
                vector< pair<int, int> > new_b;
                for ( size_t i = 0; i < this->boundaries.size(); i++ ){
                    if ( this->boundaries[i].second - 
                         this->boundaries[i].first > 1 )
                    {
                        int mid = (this->boundaries[i].first +
                                   this->boundaries[i].second) / 2;
                        
                        new_b.push_back(make_pair(this->boundaries[i].first,
                                                  mid)
                                       );
                        new_b.push_back(make_pair(mid,
                                                  this->boundaries[i].second)
                                       );
                    }
                }
                
                this->boundaries = new_b;
                
                for ( size_t i = 0; i < this->boundaries.size(); i++ ){
                    for ( size_t j = 0; j < i; j++ ){
                        this->next_boundaries.push_back( make_pair(i, j) );
                    }
                }
                
                random_shuffle(this->next_boundaries.begin(),
                               this->next_boundaries.end());
                
                return this->get_next();
            }
            
            int a = this->next_boundaries.back().first;
            int b = this->next_boundaries.back().second;
            
            int va = this->boundaries[a].first + this->boundaries[a].second;
            int vb = this->boundaries[b].first + this->boundaries[b].second;
            
            this->next_boundaries.pop_back();
            
            return make_pair(va / 2, vb / 2);
        }
        
        bool has_next()
        {
            if ( this->next_boundaries.size() > 0 ){
                return true;
            }
            
            for ( size_t i = 0; i < boundaries.size(); i++ ){
                if ( has_available(i) )
                {
                    return true;
                }
            }
            return false;
        }
        
        size_t num_intervals()
        {
            return this->boundaries.size();
        }
};

void get_sequence(const char* filename,
                  map<diagnosis_phase_detector::phase, vector<int> >& frames)
{
    anonadado::instance inst;
    inst.read(filename);
    vector<int> step_index;
    vector<int> roi_index;
    
    inst.get_annotations("diagnosis_step", step_index);
    inst.get_annotations("roi", roi_index);
    
    num_frames = inst.num_frames();
    cout << num_frames << " frames.\n";
    
    frames.clear();
    
    for ( int f = 0; f < num_frames; f++ ){
        
        annotation* a = inst.get_active_annotation(step_index[0], f);
        
        if ( !a )
            continue;
        
        anonadado::choice_feature* step_feature =
                            (anonadado::choice_feature*)a->get_feature("step");
        
        if ( diagnosis_phase_detector::string_to_phase(
                                                step_feature->get_value()) ==
             diagnosis_phase_detector::diagnosis_transition
           )
        {
            continue;
        }
        
        Mat img, aux;
        inst.get_frame(f, img);
        
        if ( !img.data ){
            continue;
        }
        
        diagnosis_phase_detector::phase step = 
           diagnosis_phase_detector::string_to_phase(step_feature->get_value());
        
        if ( frames.find(step) == frames.end() ){
            vector<int> next;
            frames[step] = next;
        }
        
        frames[step].push_back(f);
    }
}

void draw_ui(Mat& a, Mat& b, Mat& dst)
{
    Mat ra, rb;
    resize(a, ra, Size(4 * rows, 4 * cols));
    resize(b, rb, Size(4 * rows, 4 * cols));
    
    dst = Mat::zeros(ra.rows + 20, ra.cols * 2, CV_8UC3);
    
    Mat dst_roi_a = dst(Rect(0, 0, ra.cols, ra.rows));
    ra.copyTo(dst_roi_a);
    
    Mat dst_roi_b = dst(Rect(ra.cols, 0, ra.cols, ra.rows));
    rb.copyTo(dst_roi_b);
    
    if ( best != 1 ){
        for ( int r = 0; r < 20; r++ ){
            for ( int c = 0; c < 4 * cols; c++ ){
                dst.at<Vec3b>(ra.rows + r, 4 * cols * ( best / 2) + c) =
                    Vec3b(255, 0, 0);
            }
        }
    }
}

int main(int argc, const char* argv[])
{
    argc--;
    argv++;
    srand (time(NULL));
    
    map<diagnosis_phase_detector::phase, vector<int> > frames;
    map<diagnosis_phase_detector::phase, pair_generator> generators;
    
    get_sequence(argv[0], frames);
    size_t change_rate = 10;
    
    {
        map<diagnosis_phase_detector::phase, vector<int> >::iterator it, end;
        it = frames.begin();
        end = frames.end();
        
        for ( ; it != end; ++it ){
            pair_generator pg(0, it->second.size());
            generators[it->first] = pg;
        }
    }
    
    bool has = true;
    
    anonadado::instance inst;
    inst.read(argv[0]);
    
    map<diagnosis_phase_detector::phase, string> names;
    names[diagnosis_phase_detector::diagnosis_plain] = "plain";
    names[diagnosis_phase_detector::diagnosis_green] = "green";
    names[diagnosis_phase_detector::diagnosis_hinselmann] = "hinselmann";
    names[diagnosis_phase_detector::diagnosis_schiller] = "schiller";
    
    while ( has ){
        
        has = false;
        
        /* Label instances equally distributed from every phase */
        map<diagnosis_phase_detector::phase, pair_generator>::iterator it, end;
        it = generators.begin();
        end = generators.end();
        
        for ( ; it != end; ++it ){
            int num_pairs = change_rate;
            
            if ( frames[it->first].size() < 2 )
            {
                continue;
            }
            
            Mat img = Mat::zeros(200, frames[it->first].size(), CV_8UC3);
            
            while ( num_pairs-- > 0 && it->second.has_next() )
            {
                pair<int, int> next = it->second.get_next();
                
                for ( int i=0; i<200; i++){
                    img.at<Vec3b>(i, next.first) = Vec3b(0, 0, 255);
                    img.at<Vec3b>(i, next.second) = Vec3b(0, 0, 255);
                }
                
                has = true;
                
                while ( true ){
                    Mat a, b, dst;
                    inst.get_frame(frames[it->first][next.first], a);
                    inst.get_frame(frames[it->first][next.second], b);
                    draw_ui(a, b, dst);
                    
                    imshow(names[it->first], dst);
                    int key = waitKey(0) % 0x100;
                    if ( key == 81 ){
                        best = 0;
                    } else if ( key == 83 ){
                        best = 2;
                    } else if ( key == 32 ){
                        break;
                    } else {
                        best = 1;
                    }
                }
                
                best = 1;
            }
            
            destroyWindow(names[it->first]);
        }
    }
    
    return 0;
}
