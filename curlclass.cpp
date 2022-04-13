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
#include <mutex>
#include <thread>
#include <queue>
#include <chrono>

using namespace rapidjson;
using namespace std;
using namespace cv;

/*
 * Struct to save every violation event data.
 */
struct violationData{
    vector<Mat> imgs;
    string json;
};


size_t write_func(void *ptr, size_t size, size_t nmemb, string *s){
    s->append(static_cast<char *>(ptr), size*nmemb);
    return size*nmemb;
}


/*****
 * Singleton class implementation of http post, using thread to post communication data.
 *****/
class ViolationUploader{
private:
    static ViolationUploader* pinstance_;
    static mutex mutex_instance;
    static mutex mutex_queue;
    queue<violationData *> info_q;
    string ip_address;
    string image_upload_url;
    string json_upload_url;

    vector<string> postImages(vector<Mat> imgs){
        /*
         * Post image files to backend server, and return the response id for each image.
         */
        LOG(INFO) << "Posting violation images to backend server!" << endl;
        CURL *curl;
        CURLcode res;

        curl_mime *form = nullptr;
        curl_mimepart *field = nullptr;

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

            for (auto m : imgs){
                field = curl_mime_addpart(form);
                curl_mime_name(field, "file");
                vector<uchar> vec_img;
                imencode(".jpg", m, vec_img);
                curl_mime_filename(field, "001.jpg"); // need to change later to add a suitable file name
                curl_mime_data(field, reinterpret_cast<const char*>(vec_img.data()), vec_img.size());
            }

            curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
            curl_easy_setopt(curl, CURLOPT_URL, image_upload_url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
            string s;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

            res = curl_easy_perform(curl);
            LOG_IF(ERROR, res!=CURLE_OK) << "curl_easy_perform() faild: " << curl_easy_strerror(res) << endl;

            Document doc;
            doc.Parse(s.c_str(), s.size());
            const Value& a = doc["data"];
            for (SizeType i=0; i < a.Size(); i++){
                r.push_back(a[i]["id"].GetString());
            }

            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
        return r;
    }
    
    void postJsonData(string json_data){
        CURL *curl;
        CURLcode res;

        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();

        if (curl){
            curl_easy_setopt(curl, CURLOPT_URL, json_upload_url.c_str());
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
            res = curl_easy_perform(curl);
            LOG_IF(ERROR, res!=CURLE_OK) << "curl_easy_perform() faild: " << curl_easy_strerror(res) << endl;
            curl_easy_cleanup(curl);
        }

        curl_global_cleanup();
    }

    void postJson(string json_incomp, vector<string> ids){
        LOG(INFO) << "Posting violation json info to backend server!" << endl;
        Document doc;
        doc.Parse(json_incomp.c_str(), json_incomp.size());

        // for (auto id : ids){
        //     Value v;
        //     v.SetString(id.c_str(), id.size(), doc.GetAllocator());
        //     doc.AddMember("imgOne", v, doc.GetAllocator());
        // }

        Value v;
        v.SetString(ids[0].c_str(), ids[0].size(), doc.GetAllocator());
        doc.AddMember("imgOne", v, doc.GetAllocator());
        v.SetString(ids[1].c_str(), ids[1].size(), doc.GetAllocator());
        doc.AddMember("imgTwo", v, doc.GetAllocator());
        v.SetString(ids[2].c_str(), ids[2].size(), doc.GetAllocator());
        doc.AddMember("imgThree", v, doc.GetAllocator());

        StringBuffer s;
        Writer<StringBuffer> writer(s);
        doc.Accept(writer);
        string json_comp = s.GetString();
        
        postJsonData(json_comp);
    }


protected:
    ViolationUploader(string ip_param){
        ip_address = ip_param;
        image_upload_url = "http://" + ip_address + ":8093/api/file/upload";
        json_upload_url = "http://" + ip_address + ":8093/api/record/insert";
    }
    ~ViolationUploader() {}

public:
    ViolationUploader(ViolationUploader &other) = delete;

    void operator=(const ViolationUploader &) = delete;

    static ViolationUploader *GetInstance(string ip_param);

    void collectInfo(violationData *data_p){
        lock_guard<mutex> lock(mutex_queue);
        info_q.push(data_p);
    };

    void postInfo(){
        int i = 0;
        while(true){
            if (info_q.empty()){
                i += 1;
                this_thread::sleep_for(chrono::milliseconds(1000));
                if (i % 10 == 0){
                    LOG(INFO) << "HTTP queue idle for 10 seconds" << endl;
                    i = 0;
                }
            }
            else{
                violationData *t;
                {
                    lock_guard<mutex> lock(mutex_queue);
                    t = info_q.front();
                    info_q.pop();
                }
                vector<string> respon_ids = postImages(t->imgs);
                postJson(t->json, respon_ids);
            }
        }

    }

};

/*
 * Supplymentary definition of class.
 */
ViolationUploader* ViolationUploader::pinstance_{nullptr};
mutex ViolationUploader::mutex_instance;
mutex ViolationUploader::mutex_queue;

ViolationUploader *ViolationUploader::GetInstance(string ip_param){
    lock_guard<mutex> lock(mutex_instance);
    if (pinstance_ == nullptr)
    {
        pinstance_ = new ViolationUploader(ip_param);
    }
    return pinstance_;
}


/*
 * Function to be passed to thread.
 * Initialize a ViolationUploader inside, and then start uploading constantly.
 */ 
void ThreadUploader(){
    string ip_address = "192.168.0.91";
    ViolationUploader* uploader = ViolationUploader::GetInstance(ip_address);
    uploader->postInfo();
}

/* 
 * main function used to test.
 */

int main(int argc, char* argv[]){
    thread th(ThreadUploader);
    th.detach();

    /*
     * There after should have some running code processing and analyzing video stream.
     * And use the ->collectInfo interface to put results inside the message queue.
    */

   

}



