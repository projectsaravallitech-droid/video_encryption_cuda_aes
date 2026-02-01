#include <iostream>
#include <memory>
using namespace std;

// Lazy Singleton with unique_ptr
class Singleton1 {
private:
    static unique_ptr<Singleton1> obj;
    
    Singleton1() {
        cout << "Object of Singleton1 created" << endl;
    }
    
    // Delete copy operations
    Singleton1(const Singleton1& dupl) = delete;
    Singleton1& operator=(const Singleton1& dupl) = delete;
    
    // Delete move operations
    Singleton1(Singleton1&& dupl) = delete;
    Singleton1& operator=(Singleton1&& dupl) = delete;

public:
    static Singleton1& get_object() {
        if (obj == nullptr) {
            obj.reset(new Singleton1());
        }
        return *obj;
    }
    
    ~Singleton1() {
        cout << "Singleton1 destroyed" << endl;
    }
};
unique_ptr<Singleton1> Singleton1::obj = nullptr;


// Eager singleton implementaton
class Singleton2 {
private:
    static unique_ptr<Singleton2> obj;
    
    Singleton2() {
        cout << "Object of Singleton2 created" << endl;
    }
    
    Singleton2(const Singleton2& dupl) = delete;
    Singleton2& operator=(const Singleton2& dupl) = delete;
    Singleton2(Singleton2&& dupl) = delete;
    Singleton2& operator=(Singleton2&& dupl) = delete;

public:
    static Singleton2& get_object() {
        return *obj;
    }
    
    ~Singleton2() {
        cout << "Singleton2 destroyed" << endl;
    }
};
// Eager initialization
unique_ptr<Singleton2> Singleton2::obj = unique_ptr<Singleton2>(new Singleton2());


int main() {
    Singleton1& obj1 = Singleton1::get_object();
    Singleton2& obj2 = Singleton2::get_object();
    
    // this code will not compile
    // Singleton1 copy = obj1;
    // copy = obj1;
    
    // Duplicate references can be created
    Singleton1& ref = obj1;
    Singleton2& ref2 = Singleton2::get_object();
    
    return 0;
}