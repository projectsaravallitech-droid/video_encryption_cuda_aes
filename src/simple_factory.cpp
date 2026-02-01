//Simple factory pattern implementation using the example of a game where the enemies are represented by classes
#include<iostream>
#include<memory>
using namespace std;

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

//actual enemies we want to implement
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

class Skeleton: public Enemy{
  public:
    void attack(Player &obj){
      obj.hp = obj.hp - 15;
      cout<<"The skeleton attacks the player and drains 15 hp pts";
    }

    void move(){
      cout<<"The skeleton moves 1 space at a time";
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

class EnemyFactory{
  public:
    static unique_ptr<Enemy> get_enemy(string type){
      if(type == "goblin"){
        return make_unique<Goblin>();
      }
      else if(type == "skeleton"){
        return make_unique<Skeleton>();
      }
      else if(type == "archer"){
        return make_unique<Archer>();
      }
      else{
        return nullptr;
      }
    }
};

int main(){
  //here, instead of making different ibjects and if-else loops for the individual enemies, we just use the enemy factory class and create the required object
  //main advantage of using factory is that we do not need to handle individual classes every time we want to handle the enemy logic
  unique_ptr<Enemy> goblin_obj = EnemyFactory::get_enemy("goblin");
  unique_ptr<Enemy> archer_obj = EnemyFactory::get_enemy("archer");
  unique_ptr<Enemy> skeleton_obj = EnemyFactory::get_enemy("skeleton");

  Player player;
  
  goblin_obj->attack(player);
  goblin_obj->move();

  archer_obj->attack(player);
  skeleton_obj->attack(player);

  return 0;

}