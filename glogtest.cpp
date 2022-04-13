#include <glog/logging.h>

int main(int argc, char* argv[]){
    google::InitGoogleLogging(argv[0]);
    LOG(INFO) << "Hello world!" << std::endl;
    int num_cookies = 10;
    LOG(WARNING) << "Found " << num_cookies << " cookies";
}