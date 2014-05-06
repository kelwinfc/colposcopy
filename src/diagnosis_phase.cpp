#include "diagnosis_phase.hpp"

diagnosis_phase_detector::phase diagnosis_phase_detector::string_to_phase(
                                                                    string s)
{
    if ( s == "plain" ){
        return diagnosis_plain;
    } else if ( s == "green" ){
        return diagnosis_green;
    } else if ( s == "hinselmann" ){
        return diagnosis_hinselmann;
    } else if ( s == "schiller" ){
        return diagnosis_schiller;
    } else if ( s == "transition" ){
        return diagnosis_transition;
    } else {
        return diagnosis_unknown;
    }
}

diagnosis_phase_detector::diagnosis_phase_detector()
{
    
}

void diagnosis_phase_detector::read(string filename)
{
    
}

void diagnosis_phase_detector::train(vector<Mat>& src, vector<phase>& labels)
{
    
}

float diagnosis_phase_detector::eval(vector<Mat>& src, vector<phase>& labels)
{
    int n = labels.size();
    uint f = 0;

    if ( n == 0 ){
        return 0.0;
    }

    vector<phase> output;
    this->detect(src, output);

    for ( int i = 0; i < n; i++ ){
        if ( output[i] != labels[i] ){
            f++;
        }
    }
    
    return ((float)f) / labels.size();
}

void diagnosis_phase_detector::detect(vector<Mat>& src, vector<phase>& dst)
{
    vector<Mat>::iterator it, end;

    dst.clear();
    it = src.begin();
    end = src.end();
    
    for ( ; it != end; ++it ){
        dst.push_back(diagnosis_unknown);
    }
}

float diagnosis_phase_detector::get_confussion_matrix(vector<Mat>& src,
                 vector<phase>& labels, map< pair<phase, phase>, int>& matrix)
{
    phase steps[] = {
                     diagnosis_green,
                     diagnosis_hinselmann,
                     diagnosis_plain,
                     diagnosis_schiller,
                     diagnosis_transition,
                     diagnosis_unknown
                    };
    int num_steps = 6;

    matrix.clear();
    for ( int i=0; i<num_steps; i++ ){
        for ( int j=0; j<num_steps; j++ ){
            matrix[ make_pair(steps[i],steps[j]) ] = 0;
        }
    }

    vector<phase> output;
    this->detect(src, output);

    int f = 0;
    for ( uint i=0; i<src.size(); i++ ){
        pair<phase, phase> p = make_pair( labels[i], output[i] );
        matrix[p] = matrix[p] + 1;
        if ( labels[i] != output[i] ){
            f++;
        }
    }

    return ((float)f) / ((float)output.size());
}

histogram_based_dp_detector::histogram_based_dp_detector()
{
    this->max_error = 0.01;
    this->bindw = 1;
}

void histogram_based_dp_detector::read(string filename)
{
    
}

void histogram_based_dp_detector::detect(vector<Mat>& src, vector<phase>& dst)
{
    vector< vector<float> > hists;
    this->compute_histograms(src, hists);
    this->detect(hists, dst);
}

float histogram_based_dp_detector::eval(vector<Mat>& src, vector<phase>& labels)
{
    vector< vector<float> > hists;
    this->compute_histograms(src, hists);
    return this->eval(hists, labels);
}

void histogram_based_dp_detector::get_histogram(Mat& a, vector<float>& h)
{
    Mat aux;
    float max_v = 0;
    vector<Mat> hsv_planes;

    cvtColor(a, aux, CV_BGR2HSV);
    split( aux, hsv_planes );

    h.resize( 256 / this->bindw);
    fill(h.begin(), h.end(), 0.0);
    
    for ( int r=0; r<a.rows; r++ ){
        for ( int c=0; c<a.rows; c++ ){
            int b = hsv_planes[0].at<uchar>(r,c) / this->bindw;
            h[b] += 1.0;
            max_v = max(max_v, h[b]);
        }
    }

    for ( uint i = 0; i < h.size(); i++ ){
        h[i] /= max_v;
    }
}

float histogram_based_dp_detector::distance(Mat& a, Mat& b)
{
    vector<float> ha, hb;

    this->get_histogram(a, ha);
    this->get_histogram(b, hb);
    
    return this->distance(ha, hb);
}

float histogram_based_dp_detector::distance(vector<float>& ha,
                                            vector<float>& hb)
{
    float ret = 0.0;
    float total = 0.0;
    
    for ( uint i = 0; i < ha.size(); i++ ){
        ret += min(ha[i], hb[i]);
        total += max(ha[i], hb[i]);
    }

    return 1.0 - ret / total;
}

float histogram_based_dp_detector::compute_threshold(vector<Mat>& src,
                                                     vector<phase>& labels,
                                                     vector< vector<float> >& h,
                                                     int index)
{
    vector< pair<float, int> > q;
    uint n = labels.size();
    uint misclassified = 0;
    
    for ( uint i = 0; i < n; i++ ){
        if ( labels[i] != diagnosis_unknown ){
            misclassified++;
        }
    }
    
    for ( uint i = 0; i < n; i++ ){
        q.push_back( make_pair(this->distance(h[index], h[i]), i) );
    }
    sort(q.begin(), q.end());
    
    int best_threshold = 0;
    float min_error = (float)misclassified / (float)n;
    
    for ( uint i = 0; i < n; i++ ){
        if ( labels[q[i].second] != labels[index] ){
            misclassified++;
        } else {
            misclassified--;
        }

        float next_error = (float)misclassified / (float)n;
        if ( next_error < min_error ){
            best_threshold = i;
            min_error = next_error;
        }
    }
    
    return q[best_threshold].first;
}

void histogram_based_dp_detector::compute_thresholds(vector<Mat>& src,
                                                     vector<phase>& labels,
                                                     vector< vector<float> >& h,
                                                     vector<float>& threshold)
{
    uint n = labels.size();
    
    threshold.resize(n);
    for ( uint i = 0; i < n; i++ ){
        threshold[i] = this->compute_threshold(src, labels, h, i);
    }
}

void histogram_based_dp_detector::compute_reliability(vector<Mat>& src,
        vector<phase>& labels, vector< vector<float> >& h,
        vector<float>& threshold, vector<float>& reliability)
{
    int t=0,f=0;
    uint n = labels.size();

    reliability.resize(n);
    fill(reliability.begin(), reliability.end(), 0.0);
    
    for ( uint index=0; index<n; index++ ){
        for ( uint i=0; i<n; i++ ){
            phase predicted = diagnosis_unknown;
            if ( this->distance(h[index], h[i]) <= threshold[index] ){
                predicted = labels[index];
            }
            
            if ( predicted == labels[i] ){
                t++;
            } else {
                f++;
            }
        }

        reliability[index] = ((float)t) / ((float)(t + f));
    }
}

void histogram_based_dp_detector::add_to_index(vector<float>& hist,
                                               phase label,
                                               float threshold,
                                               float reliability)
{
    this->index_histogram.push_back(hist);
    this->index_phase.push_back(label);
    this->index_threshold.push_back(threshold);
    this->index_reliability.push_back(reliability);
}

void histogram_based_dp_detector::remove_last()
{
    this->index_histogram.pop_back();
    this->index_phase.pop_back();
    this->index_threshold.pop_back();
    this->index_reliability.pop_back();
}

float histogram_based_dp_detector::eval(vector< vector<float> >& src,
                                        vector<phase>& labels)
{
    int n = labels.size();
    uint f = 0;

    if ( n == 0 ){
        return 0.0;
    }

    vector<phase> output;
    this->detect(src, output);

    for ( int i = 0; i < n; i++ ){
        if ( output[i] != labels[i] ){
            f++;
        }
    }

    return ((float)f) / labels.size();
}

void histogram_based_dp_detector::detect(vector< vector<float> >& src,
                                         vector<phase>& dst)
{
    uint n = src.size();
    for ( uint i = 0; i < n; i++ ){
        
        map<phase, float> r;
        
        r[diagnosis_plain] = 0.0;
        r[diagnosis_green] = 0.0;
        r[diagnosis_hinselmann] = 0.0;
        r[diagnosis_schiller] = 0.0;
        r[diagnosis_transition] = 0.0;
        
        bool has = false;
        for ( uint k = 0; k < this->index_histogram.size(); k++ ){
            float next_distance = distance(this->index_histogram[k], src[i]);
            if ( next_distance <= this->index_threshold[k] ){
                r[this->index_phase[k]] += this->index_reliability[k];
                has = true;
            }
        }
        
        if ( has ){
            phase best_phase = diagnosis_unknown;
            float best_r = 0.0;
            map<phase, float>::iterator it;
            for ( it = r.begin(); it != r.end(); ++it ){
                if ( it->second > best_r ){
                    best_phase = it->first;
                    best_r = it->second;
                }
            }
            dst.push_back(best_phase);
        } else {
            dst.push_back(diagnosis_unknown);
        }
    }
}

pair<int, float> histogram_based_dp_detector::best_frame(vector<Mat>& src,
                        vector<phase>& labels, vector<bool>& indexed,
                        vector<vector<float> >& hists, vector<float>& threshold,
                        vector<float>& reliability)
{
    float min_error = 1.0;
    int ret = 0;
    uint n = labels.size();

    for ( uint i = 0; i < n; i++ ){

        if ( indexed[i] || labels[i] == diagnosis_unknown ){
            continue;
        }
        
        this->add_to_index(hists[i], labels[i], threshold[i], reliability[i]);
        float next_error = this->eval(hists, labels);
        
        if ( next_error <= min_error ){
            min_error = next_error;
            ret = i;
        }

        this->remove_last();
    }
    
    return make_pair(ret, min_error);
}

void histogram_based_dp_detector::compute_histograms(vector<Mat>& src,
                                                     vector<vector<float> >& h)
{
    uint n = src.size();
    for ( uint i = 0; i < n; i++ ){
        vector<float> next_h;
        this->get_histogram(src[i], next_h);
        h.push_back(next_h);
    }
}

void histogram_based_dp_detector::train(vector<Mat>& src, vector<phase>& labels)
{
    cout << "Training\n";
    
    vector<bool> indexed;
    vector<float> threshold, reliability;
    vector< vector<float> > hists;
    
    int n = labels.size();
    float current_error = 1.0;
    float previous_error = -1.0;
    
    indexed.resize(n);
    fill(indexed.begin(), indexed.end(), false);
    
    this->compute_histograms(src, hists);
    cout << "Histograms computed\n";
    this->compute_thresholds(src, labels, hists, threshold);
    cout << "Thresholds computed\n";
    this->compute_reliability(src, labels, hists, threshold, reliability);
    cout << "Reliability computed\n";
    
    while ( current_error > this->max_error &&
            this->index_histogram.size() < src.size() && 
            ( previous_error < 0.0 || previous_error > current_error )
          )
    {
        previous_error = current_error;
        
        pair<int, float> best = this->best_frame(src, labels, indexed, hists,
                                                 threshold, reliability);

        this->add_to_index(hists[best.first], labels[best.first],
                           threshold[best.first], reliability[best.first]);
        indexed[best.first] = true;
        current_error = best.second;
        cout << "Add (" << labels[best.first] << ") "
             << best.first << ": " << current_error
             << " (total: " << this->index_histogram.size() << ")"
             << endl;
        
        stringstream ss;
        string filename;
        ss <<  "results/phase_index/"
           << this->index_histogram.size() << ".jpg";
        ss >> filename;
        imwrite(filename, src[best.first]);
    }
    
    cout << "Done\n";
}
