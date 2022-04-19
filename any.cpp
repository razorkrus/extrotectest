#include <iostream>
#include <string>
#include <stdlib.h>

using namespace std;

class Tester{
private:
    int *ptr;
    string id;

public:
    Tester(string s) {
        id = s;
    }
    ~Tester() {}

    void Get(){        
        if (true){
            int *p = new int;
            cout << "p value is: " << p << endl;
            ptr = p;
        }
    }

    void Show(){
        cout << "Tester id is: " << id << endl;
        cout << "ptr value is: " << ptr << endl;
    }
};

int main(){
    Tester t = Tester("Helix");
    t.Get();
    t.Show();

    Tester s = Tester("Kritos");
    s.Get();
    s.Show();
}