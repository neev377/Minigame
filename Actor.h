#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

class StudentWorld;

class Actor : public GraphObject {
  public:
    Actor(StudentWorld* studentWorld, int objectType, int imageID, double startX, double startY, Direction dir = 0, int depth = 0, double size = 1.0);
    virtual ~Actor() {}
    virtual void doSomething() = 0;
    StudentWorld* studentWorld() const;
    int objectType() const;
    bool isActive() const;
    void deactivate();
    virtual void takeDamage(int amount);
  private:
    StudentWorld* m_studentWorld;
    bool m_active;
    int m_objectType;
};

class Dirt : public Actor {
  public:
    Dirt(StudentWorld* studentWorld, double startX, double startY);
    void doSomething();
  private:
};

class Food : public Actor {
  public:
    Food(StudentWorld* studentWorld, double startX, double startY);
    void doSomething();
  private:
};

class Pit : public Actor {
  public:
    Pit(StudentWorld* studentWorld, double startX, double startY);
    bool isEmpty() const;
    void doSomething();
  private:
    int m_numberOfRegularSalmonella;
    int m_numberOfAggressiveSalmonella;
    int m_numberOfEColi;
};

class Projectile : public Actor {
  public:
    Projectile(StudentWorld* studentWorld, int objectType, int imageID, double startX, double startY, Direction dir, int maximumTravelDistance, int damage);
    virtual ~Projectile() {}
    void doSomething();
  private:
    int m_distanceTraveled;
    int m_maximumTravelDistance;
    int m_damage;
    
    // Helper Function
    bool damageableObject(int objectType) const;
};

class Spray : public Projectile {
  public:
    Spray(StudentWorld* studentWorld, double startX, double startY, Direction dir);
  private:
};

class Flame : public Projectile {
  public:
    Flame(StudentWorld* studentWorld, double startX, double startY, Direction dir);
  private:
};

class Item : public Actor {
  public:
    Item(StudentWorld* studentWorld, int objectType, int imageID, double startX, double startY, int ScoreChange, bool hasSound);
    virtual ~Item() {}
    void doSomething();
    virtual void playerInteraction() = 0;
  private:
    int m_lifeTime;
    int m_scoreChange;
    bool m_hasSound;
};

class HealthGoodie : public Item {
public:
    HealthGoodie(StudentWorld* studentWorld, double startX, double startY);
    void playerInteraction();
private:
};

class FTGoodie : public Item {
  public:
    FTGoodie(StudentWorld* studentWorld, double startX, double startY);
    void playerInteraction();
  private:
};

class LifeGoodie : public Item {
  public:
    LifeGoodie(StudentWorld* studentWorld, double startX, double startY);
    void playerInteraction();
  private:
};

class Fungus : public Item {
  public:
    Fungus(StudentWorld* studentWorld, double startX, double startY);
    void playerInteraction();
  private:
};

class Agent : public Actor {
  public:
    Agent(StudentWorld* studentWorld, int objectType, int imageID, double startX, double startY, Direction dir, int hitPoints);
    virtual ~Agent() {}
    void takeDamage(int amount);
    void gainHitPoints(int amount);
    int hitPoints() const;
    virtual void playHurtSound() const = 0;
    virtual void playDeadSound() const = 0;
  private:
    int m_hitPoints;
};

class Socrates : public Agent {
  public:
    Socrates(StudentWorld* studentWorld, double startX, double startY);
    void doSomething();
    void increaseFTCharges(int amount);
    void playHurtSound() const;
    void playDeadSound() const;
    int sprays() const;
    int ftCharges() const;
  private:
    int m_sprays;
    int m_FTcharges;
};

class Bacteria : public Agent {
  public:
    Bacteria(StudentWorld* studentWorld, int objectType, int imageID, double startX, double startY, int hitPoints, int damage, bool isAgressiveSalmonella, bool isSalmonella);
    virtual ~Bacteria() {}
    void doSomething();
    bool aggressiveAction();
    virtual void finalAction() = 0;
    double distance(double x1, double y1, double x2, double y2) const;
    void resetMovementPlan();
    int movementPlan();
    void decreaseMovementPlan();
  private:
    int m_movementPlanDistance;
    int m_damage;
    int m_totalFood;
    bool m_isAggressive;
    bool m_isSalmonella;
};

class Salmonella : public Bacteria {
  public:
    Salmonella(StudentWorld* studentWorld, int objectType, double startX, double startY, int hitPoints, int damage, bool isAggressiveSalmonella);
    virtual ~Salmonella() {}
    void finalAction();
    void playHurtSound() const;
    void playDeadSound() const;
  private:
};

class RegularSalmonella : public Salmonella {
  public:
    RegularSalmonella(StudentWorld* studentWorld, double startX, double startY);
  private:
};

class AggressiveSalmonella : public Salmonella {
  public:
    AggressiveSalmonella(StudentWorld* studentWorld, double startX, double startY);
  private:
};

class Ecoli : public Bacteria {
  public:
    Ecoli(StudentWorld* studentWorld, double startX, double startY);
    void finalAction();
    void playHurtSound() const;
    void playDeadSound() const;
  private:
};



#endif // ACTOR_H_
