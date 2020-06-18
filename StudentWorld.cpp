#include "StudentWorld.h"
#include "GameConstants.h"
#include <string>
#include <sstream>
#include "Actor.h"
#include <math.h>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Constructor
StudentWorld::StudentWorld(string assetPath)
    : GameWorld(assetPath), m_pits(0), m_player(nullptr)
{}

// Destructor
StudentWorld::~StudentWorld() {
    cleanUp();
}

// Hepler function that gets a random point within the petri dish
void StudentWorld::getRandomPoint(double &x, double &y) {
    int R = 120;
    double shiftX = VIEW_WIDTH / 2;
    double shiftY = VIEW_HEIGHT / 2;

    double rand1 = ((double) rand () / (RAND_MAX));
    double rand2 = ((double) rand () / (RAND_MAX));
    
    double theta = rand1 * 2 * M_PI;
    
    double r = R * sqrt (rand2);
    
    x = r * cos (theta) + shiftX;
    y = r * sin (theta) + shiftY;
}

// Initiazes the student world at the beginning of each level
int StudentWorld::init()
{
    int L = getLevel();
    
    // Creates a new Socrates player for the current level
    m_player = new Socrates(this, 0, VIEW_HEIGHT/2);
    
    // Initializes initial amount of pits
    m_pits = 0;
    
    double x;
    double y;
    
    // Creates L number of pits for the current level
    for (int i = 0; i < L; i++) {
        Pit* pit = nullptr;
        bool overlap = false;
        // Make sure pits do not overlap with other pits
        do {
            getRandomPoint(x, y);
            pit = new Pit(this, x, y);
            list<Actor*> overlaps;
            overlap = false;
            getOverlap(pit, overlaps, 2*SPRITE_RADIUS);
            for (list<Actor*>::iterator p = overlaps.begin(); p != overlaps.end(); p++) {
                if ((*p)->objectType() == ID_PIT || (*p)->objectType() == ID_FOOD) {
                    overlap = true;
                    delete pit;
                }
            }
        } while (overlap);
            
        if (pit != nullptr) {
            m_actors.push_back(pit);
            m_pits++;
        }
    }
    
    // Creates a random number of food objects for the current level
    int nFood = min(5 * L, 25);
    for (int i = 0; i < nFood; i++) {
        Food* food = nullptr;
        bool overlap = false;
        // Make sure food objects do not overlap with pits or other food
        do {
            getRandomPoint(x, y);
            food = new Food(this, x, y);
            list<Actor*> overlaps;
            overlap = false;
            getOverlap(food, overlaps, 2*SPRITE_RADIUS);
            for (list<Actor*>::iterator p = overlaps.begin(); p != overlaps.end(); p++) {
                if ((*p)->objectType() == ID_PIT || (*p)->objectType() == ID_FOOD) {
                    overlap = true;
                    delete food;
                }
            }
        } while (overlap);
        if (food != nullptr)
            m_actors.push_back(food);
    }

    // Creates a random number of dirt piles for the current level
    int nDirtObjects = max(180 - 20 * L, 20);
    for (int i = 0; i < nDirtObjects; i++) {
        Dirt* dirt = nullptr;
        bool overlap = false;
        // Make sure dirt piles do not overlap with pits or food
        do {
            getRandomPoint(x, y);
            dirt = new Dirt(this, x, y);
            list<Actor*> overlaps;
            overlap = false;
            getOverlap(dirt, overlaps, 2*SPRITE_RADIUS);
            for (list<Actor*>::iterator p = overlaps.begin(); p != overlaps.end(); p++) {
                if ((*p)->objectType() == ID_PIT || (*p)->objectType() == ID_FOOD) {
                    overlap = true;
                    delete dirt;
                }
            }
        } while (overlap);
        if (dirt != nullptr)
            m_actors.push_back(dirt);
    }
    
    return GWSTATUS_CONTINUE_GAME;
}

// Lets each active actor in the current tik of the game do something
int StudentWorld::move()
{
    int L = getLevel();
    
    // Allow player to do something, according to user input
    m_player->doSomething();
 
    // Loop through actors in the game and allow them to do something if they are active
    for (list<Actor*>::iterator p = m_actors.begin(); p != m_actors.end(); p++) {
        if ((*p)->isActive())
            (*p)->doSomething();
        if (!(m_player->isActive())) {
            decLives();
            return GWSTATUS_PLAYER_DIED;
        }
        // Checks how many bacteria are left in the game
        bool zeroBacteria = true;
        for (list<Actor*>::iterator p = m_actors.begin(); p != m_actors.end(); p++) {
            int type = (*p)->objectType();
            if (type == ID_REGULAR_SALMONELLA || type == ID_AGGRESSIVE_SALMONELLA || type == ID_ECOLI)
                zeroBacteria = false;
        }
        // If there are no more bacteria or pits, the level is finished
        if (zeroBacteria && m_pits == 0)
            return GWSTATUS_FINISHED_LEVEL;
    }
    
    // The game must get rid of all actors that are not active
    for (list<Actor*>::iterator p = m_actors.begin(); p != m_actors.end(); p++) {
        if (!((*p)->isActive())) {
            delete *p;
            m_actors.erase(p);
        }
    }
    
    // Potentially introduce a new fungus object into the current level
    int chanceFungus = max(510 - L * 10, 200);
    int fungusActivation = randInt(0, chanceFungus);
    if (fungusActivation == 0) {
        double angle = randInt(1, 360) * M_PI / 180;
        double x = cos(angle) * VIEW_RADIUS + VIEW_WIDTH/2;
        double y = sin(angle) * VIEW_RADIUS + VIEW_HEIGHT/2;
        m_actors.push_back(new Fungus(this, x, y));
    }
    
    // Potentially introduce a new goodie object into the current level
    int chanceGoodie = max(510 - L * 10, 250);
    int goodieActivation = randInt(0, chanceGoodie);
    if (goodieActivation == 0) {
        int angle = randInt(1, 360) * M_PI / 180;
        double x = cos(angle) * VIEW_RADIUS + VIEW_WIDTH/2;
        double y = sin(angle) * VIEW_RADIUS + VIEW_HEIGHT/2;
        
        // Randomize which type of goodie will be added
        int whichGoodie = randInt(1, 10);
        if (whichGoodie == 1)
            m_actors.push_back(new LifeGoodie(this, x, y));
        else if (whichGoodie <= 4)
            m_actors.push_back(new FTGoodie(this, x, y));
        else
            m_actors.push_back(new HealthGoodie(this, x, y));
    }
    
    // Update the game text that will be presented to the user at the top of the screen
    ostringstream text;
    text.fill('0');
    text << "Score: " << setw(6) << getScore() << "  Level: " << L << "  Lives: " << getLives() << "  Health: " << setw(1) << player()->hitPoints() << "  Sprays: " << setw(1) << m_player->sprays() << "  Flames: " << setw(1) << m_player->ftCharges();
    string gameText = text.str();
    setGameStatText(gameText);
        
    return GWSTATUS_CONTINUE_GAME;
}

// Called at the end of each completed level, so that the next level can build off scratch
void StudentWorld::cleanUp()
{
    // Delete the player
    if (m_player != nullptr)
        delete m_player;
    m_player = nullptr;
    // Delete all other actors
    for (list<Actor*>::iterator p = m_actors.begin(); p != m_actors.end(); p++) {
        if (*p != nullptr)
            delete *p;
        m_actors.erase(p);
    }
}

// Introduce a new actor into the level
void StudentWorld::addActor(Actor* newActor) {
    m_actors.push_back(newActor);
}

// Decrease recorded number of pits by one
void StudentWorld::decreasePits() {
    m_pits--;
}

// Check to see if two given actors overlap within a certain radius
bool StudentWorld::isOverlap(Actor* actor1, Actor* actor2, double radius) const {
    
    double x1 = actor1->getX();
    double y1 = actor1->getY();
    double x2 = actor2->getX();
    double y2 = actor2->getY();
    
    double dx = x2 - x1;
    double dy = y2 - y1;
    
    double distance = sqrt(dx*dx + dy*dy);

    return (distance <= radius);
}

// Create a list of all actors in the game that overlap with a given actor within a certain radius
void StudentWorld::getOverlap(Actor* actor, list<Actor*>& actorsThatOverlap, double radius) {
    for (list<Actor*>::iterator p = m_actors.begin(); p != m_actors.end(); p++) {
        if (isOverlap(actor, *p, radius) && *p != actor)
            actorsThatOverlap.push_back(*p);
    }
    if (actor->objectType() != ID_SOCRATES) {
        if (isOverlap(actor, m_player, radius))
            actorsThatOverlap.push_back(m_player);
    }
}

// Return the player
Socrates* StudentWorld::player() const {
    return m_player;
}
