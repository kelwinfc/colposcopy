#ifndef __ANONADADO_API
#define __ANONADADO_API

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filestream.h"
#include "rapidjson/stringbuffer.h"

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "utils.hpp"

namespace anonadado {

    class feature {
        protected:
            std::string name;
            std::string type;
        public:
            feature();
            feature(feature& a);

            virtual void read(const rapidjson::Value& v,
                              bool just_value=false);
            void read(std::string filename, bool just_value);
            
            std::string get_type();
            std::string get_name();
    };

    /* Bool Feature */
    class bool_feature : public feature {
        protected:
            bool default_value;
            bool value;

        public:
            bool_feature();
            bool_feature(bool_feature& f);
            
            virtual void read(const rapidjson::Value& v,
                              bool just_value=false);

            bool get_value();
    };

    /* String Feature */
    class str_feature : public feature {
        protected:
            std::string default_value;
            std::string value;
        
        public:
            str_feature();
            str_feature(str_feature& f);
            
            virtual void read(const rapidjson::Value& v,
                              bool just_value=false);

            std::string get_value();
            
    };

    /* Float Feature */
    class float_feature : public feature {
        protected:
            float default_value;
            float value;

        public:
            float_feature();
            float_feature(float_feature& f);
            
            virtual void read(const rapidjson::Value& v,
                              bool just_value=false);

            float get_value();
            
    };

    /* Int Feature */
    class int_feature : public feature {
        protected:
            int default_value;
            int value;

        public:
            int_feature();
            int_feature(int_feature& f);
            
            virtual void read(const rapidjson::Value& v,
                              bool just_value=false);

            int get_value();
            
    };
    
    /* Choice Feature */
    class choice_feature : public feature {
        protected:
            int default_value;
            int value;
            std::vector<std::string> values;
        
        public:
            choice_feature();
            choice_feature(choice_feature& f);
            
            virtual void read(const rapidjson::Value& v,
                              bool just_value=false);

            std::string get_value();
            
    };
    
    /* Bounding Box Feature */
    class bbox_feature : public feature {
        protected:
            BBOX default_value;
            BBOX value;
        
        public:
            bbox_feature();
            bbox_feature(bbox_feature& f);
            
            virtual void read(const rapidjson::Value& v,
                              bool just_value=false);
            
            BBOX get_value();
            
    };
    
    /* Vector Feature */
    class vector_feature : public bbox_feature {
        public:
            vector_feature();
            vector_feature(vector_feature& f);
    };
    
    /* Point Feature */
    class point_feature : public feature {
        protected:
            POINT default_value;
            POINT value;

        public:
            point_feature();
            point_feature(point_feature& f);
            
            virtual void read(const rapidjson::Value& v,
                              bool just_value=false);

            POINT get_value();
            
    };
    
    /*************************************************************************/
    
    class annotation {
        private:
            int frame;
            std::string name;
            bool is_unique;
            bool is_global;

            std::map<std::string, feature*> features;
        
        public:
            annotation();
            annotation(annotation& a);
            
            ~annotation();
            
            void read(const rapidjson::Value& v, bool just_value = false);
            void read(std::string filename);

            std::string get_name();
            int get_frame();
            feature* get_feature(std::string name);
            void get_features(std::vector<std::string>& f);
        
        private:
            void clear_features();
    };
    
    class domain {

        private:
            std::map<std::string, annotation*> labels;
            std::string name;
        
        public:
            domain();

            void read(std::string filename);
            
            annotation* get_descriptor(std::string label_name);
            annotation* get_instance(std::string label_name);
        
        private:
            void clear_labels();
    };
    
    class instance {
        private:
            domain* d;
            std::vector< std::vector<annotation*> > annotations;
            
            std::string domain_filename;
            
            std::string instance_name;
            std::string instance_filename;

            std::string video_filename;
            std::string sequence_filename;
        
        public:
            instance();
            ~instance();
            
            void read(std::string filename);
            void get_active_annotations(int frame_number,
                                        std::vector<int>& annotation_index);
            annotation* get_active_annotation(int index, int frame);
            void get_frame(int index, cv::Mat& dst);
            int num_frames();
            void get_annotations(std::string name, std::vector<int>& out);
        
        private:
            void clear_annotations();
    };
};

#endif
