#include "ranking.hpp"

using namespace rank_learning;

ranking::ranking()
{
    
}

ranking::~ranking()
{
    
}

void ranking::train(std::vector<sample>& samples,
                    std::vector< pair<int, int> >& feedback)
{
    
}

float ranking::predict(sample& a, sample& b)
{
    
}

void ranking::rank(std::vector<sample>& samples)
{
    
}

void ranking::rank(std::vector<sample>& samples,
                   std::vector<int>& positions)
{
    
}


float ranking::accuracy(std::vector<sample>& samples,
                        std::vector< pair<int, int> >& feedback)
{
    if ( feedback.size() == 0 )
        return 1.0;

    vector<int> order;
    this->rank(samples, order);

    int tt = 0;
    int ff = 0;

    for (size_t i = 0; i < feedback.size(); i++)
    {
        int f = feedback[i].first;
        int s = feedback[i].second;

        if ( order[f] > order[s] ){
            tt++;
        } else if ( order[f] < order[s] ){
            ff++;
        } else {

        }
    }

    return ((float)tt) / ((float)(tt + ff));
}

l2r_ranking::l2r_ranking(float B, size_t num_iterations)
{
    this->beta = B;
    this->num_iterations = num_iterations;
}

l2r_ranking::~l2r_ranking()
{
    
}

void l2r_ranking::train(vector< sample >& samples,
                        vector< pair<int, int> >& feedback)
{
    if ( samples.size() == 0 ){
        this->w.clear();
        return;
    }
    
    this->w.resize(samples[0].size());
    fill(this->w.begin(), this->w.end(), 1.0);
    this->normalize_weights();
    
    for ( size_t iteration = 0; iteration < this->num_iterations; iteration++ )
    {
        // Evaluate losses
        vector<float> loss;
        this->loss(samples, feedback, loss);
        
        // Set the new weight vector
        this->update_weights(loss);
    }
    /*
    */
    vector< pair<float, int> > wo;
    for ( size_t i = 0; i < this->w.size(); i++ ){
        wo.push_back(make_pair(-this->w[i], i));
    }
    sort(wo.begin(), wo.end());
    /*
    cout << "Weight:";
    for ( size_t i = 0; i < wo.size(); i++ ){
        cout << " (" << wo[i].second << ", " << abs(wo[i].first) << ")";
    }
    cout << endl;
    */
}

float l2r_ranking::predict(sample& a, sample& b)
{
    size_t n = min(a.size(), b.size());
    float ret = 0.0;
    
    for ( size_t i = 0; i < n; i++ ){
        ret += this->w[i] * this->R(a, b, i);
    }
    
    return ret;
}

void l2r_ranking::rank(vector<sample>& samples)
{
    vector<int> total_order;
    greedy_order(samples, total_order);
    size_t n = samples.size();
    
    vector<sample> aux;
    aux.resize(n);
    
    for ( size_t i = 0; i < n; i++ ){
        aux[total_order[i]] = samples[i];
    }
    
    samples = aux;
}

void l2r_ranking::rank(std::vector<sample>& samples,
                       std::vector<int>& positions)
{
    this->greedy_order(samples, positions);
}

float l2r_ranking::Z()
{
    float ret = 0.0;
    vector<float>::iterator it=this->w.begin(), end=this->w.end();
    
    for ( ; it != end; ++it ){
        ret += *it;
    }
    
    return ret;
}

void l2r_ranking::normalize_weights()
{
    float z = this->Z();
    if ( z == 0.0 ){
        z = 1e-6;
    }
    
    size_t n = this->w.size();
    
    for ( size_t i = 0; i < n; i++ ){
        this->w[i] /= z;
    }
}

void l2r_ranking::loss(vector<sample>& samples,
                       vector< pair<int, int> >& feedback,
                       vector<float>& loss
                      )
{
    loss.resize(this->w.size());
    fill(loss.begin(), loss.end(), 1.0);
    
    if ( feedback.size() == 0 ){
        return;
    }
    
    for ( size_t i = 0; i < this->w.size(); i++ ){
        vector< pair<int, int> >::iterator it=feedback.begin(),
                                           end=feedback.end();
        size_t ret = 0;
        
        for ( ; it != end; ++it ){
            pair<int, int> next_pair = *it;
            ret += this->R(samples[next_pair.first], samples[next_pair.second],
                           i);
        }
        
        loss[i] = /*1.0 - */(float) ret / (float) feedback.size();
    }
}

float l2r_ranking::R(sample& a, sample& b, int i)
{
    float ret = 0.5;
    if ( abs(a[i] - b[i]) < 1e-5 ){
        ret = 0.5;
    } else if ( a[i] < b[i] ){
        ret = 1.0;
    } else {
        ret = 0.0;
    }
    return ret;
}

void l2r_ranking::update_weights(vector<float>& loss)
{
    size_t n = this->w.size();
    
    for ( size_t i = 0; i < n; i++ ){
        this->w[i] = this->w[i] * pow(this->beta, loss[i]);
    }
    
    this->normalize_weights();
}

int get_max_v(vector<float>& V, vector<bool>& taken)
{
    int ret = -1;
    size_t n = V.size();
    
    for ( size_t i = 0; i < n; i++ ){
        if ( !taken[i] ){
            if ( ret == -1 || V[ret] < V[i] ){
                ret = i;
            }
        }
    }
    
    return ret;
}

void l2r_ranking::update_V(vector<sample>& samples, vector<float>& V,
                           vector<bool>& taken)
{
    size_t n = V.size();
    for ( size_t i = 0; i < n; i++ ){
        if ( !taken[i] ){
            V[i] = this->compute_greedy_V(samples, i, taken);
        }
    }
}

void l2r_ranking::greedy_order(vector<sample>& samples,
                               vector<int>& total_order)
{
    vector<float> V;
    vector<bool> taken;
    size_t n = samples.size();
    size_t remaining = n;
    V.resize(n);
    taken.resize(n);
    total_order.resize(n);
    
    fill(taken.begin(), taken.end(), false);
    fill(total_order.begin(), total_order.end(), -1);
    
    this->update_V(samples, V, taken);
    while ( remaining > 0 ){
        int t = get_max_v(V, taken);
        
        total_order[t] = n - remaining;
        remaining--;
        
        taken[t] = true;
        
        this->update_V(samples, V, taken);
    }
}

float l2r_ranking::compute_greedy_V(vector<sample>& samples,
                                    int v, vector<bool>& taken)
{
    float ret = 0.0;
    size_t n = samples.size();
    
    for ( size_t i = 0; i < n; i++ ){
        if ( !taken[i] ){
            ret += this->predict(samples[v], samples[i]) - 
                   this->predict(samples[i], samples[v]);
        }
    }
    
    return ret;
}

classifier_ranking::classifier_ranking()
{
    this->cl = 0;
}

classifier_ranking::classifier_ranking(classifier* c)
{
    this-> cl = c;
}
            
classifier_ranking::~classifier_ranking()
{
    
}

Mat from_samples(sample& a, sample& b)
{
    Mat ret(1, a.size(), CV_32F);
    for (int j=0; j<a.size(); j++){
        ret.at<float>(0, j) = (a[j] < b[j] ? 1.0 : 0.0);
    }
    return ret;
}

void classifier_ranking::train(std::vector<sample>& samples,
                               std::vector< pair<int, int> >& feedback)
{
     int num_samples = feedback.size() * 2;
     vector<Mat> tr_samples;
     vector<label> labels;
     
     for(size_t i=0; i<feedback.size(); i++){
         int f1 = feedback[i].first;
         int f2 = feedback[i].second;
         
         Mat spos = from_samples(samples[f1], samples[f2]);
         tr_samples.push_back(spos);
         labels.push_back(1);
         
         Mat sneg = from_samples(samples[f1], samples[f2]);
         tr_samples.push_back(sneg);
         labels.push_back(0);
     }
}

float classifier_ranking::predict(sample& a, sample& b)
{
    Mat s = from_samples(a, b);
    return this->cl->predict(s);
}
