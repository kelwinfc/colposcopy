#include "distance.hpp"


v_distance::v_distance(int first, int last)
{
    this->first = first;
    this->last = last;
}


void v_distance::read(string filename)
{
    //TODO
}

void v_distance::read(const rapidjson::Value& json)
{
    //TODO
}

void v_distance::write(string filename)
{
    //TODO
}

void v_distance::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

float v_distance::d(vector<float>& a, vector<float>& b)
{
    return 0.0;
}

void v_distance::set_first_and_last(int first, int last)
{
    this->first = first;
    this->last = last;
}

lk_distance::lk_distance(int first, int last)
{
    this->k = 1;
    this->first = first;
    this->last = last;
}

lk_distance::lk_distance(int k, int first, int last)
{
    this->k = k;
    this->k_inv = 1.0 / k;
    this->first = first;
    this->last = last;
}

float lk_distance::d(vector<float>& a, vector<float>& b)
{
    float ret = 0;

    size_t n = min(a.size(), b.size());
    size_t i = this->first;

    if ( this->last != -1 ){
        n = this->last;
    }

    for ( ; i < n; i++ ){
        float diff = abs(a[i] - b[i]);
        ret += pow(diff, this->k);
    }
    
    return pow(ret, this->k_inv);
}

void lk_distance::read(const rapidjson::Value& json)
{
    //TODO
}


void lk_distance::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

manhattan_distance::manhattan_distance(int first, int last)
{
    this->k = 1;
    this->k_inv = 1.0;
    this->first = first;
    this->last = last;
}

float manhattan_distance::d(vector<float>& a, vector<float>& b)
{
    float ret = 0;

    size_t n = min(a.size(), b.size());
    size_t i = this->first;

    if ( this->last != -1 ){
        n = this->last;
    }

    for ( ; i < n; i++ )
        ret += abs(a[i] - b[i]);
    
    return ret;
}

void manhattan_distance::read(const rapidjson::Value& json)
{
    //TODO
}


void manhattan_distance::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

euclidean_distance::euclidean_distance(int first, int last)
{
    this->k = 2;
    this->k_inv = 1.0 / this->k;
    this->first = first;
    this->last = last;
}

void euclidean_distance::read(const rapidjson::Value& json)
{
    //TODO
}


void euclidean_distance::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

hi_distance::hi_distance(int first, int last)
{
    this->first = first;
    this->last = last;
}

float hi_distance::d(vector<float>& a, vector<float>& b)
{
    float ret = 0.0;
    
    size_t n = min(a.size(), b.size());
    size_t i = this->first;

    if ( this->last != -1 ){
        n = this->last;
    }
    
    for ( ; i < n; i++ ){
        ret += min(a[i], b[i]);
    }
    
    return 1.0 - ret;
}

void hi_distance::read(const rapidjson::Value& json)
{
    //TODO
}

void hi_distance::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

earth_movers_distance::earth_movers_distance(int first, int last)
{
    this->first = first;
    this->last = last;
}

float earth_movers_distance::shifted_d(vector<float>& a, vector<float>& b,
                                       size_t shift)
{
    /*
    float prev_emd = 0.0;
    float current_emd = 0.0;
    */
    float ret = 0.0;
    size_t k = this->first + shift;
    
    size_t n = min(a.size(), b.size());
    
    if ( this->last != -1 ){
        n = this->last;
    }
    
    float prev_A = 0.0;
    float prev_B = 0.0;
    
    for (size_t i = this->first; i < n; i++){
        if ( k >= n )
            k = this->first;
        
        ret += abs((prev_A + a[k]) - (prev_B + b[k]));
        prev_A += a[k];
        prev_B += b[k];
        
        k++;
    }

    /*
    for (size_t i = 0; i < n; i++){
        
        if ( k == n )
            k = 0;
        
        prev_emd = current_emd;
        ret += abs(current_emd);
        
        current_emd = a[k] + prev_emd - b[k];
        k++;
    }
    */

    return ret / (n - this->first);
}

float earth_movers_distance::d(vector<float>& a, vector<float>& b)
{
    return this->shifted_d(a, b, 0);
}

void earth_movers_distance::read(const rapidjson::Value& json)
{
    //TODO
}

void earth_movers_distance::write(rapidjson::Value& json,
                                  rapidjson::Document& d)
{
    
}

circular_emd::circular_emd(int first, int last)
{
    this->first = first;
    this->last = last;
}

float circular_emd::d(vector<float>& a, vector<float>& b)
{
    float ret = this->shifted_d(a, b, 0);
    size_t num_shifts = a.size();
    
    if ( this->last != -1 ){
        num_shifts = this->last - this->first;
    }
    
    for ( size_t shift = 1; shift < num_shifts; shift++ ){
        if ( a[this->first + shift] == 0.0 && b[this->first + shift] == 0.0 ){
            continue;
        }
        ret = min(ret, this->shifted_d(a, b, shift));
    }
    
    return ret;
}

void circular_emd::read(const rapidjson::Value& json)
{
    //TODO
}

void circular_emd::write(rapidjson::Value& json, rapidjson::Document& d)
{
    
}

merge_distances::merge_distances(vector<v_distance*>& distances,
                                 vector<int>& sizes)
{
    this->distances = distances;
    this->sizes = sizes;
    
    this->shift.push_back(0);
    this->distances[0]->set_first_and_last(0, sizes[0]);
    
    for(size_t i=1; i<sizes.size(); i++){
        this->shift.push_back(this->shift[i - 1] + sizes[i - 1]);
        this->distances[i]->set_first_and_last(this->shift[i],
                                               this->shift[i] + sizes[i]);
    }
}
        
float merge_distances::d(vector<float>& a, vector<float>& b)
{
    float ret = 0.0;
    
    for (size_t i = 0; i < this->sizes.size(); i++){
        ret += this->distances[i]->d(a, b);
    }
    
    return ret;
}

void merge_distances::read(const rapidjson::Value& json)
{
    //TODO
}

void merge_distances::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}
