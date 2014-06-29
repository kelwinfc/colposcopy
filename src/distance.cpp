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
        ret += pow(diff, this->k_inv);
    }
    return ret;
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
    float total = 0.0;

    for ( uint i = 0; i < a.size(); i++ ){
        ret += min(a[i], b[i]);
        total += max(a[i], b[i]);
    }

    return 1.0 - ret / total;
}

void hi_distance::read(const rapidjson::Value& json)
{
    //TODO
}

void hi_distance::write(rapidjson::Value& json, rapidjson::Document& d)
{
    //TODO
}
