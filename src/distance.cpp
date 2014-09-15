#include "distance.hpp"

v_distance::v_distance()
{
    // noop
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

lk_distance::lk_distance()
{
    this->k = 1;
}

lk_distance::lk_distance(int k)
{
    this->k = k;
    this->k_inv = 1.0 / k;
}

float lk_distance::d(vector<float>& a, vector<float>& b)
{
    float ret = 0;
    size_t n = min(a.size(), b.size());
    
    for ( size_t i = 0; i < n; i++ ){
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

manhattan_distance::manhattan_distance()
{
    this->k = 1;
    this->k_inv = 1.0;
}

void manhattan_distance::read(const rapidjson::Value& json)
{
    //TODO
}


void manhattan_distance::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

euclidean_distance::euclidean_distance()
{
    this->k = 2;
    this->k_inv = 1.0 / this->k;
}

void euclidean_distance::read(const rapidjson::Value& json)
{
    //TODO
}


void euclidean_distance::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

hi_distance::hi_distance()
{
    // noop
}

float hi_distance::d(vector<float>& a, vector<float>& b)
{
    float ret = 0.0;
    
    for ( uint i = 0; i < a.size(); i++ ){
        ret += min(a[i], b[i]) / (a[i] + b[i] + 1e-6);
    }

    return (float)a.size() - ret;
}

void hi_distance::read(const rapidjson::Value& json)
{
    //TODO
}

void hi_distance::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}

earth_movers_distance::earth_movers_distance()
{
    
}

float earth_movers_distance::shifted_d(vector<float>& a, vector<float>& b,
                                       size_t shift)
{
    /*
    float prev_emd = 0.0;
    float current_emd = 0.0;
    */
    float ret = 0.0;
    size_t n = a.size();
    size_t k = shift;
    
    float prev_A = 0.0;
    float prev_B = 0.0;
    
    for (size_t i = 0; i < n; i++){
        if ( k == n )
            k = 0;
        
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
    return ret;
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

circular_emd::circular_emd()
{
    
}

float circular_emd::d(vector<float>& a, vector<float>& b)
{
    float ret = this->shifted_d(a, b, 0);
    size_t n = a.size();
    
    for ( size_t shift = 1; shift < n; shift++ ){
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
