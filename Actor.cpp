#include "Actor.h"
#include "StudentWorld.h"

/*---------------------*/
/*--------Actor--------*/
/*---------------------*/

// Constructor
Actor::Actor(StudentWorld* studentWorld, int objectType, int imageID, double startX, double startY, Direction dir, int depth, double size)
    : m_studentWorld(studentWorld), m_objectType(objectType), GraphObject(imageID, startX, startY, dir, depth, size), m_active(true)
{}

// Return the student world the actor lives in
StudentWorld* Actor::studentWorld() const {
    return m_studentWorld;
}

// Return the actor's most specific type
int Actor::objectType() const {
    return m_objectType;
}

// Return whether or not the actor is active
bool Actor::isActive() const {
    return m_active;
}

// Deactivate the actor
void Actor::deactivate() {
    m_active = false;
}

// The actor takes damage and is deactivated
void Actor::takeDamage(int amount) {
    deactivate();
}

/*---------------------*/
/*--------Dirt---------*/
/*---------------------*/

// Constructor
Dirt::Dirt(StudentWorld* studentWorld, double startX, double startY)
    : Actor(studentWorld, ID_DIRT, IID_DIRT, startX, startY, 0, 1)
{}

// Dirt does nothing
void Dirt::doSomething() {
    return;
}

/*---------------------*/
/*--------Food---------*/
/*---------------------*/

// Constructor
Food::Food (StudentWorld* studentWorld, double startX, double startY)
    : Actor(studentWorld, ID_FOOD, IID_FOOD, startX, startY, 90, 1)
{}

// Food does nothing on its own
void Food::doSomething() {
    return;
}

/*---------------------*/
/*---------Pit---------*/
/*---------------------*/

Pit::Pit(StudentWorld* studentWorld, double startX, double startY)
    : Actor(studentWorld, ID_PIT, IID_PIT, startX, startY, 0, 1), m_numberOfRegularSalmonella(5), m_numberOfAggressiveSalmonella(3), m_numberOfEColi(2)
{}

bool Pit::isEmpty() const {
    if (m_numberOfRegularSalmonella == 0 && m_numberOfAggressiveSalmonella == 0 && m_numberOfEColi == 0)
        return true;
    else
        return false;
}

void Pit::doSomething() {
    
    if (isEmpty()) {
        studentWorld()->decreasePits();
        deactivate();
        return;
    }
    
    if (randInt(1, 50) == 1) {
        int RegSal = 0;
        int AggSal = 0;
        int eColi = 0;
        
        int count = 0;
        if (m_numberOfRegularSalmonella > 0) {
            count++;
            RegSal = count;
        }
        if (m_numberOfAggressiveSalmonella > 0) {
            count++;
            AggSal = count;
        }
        if (m_numberOfEColi > 0) {
            count++;
            eColi = count;
        }
        
        int bacteria = randInt(1, count);
        
        Bacteria* newBacteria = nullptr;
        
        if (bacteria == RegSal) {
            newBacteria = new RegularSalmonella(studentWorld(), getX(), getY());
            m_numberOfRegularSalmonella--;
        }
        else if (bacteria == AggSal) {
            newBacteria = new AggressiveSalmonella(studentWorld(), getX(), getY());
            m_numberOfAggressiveSalmonella--;
        }
        else if (bacteria == eColi) {
            newBacteria = new Ecoli(studentWorld(), getX(), getY());
            m_numberOfEColi--;
        }
        
        if (newBacteria != nullptr) {
            studentWorld()->addActor(newBacteria);
            studentWorld()->playSound(SOUND_BACTERIUM_BORN);
        }
    }
}

/*---------------------*/
/*-----Projectile------*/
/*---------------------*/

// Constructor
Projectile::Projectile(StudentWorld* studentWorld, int objectType, int imageID, double startX, double startY, Direction dir, int maximumTravelDistance, int damage)
    : Actor(studentWorld, objectType, imageID, startX, startY, dir, 1), m_distanceTraveled(0), m_maximumTravelDistance(maximumTravelDistance), m_damage(damage)
{}

// Projectile does something during every tick
void Projectile::doSomething() {
    // Do nothing if it is not active
    if (!isActive())
        return;
    
    // Check to see if the projectile overlaps with any damageable object and apply damage if necessary
    list<Actor*> overlaps;
    studentWorld()->getOverlap(this, overlaps, SPRITE_WIDTH);
    for (list<Actor*>::iterator p = overlaps.begin(); p != overlaps.end(); p++) {
        if (damageableObject((*p)->objectType())) {
            (*p)->takeDamage(m_damage);
            deactivate();
            return;
        }
    }
    
    // The projectile continues to move forward
    moveForward(SPRITE_WIDTH);
    m_distanceTraveled += SPRITE_WIDTH;

    // If projectile has reached max distance, it is deactivated
    if (m_distanceTraveled >= m_maximumTravelDistance)
        deactivate();
}

// Checks whether or not the projectile damages a particular object type
bool Projectile::damageableObject(int objectType) const {
    bool toReturn = false;
    switch (objectType) {
        case ID_REGULAR_SALMONELLA:
        case ID_AGGRESSIVE_SALMONELLA:
        case ID_ECOLI:
        case ID_FUNGI:
        case ID_HEALTH_GOODIE:
        case ID_FLAME_GOODIE:
        case ID_LIFE_GOODIE:
        case ID_DIRT:
            toReturn = true;
            break;
        default:
            break;
    }
    return toReturn;
}

/*---------------------*/
/*--------Spray--------*/
/*---------------------*/

// Constructor
Spray::Spray(StudentWorld* studentWorld, double startX, double startY, Direction dir)
    : Projectile(studentWorld, ID_SPRAY, IID_SPRAY, startX, startY, dir, 112, 2)
{}

/*---------------------*/
/*--------Flame--------*/
/*---------------------*/

// Constructor
Flame::Flame(StudentWorld* studentWorld, double startX, double startY, Direction dir)
    : Projectile(studentWorld, ID_FLAME, IID_FLAME, startX, startY, dir, 32, 5)
{}

/*---------------------*/
/*---------Item--------*/
/*---------------------*/

// Constructor
Item::Item(StudentWorld* studentWorld, int objectType, int imageID, double startX, double startY, int scoreChange, bool hasSound)
    : Actor(studentWorld, objectType, imageID, startX, startY, 0, 1), m_lifeTime(max(randInt(0, 300 - 10 * (studentWorld->getLevel()) - 1), 50)), m_hasSound(hasSound), m_scoreChange(scoreChange)
{}

// Item does something during every tick
void Item::doSomething() {
    // Do nothing if it is not active
    if (!isActive())
        return;
    
    // Check to see if the item overlaps with the player
    // If necessary, item will update the game score, perform its unique player interaction, and play a sound indicating a goodie was obtained
    if(studentWorld()->isOverlap(this, studentWorld()->player(), SPRITE_WIDTH)) {
        studentWorld()->increaseScore(m_scoreChange);
        if (m_hasSound)
            studentWorld()->playSound(SOUND_GOT_GOODIE);
        playerInteraction();
        deactivate();
        return;
    }
    
    // Decrease lifetime of the item
    m_lifeTime--;
    
    // If liftime is depleted, the item is deactivated
    if (m_lifeTime <= 0)
        deactivate();
}

/*---------------------*/
/*-----HealthGoodie----*/
/*---------------------*/

// Constructor
HealthGoodie::HealthGoodie(StudentWorld* studentWorld, double startX, double startY)
    : Item(studentWorld, ID_HEALTH_GOODIE, IID_RESTORE_HEALTH_GOODIE, startX, startY, 250, true)
{}

// Health goodie increases player's hit points
void HealthGoodie::playerInteraction() {
    studentWorld()->player()->gainHitPoints(100);
}

/*---------------------*/
/*-------FTGoodie------*/
/*---------------------*/

// Constructor
FTGoodie::FTGoodie(StudentWorld* studentWorld, double startX, double startY)
    : Item(studentWorld, ID_FLAME_GOODIE, IID_FLAME_THROWER_GOODIE, startX, startY, 300, true)
{}

// Flame goodies reset player's flame charges to 5
void FTGoodie::playerInteraction() {
    studentWorld()->player()->increaseFTCharges(5);
}

/*---------------------*/
/*------LifeGoodie-----*/
/*---------------------*/

// Constructor
LifeGoodie::LifeGoodie(StudentWorld* studentWorld, double startX, double startY)
    : Item(studentWorld, ID_LIFE_GOODIE, IID_EXTRA_LIFE_GOODIE, startX, startY, 500, true)
{}

// Life goodies give the player an extra life
void LifeGoodie::playerInteraction() {
    studentWorld()->incLives();
}

/*---------------------*/
/*-------Fungus--------*/
/*---------------------*/

// Constructor
Fungus::Fungus(StudentWorld* studentWorld, double startX, double startY)
    : Item(studentWorld, ID_FUNGI, IID_FUNGUS, startX, startY, -50, false)
{}

// Fungus damages the player by 20 hit points
void Fungus::playerInteraction() {
    studentWorld()->player()->takeDamage(20);
}

/*---------------------*/
/*--------Agent--------*/
/*---------------------*/

// Constructor
Agent::Agent(StudentWorld* studentWorld, int objectType, int imageID, double startX, double startY, Direction dir, int hitPoints)
    : Actor(studentWorld, objectType, imageID, startX, startY, dir), m_hitPoints(hitPoints)
{}

// Returns number of hit points the agent has
int Agent::hitPoints() const {
    return m_hitPoints;
}

// Increase the agent's number of hit points
void Agent::gainHitPoints(int amount) {
    if (m_hitPoints + amount > 100)
        m_hitPoints = 100;
    else
        m_hitPoints += amount;
}

// Damages the agent by a given amount of hitpoints
void Agent::takeDamage(int amount) {
    m_hitPoints -= amount;
    
    // Checks to see if the agent is now dead
    if (m_hitPoints <= 0) {
        m_hitPoints = 0;
        deactivate();
        playDeadSound();
        return;
    }
    playHurtSound();
}

/*---------------------*/
/*------Socrates-------*/
/*---------------------*/

// Constructor
Socrates::Socrates(StudentWorld* studentWorld, double startX, double startY)
    : m_sprays(20), m_FTcharges(5), Agent(studentWorld, ID_SOCRATES, IID_PLAYER, 0, VIEW_HEIGHT/2, 0, 100)
{}

// Socrates does something during every tick
void Socrates::doSomething() {
    // If Socrates is not active, do nothing
    if (!isActive())
        return;
    
    // Perform some action given by user input
    int ch;
    if (studentWorld()->getKey(ch))
    {
        // Obtain player's current positional angle
        double angle = 0;
        double x = getX();
        double y = getY();
        
        if (x == 0)
            angle = M_PI;
        else if (x == VIEW_WIDTH)
            angle = 0;
        else if (y == 0)
            angle = 3 * M_PI / 2;
        else if (y == VIEW_HEIGHT)
            angle = M_PI / 2;
        else if (y > VIEW_HEIGHT/2)
            angle = acos( (x - (VIEW_WIDTH/2)) / VIEW_RADIUS);
        else if (y < VIEW_HEIGHT/2)
            angle = -1 * acos( (x - (VIEW_WIDTH/2)) / VIEW_RADIUS);
        
        double newX;
        double newY;
        switch (ch)
        {
            // Move counterclockwise if user input is left key
            case KEY_PRESS_LEFT:
                newX = cos(angle + 5 * M_PI / 180) * VIEW_RADIUS + VIEW_HEIGHT/2;
                newY = sin(angle + 5 * M_PI / 180) * VIEW_RADIUS + VIEW_WIDTH/2;
                moveTo(newX, newY);
                setDirection((angle * 180) / M_PI + 185);
                break;
                
            // Move clockwise if user input is right key
            case KEY_PRESS_RIGHT:
                newX = cos(angle - 5 * M_PI / 180) * VIEW_RADIUS + VIEW_HEIGHT/2;
                newY = sin(angle - 5 * M_PI / 180) * VIEW_RADIUS + VIEW_WIDTH/2;
                moveTo(newX, newY);
                setDirection((angle * 180) / M_PI + 175);
                break;
                
            // Try to fire a spray projetile if user input is space key
            case KEY_PRESS_SPACE:
                if (m_sprays >= 1) {
                    // Spray projectile is fired in current direction the player is facing
                    getPositionInThisDirection(getDirection(), 2*SPRITE_RADIUS, newX, newY);
                    Spray* newSpray = new Spray(studentWorld(), newX, newY, getDirection());
                    studentWorld()->addActor(newSpray);
                    m_sprays--;
                    studentWorld()->playSound(SOUND_PLAYER_SPRAY);
                }
                break;
                
            // Try to fire flame projectiles if user input is enter key
            case KEY_PRESS_ENTER:
                if (m_FTcharges >= 1) {
                    // Flame projectiles are fired outward from player in all directions
                    for (int i = 0; i < 16; i++) {
                        getPositionInThisDirection(getDirection() + (i * 22), 2*SPRITE_RADIUS, newX, newY);
                        Flame* newFlame = new Flame(studentWorld(), newX, newY, getDirection() + (i * 22));
                        studentWorld()->addActor(newFlame);
                    }
                    m_FTcharges--;
                    studentWorld()->playSound(SOUND_PLAYER_FIRE);
                }
                break;
            default:
                break;
        }
    }
    
    // If there was no user input, attempt to increase sprays count
    else if (m_sprays < 20)
        m_sprays++;
}

// Play sound when hurt
void Socrates::playHurtSound() const {
    studentWorld()->playSound(SOUND_PLAYER_HURT);
}

// Play sound when killed
void Socrates::playDeadSound() const {
    studentWorld()->playSound(SOUND_PLAYER_DIE);
}

// Return sprays count
int Socrates::sprays() const {
    return m_sprays;
}

// Return flame charges count
int Socrates::ftCharges() const {
    return m_FTcharges;
}

// Increase the flame charges
void Socrates::increaseFTCharges(int amount) {
    m_FTcharges += amount;
}

/*---------------------*/
/*------Bacteria-------*/
/*---------------------*/

// Constructor
Bacteria::Bacteria(StudentWorld* studentWorld, int objectType, int imageID, double startX, double startY, int hitPoints, int damage, bool isAgressiveSalmonella, bool isSalmonella)
    : Agent(studentWorld, objectType, imageID, startX, startY, 90, hitPoints), m_movementPlanDistance(0), m_damage(damage), m_totalFood(0), m_isAggressive(isAgressiveSalmonella), m_isSalmonella(isSalmonella)
{}

// Calculate the distance between two points
double Bacteria::distance(double x1, double y1, double x2, double y2) const {
    double dx = x2 - x1;
    double dy = y2 - y1;
    return sqrt(dx*dx + dy*dy);
}

// Reset movement plan to 10
void Bacteria::resetMovementPlan() {
    m_movementPlanDistance = 10;
}

// Return current movement plan
int Bacteria::movementPlan() {
    return m_movementPlanDistance;
}

// Decrease movement plan
void Bacteria::decreaseMovementPlan() {
    m_movementPlanDistance--;
}

// An action performed by aggressive bacteria, returns whether or not the action occured
bool Bacteria::aggressiveAction() {
    
    // Obtain current position of the player and the relative distance to the player
    double playerX = studentWorld()->player()->getX();
    double playerY = studentWorld()->player()->getY();
    double distanceToSocrates = distance(getX(), getY(), playerX, playerY);
    
    // Check if the distance to the player is less than or equal to 72 units
    if (distanceToSocrates <= 72) {
        
        // Obtain angle/direction from bacteria to the player
        double angle = 0;
        double dx = playerX - getX();
        double dy = playerY - getY();
        
        if (dy == 0 && dx > 1)
            angle = 0;
        else if (dy == 0 && dx < 1)
            angle = 180;
        else if (dx == 0 && dy > 0)
            angle = 90;
        else if (dx == 0 && dy < 0)
            angle = 270;
        else if (dy > 0)
            angle = atan(dy/dx) * 180 / M_PI;
        else if (dy < 0)
            angle = ( atan(dy/dx) * 180 / M_PI ) + 180;
        
        double x = 0;
        double y = 0;
        
        bool freeMovement = true;
        // Check for clear path of 3 units in direction of the player
        for (int i = 1; i <= 3; i++) {
            getPositionInThisDirection(angle, i, x, y);
            Actor* tempSalmonella = new AggressiveSalmonella(studentWorld(), x, y);
            
            // Check to make sure bacteria would not overlap with dirt pile
            list<Actor*> overlapsInDirection;
            studentWorld()->getOverlap(tempSalmonella, overlapsInDirection, SPRITE_WIDTH/2);
            for (list<Actor*>::iterator p = overlapsInDirection.begin(); p != overlapsInDirection.end(); p++) {
                if ((*p)->objectType() == ID_DIRT)
                    freeMovement = false;
            }
            
            // Check to make sure bacteria would not exit the petri dish
            if (distance(x, y, VIEW_WIDTH/2, VIEW_HEIGHT/2) >= VIEW_RADIUS)
                freeMovement = false;
            
            delete tempSalmonella;
            
            if (!freeMovement)
                break;
        }
        
        // If path is clear, make the movement towards the player
        if (freeMovement) {
            moveTo(x, y);
            setDirection(angle);
        }
        return true;
    }
    return false;
}

// Bacteria does something during every tick
void Bacteria::doSomething() {
    // Bacteria does nothing if it is not active
    if (!isActive())
        return;
    
    // Performs aggressive action if the bacteria is aggressive
    bool returnEarly = false;
    if (m_isAggressive)
        returnEarly = aggressiveAction();
    
    // Check to see if the bacteria is currently overlapping with any food objects
    bool overlapsFood = false;
    Actor* food = nullptr;
    list<Actor*> overlaps;
    studentWorld()->getOverlap(this, overlaps, SPRITE_WIDTH);
    for (list<Actor*>::iterator p = overlaps.begin(); p != overlaps.end(); p++) {
        if ((*p)->objectType() == ID_FOOD) {
            overlapsFood = true;
            food = (*p);
        }
    }
    
    // Check to see if the bacteria is currently overlapping with the player and damage the player if necessary
    if (studentWorld()->isOverlap(this, studentWorld()->player(), SPRITE_WIDTH)) {
        studentWorld()->player()->takeDamage(m_damage);
    }
    // Check the bacteria's current food count
    else if (m_totalFood >= 3) {
        
        // If the found count is at least three, spawn a new bacteria object of the same type
        double newX = getX();
        double newY = getY();
        
        if (newX < VIEW_WIDTH/2)
            newX += SPRITE_WIDTH/2;
        else if (newX > VIEW_WIDTH/2)
            newX -= SPRITE_WIDTH/2;
        
        if (newY < VIEW_HEIGHT/2)
            newY += SPRITE_WIDTH/2;
        else if (newY > VIEW_HEIGHT/2)
            newY -= SPRITE_WIDTH/2;
        
        if (m_isAggressive)
            studentWorld()->addActor(new AggressiveSalmonella(studentWorld(), newX, newY));
        else if (m_isSalmonella)
            studentWorld()->addActor(new RegularSalmonella(studentWorld(), newX, newY));
        else
            studentWorld()->addActor(new Ecoli(studentWorld(), newX, newY));
        
        // Reset food count
        m_totalFood = 0;
    }
    // Check if the bacteria is overlapping with any food objects
    else if(overlapsFood) {
        // Increase food count and get rid of a single food object
        m_totalFood++;
        food->deactivate();
    }
    
    // If the earlier aggressive action was successful, return now
    if (returnEarly)
        return;
    
    // Perform the bacteria's final action
    finalAction();
}

/*---------------------*/
/*-----Salmonella------*/
/*---------------------*/

// Constructor
Salmonella::Salmonella(StudentWorld* studentWorld, int objectType, double startX, double startY, int hitPoints, int damage, bool isAggressiveSalmonella)
    : Bacteria(studentWorld, objectType, IID_SALMONELLA, startX, startY, hitPoints, damage, isAggressiveSalmonella, true)
{}

// Play sound when salmonella is hurt
void Salmonella::playHurtSound() const {
    studentWorld()->playSound(SOUND_SALMONELLA_HURT);
}

// Play sound when salmonella dies
void Salmonella::playDeadSound() const {
    studentWorld()->playSound(SOUND_SALMONELLA_DIE);
    studentWorld()->increaseScore(100);
}

// The final action performed by salmonella during its call to do something
void Salmonella::finalAction() {
    // Check to see if the current movement plan is positive
    if (movementPlan() > 0) {
        
        decreaseMovementPlan();
        
        bool movementFree = true;
        double x = 0;
        double y = 0;
        
        // Check if path of 3 units in current direction is valid
        for (int i = 1; i <= 3; i++) {
            getPositionInThisDirection(getDirection(), i, x, y);
            Actor* tempSalmonella = new RegularSalmonella(studentWorld(), x, y);
            
            // Check to see if the path would overlap with any dirt piles
            list<Actor*> overlapsMovement;
            studentWorld()->getOverlap(tempSalmonella, overlapsMovement, SPRITE_WIDTH/2);
            for (list<Actor*>::iterator p = overlapsMovement.begin(); p != overlapsMovement.end(); p++) {
                if ((*p)->objectType() == ID_DIRT) {
                    movementFree = false;
                    break;
                }
            }

            // Check to se if the path would take the salmonella outside the petri dish
            double distanceFromCenter = distance(x, y, VIEW_WIDTH/2, VIEW_HEIGHT/2);
            if (distanceFromCenter >= VIEW_RADIUS)
                movementFree = false;
            
            delete tempSalmonella;
            
            if (!movementFree)
                break;
        }
        
        // Move 3 units in current direction if path is valid
        getPositionInThisDirection(getDirection(), 3, x, y);
        if (movementFree) {
            moveTo(x, y);
        }
        // Otherwise, randomize the salmonella's direction
        else {
            setDirection(randInt(0, 359));
            resetMovementPlan();
        }
        return;
    }
    
    // Find any nearby food objects
    Actor* closestFood = nullptr;
    double smallestDistance = VIEW_RADIUS*2;
    list<Actor*> allActors;
    studentWorld()->getOverlap(this, allActors, VIEW_RADIUS*2);
    for (list<Actor*>::iterator p = allActors.begin(); p != allActors.end(); p++) {
        double dis = distance(getX(), getY(), (*p)->getX(), (*p)->getY());
        if ((*p)->objectType() == ID_FOOD && dis < 128 && dis < smallestDistance) {
            closestFood = (*p);
            smallestDistance = dis;
        }
    }
    
    // If there is no nearby food object found, randomize the salmonella's direction
    if (closestFood == nullptr) {
        setDirection(randInt(0, 359));
        resetMovementPlan();
        return;
    }
    
    // If a nearby food object was found, try to move towards it
    // Obtain angle/direction from salmonella to food object of interest
    double foodX = closestFood->getX();
    double foodY = closestFood->getY();
    
    double angle = 0;
    double dx = foodX - getX();
    double dy = foodY - getY();

    if (dy == 0 && dx > 1)
        angle = 0;
    else if (dy == 0 && dx < 1)
        angle = 180;
    else if (dx == 0 && dy > 0)
        angle = 90;
    else if (dx == 0 && dy < 0)
        angle = 270;
    else if (dy > 0)
        angle = atan(dy/dx) * 180 / M_PI;
    else if (dy < 0)
        angle = ( atan(dy/dx) * 180 / M_PI ) + 180;
    
    // Make sure that path of 3 units in direction of food is valid
    double x;
    double y;
    bool freeMovement = true;
    for (int i = 1; i <= 3; i++) {
        getPositionInThisDirection(angle, i, x, y);
        Actor* tempSalmonella = new RegularSalmonella(studentWorld(), x, y);
        
        // Check to see if the path would overlap with any dirt piles
        list<Actor*> overlapsInDirection;
        studentWorld()->getOverlap(tempSalmonella, overlapsInDirection, SPRITE_WIDTH/2);
        for (list<Actor*>::iterator p = overlapsInDirection.begin(); p != overlapsInDirection.end(); p++) {
            if ((*p)->objectType() == ID_DIRT)
                freeMovement = false;
        }
        
        // Check to see if the path would make the salmonella exit the petri dish
        double distanceFromCenter = distance(x, y, VIEW_WIDTH/2, VIEW_HEIGHT/2);
        if (distanceFromCenter >= VIEW_RADIUS)
            freeMovement = false;
        
        delete tempSalmonella;
        
        if (!freeMovement)
            break;
    }
    
    // If the path is valid, make the movement
    if (freeMovement) {
        moveAngle(angle, 3);
        setDirection(angle);
    }
    // If the path is not valid, randomize the salmonella's direction
    else {
        setDirection(randInt(0, 359));
        resetMovementPlan();
    }
}

/*---------------------*/
/*--RegularSalmonella--*/
/*---------------------*/

// Constructor
RegularSalmonella::RegularSalmonella(StudentWorld* studentWorld, double startX, double startY)
    : Salmonella(studentWorld, ID_REGULAR_SALMONELLA, startX, startY, 4, 1, false)
{}

/*---------------------*/
/*-AggressiveSalmonella*/
/*---------------------*/

// Constructor
AggressiveSalmonella::AggressiveSalmonella(StudentWorld* studentWorld, double startX, double startY)
    : Salmonella(studentWorld, ID_AGGRESSIVE_SALMONELLA, startX, startY, 10, 2, true)
{}

/*---------------------*/
/*--------Ecoli--------*/
/*---------------------*/

// Constructor
Ecoli::Ecoli(StudentWorld* studentWorld, double startX, double startY)
    : Bacteria(studentWorld, ID_ECOLI, IID_ECOLI, startX, startY, 5, 4, false, false)
{}

// Ecoli plays sound when hurt
void Ecoli::playHurtSound() const {
    studentWorld()->playSound(SOUND_ECOLI_HURT);
}

// Ecoli plays sound when it dies
void Ecoli::playDeadSound() const {
    studentWorld()->playSound(SOUND_ECOLI_DIE);
    studentWorld()->increaseScore(100);
}

// Final action performed by Ecoli whenever it is called to do something
void Ecoli::finalAction() {
    
    // Obtain distance from Ecoli to player's current position
    double playerX = studentWorld()->player()->getX();
    double playerY = studentWorld()->player()->getY();
    double distanceToSocrates = distance(getX(), getY(), playerX, playerY);
    
    // If the distance is less than 256 units, attempt to move towards the player
    if (distanceToSocrates <= 256) {
        
        // Obtain angle/direction from Ecoli to player
        double angle = 0;
        double dx = playerX - getX();
        double dy = playerY - getY();
        double hyp = distance(playerX, playerY, getX(), getY());
        
        if (dy == 0 && dx > 1)
            angle = 0;
        else if (dy == 0 && dx < 1)
            angle = 180;
        else if (dx == 0 && dy > 0)
            angle = 90;
        else if (dx == 0 && dy < 0)
            angle = 270;
        else if (dy > 0)
            angle = acos( dx / hyp);
        else if (dy < 0)
            angle = -1 * acos( dx / hyp);
        
        // Attempt to move towards the player in the found angle or increments of it
        for (int i = 0; i < 10; i++) {
            
            double x = 0;
            double y = 0;
            
            // Check to see if path of 2 units in current direction is valid
            bool freeMovement = true;
            for (int j = 1; j <= 2; j++) {
                getPositionInThisDirection((angle + i * 10) * 180 / M_PI, j, x, y);
                Actor* tempEcoli = new Ecoli(studentWorld(), x, y);
                
                // Check if the path overlaps with any dirt piles
                list<Actor*> overlapsInDirection;
                studentWorld()->getOverlap(tempEcoli, overlapsInDirection, SPRITE_WIDTH/2);
                for (list<Actor*>::iterator p = overlapsInDirection.begin(); p != overlapsInDirection.end(); p++) {
                    if ((*p)->objectType() == ID_DIRT)
                        freeMovement = false;
                }
                
                // Check if the path would cause the Ecoli to exit the petri dish
                double distanceFromCenter = distance(x, y, VIEW_WIDTH/2, VIEW_HEIGHT/2);
                if (distanceFromCenter >= VIEW_RADIUS)
                    freeMovement = false;
                
                delete tempEcoli;
                
                if (!freeMovement)
                    break;
            }
            
            // If path is valid, make the movement
            if (freeMovement) {
                moveTo(x, y);
                return;
            }
        }
    }
}
