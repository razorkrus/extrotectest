#include "curlclass.hpp"


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
    string ip_address = "192.168.0.91";
    thread th(ThreadUploader);
    th.detach();
    LOG(INFO) << "Post thread detached!" << endl;

    ViolationUploader* uploader_p = ViolationUploader::GetInstance(ip_address);
    LOG(INFO) << "Uploader pointer created" << endl;

    while (true){
        this_thread::sleep_for(chrono::seconds(rand() % 20));
        LOG(INFO) << "Sleep ended! Now creating data!" << endl;
        violationData vData;

        LOG(INFO) << "vData created!" << endl;

        vector<string> filenames = {"325185.jpg", "2693529.jpg"};
        LOG(INFO) << "filenames created!" << endl;
        LOG(INFO) << "vData.imgs.empty() :" << vData.imgs.empty() << endl;

        for (auto name : filenames){
            vData.imgs.push_back(imread(name, IMREAD_COLOR));
        }
        LOG(INFO) << "vData.imgs[0].empty() is: "<<vData.imgs[0].empty() << endl;
        LOG(INFO) << "Image data pushed to the struct!" << endl;

        vData.json = "{\"project\":\"rapidjson\",\"stars\":10}";
        LOG(INFO) << "Json data pushed to the struct!" << endl;

        LOG(INFO) << "Struct pointer added to the queue!" << endl;
        LOG(INFO) << "&vData is: " << &vData << endl;
        uploader_p->collectInfo(pvData);
        
    }
    /*
     * There after should have some running code processing and analyzing video stream.
     * And use the ->collectInfo interface to put results inside the message queue.
    */

   

}



