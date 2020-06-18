#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include <string>
#include <list>
using namespace std;

// Constants for object type

const int ID_SOCRATES               = 0;
const int ID_REGULAR_SALMONELLA     = 1;
const int ID_AGGRESSIVE_SALMONELLA  = 2;
const int ID_ECOLI                  = 3;
const int ID_PIT                    = 4;
const int ID_FLAME                  = 5;
const int ID_SPRAY                  = 6;
const int ID_DIRT                   = 7;
const int ID_FOOD                   = 8;
const int ID_HEALTH_GOODIE          = 9;
const int ID_FLAME_GOODIE           = 10;
const int ID_LIFE_GOODIE            = 11;
const int ID_FUNGI                  = 12;

class Socrates;
class Actor;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
    ~StudentWorld();
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    void addActor(Actor* newActor);
    Socrates* player() const;
    bool isOverlap(Actor* actor1, Actor* actor2, double radius) const;
    void getOverlap(Actor* actor, list<Actor*>& actorsThatOverlap, double radius);
    void decreasePits();

private:
    // Data Members
    Socrates* m_player;
    list<Actor*> m_actors;
    int m_pits;
    
    // Helper Functions
    void getRandomPoint(double &x, double &y);
};

#endif // STUDENTWORLD_H_
