#include "ranking.hpp"

using namespace rank_learning;

ranking::ranking()
{
    this->beta = 0.5;
    this->num_iterations = 100;
}

ranking::ranking(float B, size_t num_iterations)
{
    this->beta = B;
    this->num_iterations = num_iterations;
}

ranking::~ranking()
{
    
}

void ranking::train(vector< sample >& samples,
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
    
    cout << "Weight:";
    for ( size_t i = 0; i < this->w.size(); i++ ){
        cout << " " << this->w[i];
    }
    cout << endl;
}

float ranking::predict(sample& a, sample& b)
{
    size_t n = min(a.size(), b.size());
    float ret = 0.0;
    
    for ( size_t i = 0; i < n; i++ ){
        ret += this->w[i] * this->R(a, b, i);
    }
    
    return ret;
}

void ranking::rank(vector<sample>& samples)
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

float ranking::Z()
{
    float ret = 0.0;
    vector<float>::iterator it=this->w.begin(), end=this->w.end();
    
    for ( ; it != end; ++it ){
        ret += *it;
    }
    
    return ret;
}

void ranking::normalize_weights()
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

void ranking::loss(vector<sample>& samples,
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

float ranking::R(sample& a, sample& b, int i)
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

void ranking::update_weights(vector<float>& loss)
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

void ranking::update_V(vector<sample>& samples, vector<float>& V,
                       vector<bool>& taken)
{
    size_t n = V.size();
    for ( size_t i = 0; i < n; i++ ){
        if ( !taken[i] ){
            V[i] = this->compute_greedy_V(samples, i, taken);
        }
    }
}

void ranking::greedy_order(vector<sample>& samples,
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

float ranking::compute_greedy_V(vector<sample>& samples,
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
