#include "anonadado_utils.hpp"

std::string rapidjson_get_string(const rapidjson::Value& v, std::string field,
                                 std::string default_value)
{
    const char* f = field.c_str();

    if ( !v.HasMember(f) || !v[f].IsString() )
    {
        return default_value;
    } else {
        return v[f].GetString();
    }
}

int rapidjson_get_int(const rapidjson::Value& v, std::string field,
                      int default_value)
{
    const char* f = field.c_str();

    if ( !v.HasMember(f) || !v[f].IsInt() )
    {
        return default_value;
    } else {
        return v[f].GetInt();
    }
}

bool rapidjson_get_bool(const rapidjson::Value& v, std::string field,
                        bool default_value)
{
    const char* f = field.c_str();

    if ( !v.HasMember(f) || !v[f].IsBool() )
    {
        return default_value;
    } else {
        return v[f].GetBool();
    }
}

float rapidjson_get_float(const rapidjson::Value& v, std::string field,
                          float default_value)
{
    const char* f = field.c_str();
    
    if ( !v.HasMember(f) || !v[f].IsDouble() )
    {
        return default_value;
    } else {
        return (float)v[f].GetDouble();
    }
}

BBOX rapidjson_get_bbox(const rapidjson::Value& v, std::string field,
                        BBOX default_value)
{
    const char* f = field.c_str();
    BBOX ret = default_value;
    
    if ( v.HasMember(f) && v[f].IsArray() )
    {
        int index = 0;
        int* p[4] = {&ret.first.first, &ret.first.second,
                     &ret.second.first, &ret.second.second};
        
        for (rapidjson::SizeType i = 0; i < v[f].Size(); i++){
            const rapidjson::Value& v_json = v[f][i];

            for (rapidjson::SizeType j = 0; j < v_json.Size(); j++){
                const rapidjson::Value& vv_json = v_json[j];

                if ( vv_json.IsInt() ){
                    *p[index] = vv_json.GetInt();
                }
                index++;
            }
        }
    }
    
    return ret;
}

POINT rapidjson_get_point(const rapidjson::Value& v, std::string field,
                          POINT default_value)
{
    const char* f = field.c_str();
    POINT ret = default_value;

    if ( v.HasMember(f) && v[f].IsArray() )
    {
        int index = 0;
        int* p[2] = {&ret.first, &ret.second};

        for (rapidjson::SizeType i = 0; i < v[f].Size(); i++){
            const rapidjson::Value& v_json = v[f][i];

            if ( v_json.IsInt() ){
                *p[index] = v_json.GetInt();
            }
            index++;
        }
    }

    return ret;
}
