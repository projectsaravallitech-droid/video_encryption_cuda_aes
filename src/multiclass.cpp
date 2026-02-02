#include <iostream>
#include<string>
using namespace std;

class Player{
  public:
    int hp;
    void spawn(){
      hp = 100;
    }
};

class Enemy{
  public:
    int hp;
    virtual void attack(Player &player)=0;
    virtual void move() = 0;
};

class Goblin: public Enemy{
  public:
    Goblin(){
      hp = 100;
      cout<<"Goblin Enemy is initialized"<<endl;
    }
    void attack(Player& player){
      cout<<"The goblin attacks player with sword and drains 10 hp of player"<<endl;
      player.hp = player.hp - 10;
    }
    void move(){
      cout<<"Goblin moves 2 spaces at a time"<<endl;
    }
};

class Skeleton: public Enemy{
  public:
    Skeleton(){
      hp = 100;
      cout<<"Skeleton Enemy is initialized"<<endl;
    }
    void attack(Player &player){
      player.hp = player.hp - 15;
      cout<<"The skeleton attacks the player with hammer and drains 15 hp pts";
    }
    void move(){
      cout<<"The skeleton moves 1 space at a time"<<endl;
    }
};

class Archer: public Enemy{
  public:
    Archer(){
      hp = 30;
      cout<<"Archer Enemy is initialized"<<endl;
    }
    void attack(Player &player){
      player.hp = player.hp - 15;
      cout<<"The archer attacks the player and drains 15 hp pts"<<endl;
    }
    void move(){
      cout<<"The archer moves 1 space at a time"<<endl;
    }
};

int main(){
  string enemy = "goblin";
  Enemy* enemy_obj;
  Player player;
  //The main disadvantage of handling multiple classes individually is shown below
  if(enemy == "goblin"){
    enemy_obj = new Goblin();
  }
  else if(enemy == "skeleton"){
    enemy_obj = new Skeleton();
  }
  else if(enemy == "archer"){
    enemy_obj = new Archer();
  }
  else{
    enemy_obj = nullptr;
  }
  //we need to know each and every class and the functions it has even though some related classes perform the same things, which in this case is observed in the enemy classes
  
  enemy_obj->attack(player);
  enemy_obj->move();
  //Even if we have decreased the need of remembering different functions for different class with the use of virtual functions and inheritance, the code is still messy
  //Hence to reduce the code complexity, we need simple factory pattern

}