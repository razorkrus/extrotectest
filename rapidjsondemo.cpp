#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/reader.h"
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <map>

using namespace std;
using namespace rapidjson;

void create_new_json(){
    /*
     * Create a new DOM object to hold key value pair, and transform it into json.
     */

    // Create a new Document object
    Document doc;
    doc.SetObject();

    // Use literals as input to set key value pair 
    // the second literal as parameter was cast to Value type automatically
    doc.AddMember("test_key", "test_keyvalue", doc.GetAllocator());
    doc.AddMember("time", 1234567890, doc.GetAllocator());

    // use StringBuffer to convert the Document into a const char* step by step
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    doc.Accept(writer);

    // Verify content: s.GetString() is what we want, a const char* to hold the json format info
    cout << "StringBuffer s content is: " << s.GetString() << endl;

    // Verify type: type name returns PKc, which is `const char*`
    cout << "s.GetString() type is: " << typeid(s.GetString()).name() << endl;
}


void parse_char_p(){
     /*
     * Parse char* format json into DOM, change the value of one key, and transform it back to json.
     */

    // Parse the char* format into DOM
    const char* json_char = "{\"project\":\"rapidjson\",\"stars\":10}";
    Document doc;
    doc.Parse(json_char);

    // Change the value of key "stars", from 10 to 11
    Value &s = doc["stars"];
    s.SetInt(s.GetInt() + 1);

    // Transform DOM into json
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);

    // Output {"project":"rapidjson", "stars":11}
    cout << buffer.GetString() << endl;
}   


void parse_string(){
     /*
     * Parse string format json into DOM, change the value of one key, and transform it back to json.
     */

    // Parse the string format into DOM
    string json_string = "{\"project\":\"rapidjson\",\"stars\":10}";
    Document doc;
    doc.Parse(json_string.c_str(), json_string.size());

    // Change the value of key "stars", from 10 to 11
    Value &s = doc["stars"];
    s.SetInt(s.GetInt() + 1);

    // Transform DOM into json
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    doc.Accept(writer);

    // Output {"project":"rapidjson", "stars":11}
    cout << buffer.GetString() << endl;
}


void parse_and_add_key(string json_old){
    /*
     * Parse string format json into DOM, and add new key value pair into it.
     */ 
    
    // Create a new Document object
    Document doc;

    // Parse the input string into Document format, need to cast string to c_str first
    doc.Parse(json_old.c_str(), json_old.size());

    // Key values to be added to json 
    vector<string> vs = {to_string(rand()), to_string(rand()), to_string(rand())};

    // Add those values one by one, need to convert string into Value format first
    // And then use AddMember method to pair the key and value
    Value k;
    Value v;

    map<int, string> numWords;
    numWords[1] = "One";
    numWords[2] = "Two";
    numWords[3] = "Three";

    int i = 1;
    for (auto s: vs){
        string key = "img" + numWords[i];
        k.SetString(key.c_str(), key.size(), doc.GetAllocator());
        v.SetString(s.c_str(), s.size(), doc.GetAllocator());
        doc.AddMember(k, v, doc.GetAllocator()); 
        i++;
    }

    // v.SetString(vs[1].c_str(), vs[1].size(), doc.GetAllocator());
    // doc.AddMember("imgTwo", v, doc.GetAllocator()); // added key "imgTwo"

    // v.SetString(vs[2].c_str(), vs[2].size(), doc.GetAllocator());
    // doc.AddMember("imgThree", v, doc.GetAllocator()); // added key "imgThree"

    // use StringBuffer to convert the Document into a const char* step by step
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    doc.Accept(writer);

    // Verify content: s.GetString() is what we want, a const char* to hold the json format info
    cout << "StringBuffer s content is: " << s.GetString() << endl;
}


int main(){
    create_new_json();
    parse_char_p();
    parse_string();

    string json_string ="{\"location\": \"nowhere\", \"nodeCode\": \"test_10003\", \"nodeName\": \"Nanjing Qilin\" }";
    parse_and_add_key(json_string);
}