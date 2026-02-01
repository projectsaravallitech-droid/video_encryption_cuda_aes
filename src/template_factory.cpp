//Template factory code - considering the example of a game 
#include<iostream>
#include<map>
#include<string>
#include<functional>
using namespace std;

//Template fcatory class
template<typename T>
class Factory{
    static map<string, function<T*()>> hash;

  public:
    static void get_value(const string& type, function<T*()> createObj){
      hash[type] = createObj;
    }

    static T* create(const string& type){
      auto it = hash.find(type);
      if(it != hash.end()){
        return it->second();
      }
      else{
        return nullptr;
      }
    }
};
template<typename T>
map<string, function<T*()>> Factory<T>::hash;

//Sample player class
class Player{
  public:
    int hp;
    void spawn(){
      hp = 100;
    }
    void move(){
      cout<<"The player is moving";
    }
};

//base class - abstract - to act as a template for the other enemy classes
class Enemy{
  public:
    virtual void attack(Player& obj) = 0;
    virtual void move() = 0;
};
class Goblin: public Enemy{
  public:
    void attack(Player &obj){
      obj.hp = obj.hp - 10;
      cout<<"The goblin attacks the player and drains 10 hp pts";
    }
    void move(){
      cout<<"The goblin moves 2 spaces at a time";
    }
};
class Archer: public Enemy{
  public:
    void attack(Player &obj){
      obj.hp = obj.hp - 5;
      cout<<"The archer attacks the player and drains 5 hp pts";
    }
    void move(){
      cout<<"The archer moves 1 space at a time";
    }
};

//base class - abstract - to act as a template for the other weapon classes
class Weapon{
  public:
    virtual void use() = 0;
};
class Sword: public Weapon{
  public:
    void use(){
      cout<<"Sword is swung"<<endl;
    }
};
class Bow:public Weapon{
  public:
    void use(){
      cout<<"Bow is used to shoot arrow";
    }
};

int main(){
  Player player;

   Factory<Enemy>::get_value("goblin", [](){ return new Goblin(); });
   Factory<Enemy>::get_value("archer", [](){ return new Archer(); });

   Factory<Weapon>::get_value("sword", [](){ return new Sword(); });
   Factory<Weapon>::get_value("bow", [](){return new Bow(); });

   Enemy* goblin_obj = Factory<Enemy>::create("goblin");
   Enemy* archer_obj = Factory<Enemy>::create("archer");

   Weapon* sword_obj = Factory<Weapon>::create("sword");
   Weapon* bow_obj = Factory<Weapon>::create("bow");

   goblin_obj->attack(player);
   archer_obj->attack(player);

   sword_obj->use();
   bow_obj->use();

   return 0;

}