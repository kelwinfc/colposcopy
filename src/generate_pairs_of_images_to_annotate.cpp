#include "generate_pairs_of_images_to_annotate.hpp"

class layer_generator {
    
    public:
        queue< pair<int, int> > boundaries;
        vector<int> last_layer;
        int last_level;
        
        layer_generator()
        {
            this->last_level = -1;
        }
        
        layer_generator(int a, int b)
        {
            this->last_level = -1;
            if ( a < b ){
                this->boundaries.push( make_pair(a, b) );
            }
        }
        
        void get_next(vector<int>& next_layer, int level)
        {
            if ( level == this->last_level ){
                next_layer = this->last_layer;
                return;
            }
            
            this->last_layer.clear();
            next_layer.clear();
            
            int n = this->boundaries.size();;
            
            while ( n-- ){
                // Retrieve the next region to process
                pair<int, int> next = this->boundaries.front();
                int next_value = (next.first + next.second) / 2;
                this->boundaries.pop();
                
                // Include the central element of the next region
                next_layer.push_back(next_value);
                
                // Split the region and include its sub-regions in the backlog
                if ( next.first < next_value ){
                    this->boundaries.push(make_pair(next.first,
                                                    next_value));
                }
                if ( next_value + 1 < next.second ){
                    this->boundaries.push(make_pair(next_value + 1,
                                                    next.second));
                }
            }
            
            this->last_layer = next_layer;
            this->last_level = level;
        }
};

int rows = 64;
int cols = 64;
int num_frames = 0;
int better = -1;

/* Each position in the vector is a different video.
 * Each video is represented by:
 *   - The filename
 *   - The sequence of frames of each colposcopy stage
 *   - The generator of frames
 *   - The annotated instance
 */
vector<string> filenames;
vector<int>    v_index;
vector<map<diagnosis_phase_detector::phase, vector<int> > > frames;
vector<map<diagnosis_phase_detector::phase, layer_generator> > generators;
vector<anonadado::instance* > instances;

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
    
    frames.clear();
    
    for ( int f = 0; f < num_frames; f++ ){
        
        anonadado::annotation* a = inst.get_active_annotation(step_index[0],
                                                              f);
        
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

void get_generator(map<diagnosis_phase_detector::phase, vector<int> >& frames,
                   map<diagnosis_phase_detector::phase, layer_generator >& g)
{
    map<diagnosis_phase_detector::phase, vector<int> >::iterator it, end;
    it = frames.begin();
    end = frames.end();
    
    for ( ; it != end; ++it ){
        layer_generator lg(0, it->second.size());
        g[it->first] = lg;
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
    
    if ( better != -1 ){
        for ( int r = 0; r < 20; r++ ){
            for ( int c = 0; c < 4 * cols; c++ ){
                dst.at<Vec3b>(ra.rows + r, 4 * cols * (1 - better) + c) =
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
    
    db_ranking db;
    
    int change_rate = 10;
    if ( argc > 1 ){
        sscanf(argv[1], "%d", &change_rate);
    }
    
    diagnosis_phase_detector::phase phases[4] = {
        diagnosis_phase_detector::diagnosis_plain,
        diagnosis_phase_detector::diagnosis_green,
        diagnosis_phase_detector::diagnosis_hinselmann,
        diagnosis_phase_detector::diagnosis_schiller
    };
    
    map<diagnosis_phase_detector::phase, string> names;
    names[diagnosis_phase_detector::diagnosis_plain] = "plain";
    names[diagnosis_phase_detector::diagnosis_green] = "green";
    names[diagnosis_phase_detector::diagnosis_hinselmann] = "hinselmann";
    names[diagnosis_phase_detector::diagnosis_schiller] = "schiller";
    
    int current_phase_index = 0;
    diagnosis_phase_detector::phase cphase = phases[current_phase_index];
    
    // Parse the videos specified in the input file
    ifstream fin(argv[0]);
    string next_seq;
    //int counter = 5;
    int vindex = 0;
    
    while ( getline(fin, next_seq) != 0 /*&& counter-- > 0*/ ){
        cout << next_seq << endl;
        // Filaname
        filenames.push_back(next_seq);
        
        // Index
        v_index.push_back(db.get_video_index(next_seq));
        
        // Sequence of frames
        map<diagnosis_phase_detector::phase, vector<int> > next_frames;
        get_sequence(next_seq.c_str(), next_frames); 
        frames.push_back(next_frames);
        
        // Frame generator
        map<diagnosis_phase_detector::phase, layer_generator> next_generator;
        get_generator(next_frames, next_generator);
        generators.push_back(next_generator);
        
        // Annotated instance
        anonadado::instance* next_instance = new anonadado::instance();
        next_instance->read(next_seq.c_str());
        instances.push_back(next_instance);
        cout << "Video " << vindex++ << " done." << endl;
    }
    
    fin.close();
    
    bool has = true;
    bool exit = false;
    bool go_to_next_phase = false;
    
    while ( !exit && has ){
        int remaining = change_rate;
        go_to_next_phase = false;
        cout << "\n\n\n NEEEEXT!\n\n\n";
        cout << current_phase_index << " " << cphase << endl;
        cout << endl << endl;
        
        for ( int level = 0; has && !exit && !go_to_next_phase; level++ )
        {
            cout << "LEVEL " << level << endl;
            //boost::this_thread::sleep( boost::posix_time::seconds(1) );
            
            vector< pair< pair<int, int>,
                          pair<int, int>
                        > > pairs;
            
            // Generate pairs <(video, frame), (video, frame)> for this level
            for ( size_t va = 0; va < instances.size(); va++ ){
                vector<int> framesl_a;
                generators[va][cphase].get_next(framesl_a, level);
                
                for ( size_t fa = 0; fa < framesl_a.size(); fa++){
                    for ( size_t vb = 0; vb < va; vb++ ){
                        vector<int> framesl_b;
                        generators[vb][cphase].get_next(framesl_b, level);
                        
                        for ( size_t fb = 0; fb < framesl_b.size(); fb++ ){
                            if ( va == vb && framesl_a[fa] == framesl_b[fb] ){
                                continue;
                            }

                            pairs.push_back(
                                make_pair(make_pair(va, framesl_a[fa]),
                                          make_pair(vb, framesl_b[fb])
                                         )
                            );
                        }
                    }
                }
            }
            
            if ( pairs.size() == 0 ){
                has = false;
                break;
            } else {
                has = true;
            }
            
            // Randomly sort these pairs
            random_shuffle(pairs.begin(), pairs.end());
            
            // Eval these pairs
            for ( size_t i = 0; i < pairs.size() && !go_to_next_phase; i++ ){
                
                int va = pairs[i].first.first;
                int fa = pairs[i].first.second;
                
                int vb = pairs[i].second.first;
                int fb = pairs[i].second.second;
                
                cout << "(" << va << ":" << fa << ") "
                     << "(" << vb << ":" << fb << ") " << endl;
                
                if ( db.exists(cphase,
                               v_index[va], frames[va][cphase][fa],
                               v_index[vb], frames[vb][cphase][fb])
                   )
                {
                    continue;
                }
                
                better = -1;
                
                while ( true ){
                    Mat a, b, dst;
                    instances[va]->get_frame(frames[va][cphase][fa], a);
                    instances[vb]->get_frame(frames[vb][cphase][fb], b);
                    draw_ui(a, b, dst);
                    
                    imshow(names[cphase], dst);
                    
                    int key = waitKey(0) % 0x100;
                    if ( key == 81 ){
                        better = 1;
                    } else if ( key == 83 ){
                        better = 0;
                    } else if ( key == 32 ){
                        break;
                    } else if ( key == 27 ){
                        exit = true;
                        has = false;
                        break;
                    } else {
                        better = -1;
                    }
                }
                
                if ( exit ){
                    break;
                }
                
                // Save the annotation
                db.insert_annotation(cphase,
                                     v_index[va], frames[va][cphase][fa],
                                     v_index[vb], frames[vb][cphase][fb],
                                     better);
                
                cout << "remaining " << remaining << endl;
                
                remaining--;
                if ( remaining <= 0 ){
                    go_to_next_phase = true;
                    break;
                }
            }
        }
        cout << "go to next\n";
        cvDestroyWindow(names[cphase].c_str());
        current_phase_index = (current_phase_index + 1) % 4;
        cphase = phases[current_phase_index];
        
    }
    
    cout << "Bye!\n";
    return 0;
}
