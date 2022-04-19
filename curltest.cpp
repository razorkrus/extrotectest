#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include "curl/curl.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/reader.h"
#include <glog/logging.h>
#include <opencv2/imgcodecs.hpp>
#include <stdlib.h>

// using std::string;
// using std::vector;
using namespace rapidjson;
using namespace std;
using namespace cv;

size_t write_func(void *ptr, size_t size, size_t nmemb, string *s){
    s->append(static_cast<char *>(ptr), size*nmemb);
    return size*nmemb;
}


void get_info(string ip_address){
    /*
     * Function used to get info from backend server.
     * Used to get the road config json file from backend.
     */
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl){
        string get_url = "http://" + ip_address + ":8093/data/road/info/test01";
        curl_easy_setopt(curl, CURLOPT_URL, get_url.c_str());
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
        string s;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        res = curl_easy_perform(curl);
        LOG_IF(ERROR, res!=CURLE_OK) << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;

        LOG(INFO) << "s is: " << s << endl;

        Document doc;
        doc.Parse(s.c_str(), s.size());
        LOG(INFO) << "config is: " << doc["data"]["config"].GetString() << endl;

        LOG(INFO) << " config is string: " << doc["data"]["config"].IsString() << endl;


        string t = doc["data"]["config"].GetString();
        Document d;
        d.Parse(t.c_str(), t.size());

        LOG(INFO) << "d is object: " << d.IsObject() << endl;
        LOG(INFO) << "d has BIKE_AREA: " << d.HasMember("BIKE_AREA") << endl;
        LOG(INFO) << "bike_area is array: " << d["BIKE_AREA"].IsArray() << endl;

        // if (d.HasMember("BIKE_AREA"))
        // {
        //     const Value &a = d["BIKE_AREA"];
        //     for (SizeType i = 0; i < a.Size(); i++)
        //         LOG(INFO) << a[i].GetString() << endl;
        // }
        vector<string> keys = {"BIKE_AREA","CAR_AREA", "WARNING_AREA_1", "WARNING_AREA_2"};

        for (auto k:keys)
        {
            if (d.HasMember(k.c_str()))
                    {
                        LOG(INFO) << "====== Printing " << k << " ======" << endl;
                        const Value &a = d[k.c_str()];
                        for (SizeType i = 0; i < a.Size(); i++){
                            // LOG(INFO) << a[i].GetString() << endl;
                            string tt = a[i].GetString();
                            tt = tt.substr(1, tt.size()-2);
                            auto index = tt.find(",");
                            int x = stoi(tt.substr(0, index));
                            int y = stoi(tt.substr(index+2, tt.size()-1));
                            LOG(INFO) << x << " " << y << endl;
                            // LOG(INFO) << stoi(t.substr(0, index)) << " " << stoi(tt.substr(index+2, tt.size()-1)) << endl;
                            // LOG(INFO) << tt.substr(1, tt.size()-2) << endl;
                        }
                            
                    }
        }
        // StringBuffer buffer;
        // Writer<StringBuffer> writer(buffer);
        // doc.Accept(writer);

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}


void post_json_data(string server_url, string json_data){
    /*
     * Function used to post json data to backend server.
     */
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl){
        curl_easy_setopt(curl, CURLOPT_URL, server_url.c_str());
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}




vector<string> post_image_files(string server_url, vector<string> image_files){
    /* 
     * Function used to post image files to backend server.
     * Used by the violation data post function.
     */
    CURL *curl;
    CURLcode res;

    curl_mime *form = NULL;
    curl_mimepart *field = NULL;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    vector<string> r;

    if (curl){
        form = curl_mime_init(curl);

        field = curl_mime_addpart(form);
        curl_mime_name(field, "module");
        curl_mime_data(field, "10", CURL_ZERO_TERMINATED);

        field = curl_mime_addpart(form);
        curl_mime_name(field, "isDir");
        curl_mime_data(field, "False", CURL_ZERO_TERMINATED);

        for (auto file : image_files){
            field = curl_mime_addpart(form);

            // Code used to upload image files from harddisk.
            // curl_mime_name(field, "file");
            // curl_mime_filedata(field, file.c_str());

            // Code used to upload image files from memory.
            curl_mime_name(field, "file");
            
            // Need to change it to receive the parameters
            Mat m = imread(file, IMREAD_COLOR);

            LOG(INFO) << m.size() << endl;
            vector<uchar> vec_img;
            imencode(".jpg", m, vec_img);
            LOG(INFO) << vec_img.size() << endl;

            curl_mime_filename(field, file.c_str());

            // string rand_file_name = to_string(rand());
            // rand_file_name += ".jpg";
            // curl_mime_filename(field, rand_file_name.c_str());

            curl_mime_data(field, reinterpret_cast<const char*>(vec_img.data()), vec_img.size());

        }

        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
        curl_easy_setopt(curl, CURLOPT_URL, server_url.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
        string s;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        res = curl_easy_perform(curl);
        LOG(INFO) << s << endl;

        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));


        Document document;
        document.Parse(s.c_str(), s.size());
        const Value& a = document["data"];
        for (SizeType i=0; i < a.Size(); i++){
            // cout << a[i]["id"].GetString() << endl;
            r.push_back(a[i]["id"].GetString());
        }

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return r;
}


void test_heartbeat(string ip_address){
    /*
     * Function usede to test curl post api and rapidjson api.
     */
    string json_upload_url = "http://" + ip_address + ":8093/data/road/add";
    string json_data ="{\"location\": \"nowhere\", \"nodeCode\": \"test_10003\", \"nodeName\": \"Nanjing Qilin\" }";

    Document document;
    document.Parse(json_data.c_str(), json_data.size());

    // Document document;
    // document.SetObject();
    
    document.AddMember("this is new", "and what the hell?", document.GetAllocator());
    document.AddMember("time", 123456789, document.GetAllocator());

    StringBuffer s;
    Writer<StringBuffer> writer(s);
    document.Accept(writer);

    LOG(INFO) << "StringBuffer s is: " << s.GetString() << endl;
    
    post_json_data(json_upload_url, json_data);

    LOG(INFO) << "Heartbeat data posted!" << endl;
}


void post_violation(string ip_address, string json_incomp){
    /*
     * Function used to post single violation data.
     * First post three images to the backend server, get an id for each image.
     * Then the id info is added to a json data, and the json data was posted to
     * the backend again.
     */

    // Step 1: post images
    string image_upload_url = "http://" + ip_address + ":8093/api/file/upload";
    vector<string> image_names = {"325185.jpg", "2693529.jpg"};
    
    // Post images to backend
    vector<string> image_ids = post_image_files(image_upload_url, image_names);
    
    LOG(INFO) << "Image ids are: " << image_ids[0] << endl;

    // Step 2: post json
    string json_upload_url = "http://" + ip_address + ":8093/api/record/insert";
    Document doc;
    doc.Parse(json_incomp.c_str(), json_incomp.size());

    Value v;
    v.SetString(image_ids[0].c_str(), image_ids[0].size(), doc.GetAllocator());
    doc.AddMember("imgOne", v, doc.GetAllocator());

    v.SetString(image_ids[1].c_str(), image_ids[1].size(), doc.GetAllocator());
    doc.AddMember("imgTwo", v, doc.GetAllocator());

    // v.SetString(image_ids[2].c_str(), image_ids[2].size(), doc.GetAllocator());
    // doc.AddMember("imgThree", v, doc.GetAllocator());

    StringBuffer s;
    Writer<StringBuffer> writer(s);
    doc.Accept(writer);

    string json_comp = s.GetString();

    post_json_data(json_upload_url, json_comp);
 
}

void test_violation(string ip_address){
    /*
     * Function used to test individual funtions used inside a violation post.
     */
    string image_upload_url = "http://" + ip_address + ":8093/api/file/upload";
    vector<string> image_names = {"325185.jpg", "2693529.jpg"};
    post_image_files(image_upload_url, image_names);
}


int main(int argc, char* argv[]){
    google::InitGoogleLogging(argv[0]);
    string ip_address = "192.168.0.91";

    get_info(ip_address);

    // test_heartbeat(ip_address);

    Document doc;
    doc.SetObject();

    doc.AddMember("description", "No info", doc.GetAllocator());
    doc.AddMember("nodeId", "test_10007", doc.GetAllocator());
    doc.AddMember("time", 123456789, doc.GetAllocator());
    doc.AddMember("type", "20001", doc.GetAllocator());

    StringBuffer s;
    Writer<StringBuffer> writer(s);
    doc.Accept(writer);

    string json_incomp = s.GetString();

    // post_violation(ip_address, json_incomp);

}



