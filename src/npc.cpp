#include <cmath>
#include <cstdlib>
#include <ctime>

#include "npc.h"

NPC::NPC(const std::string& name, int x, int y) 
    : name(name), x(x), y(y), alive(true) {
    if (x < 0 || x > 100 || y < 0 || y > 100) {
        throw std::runtime_error("NPC coordinates must be in range 0-100");
    }
}

void NPC::moveRandom() {
    if (!alive) return;  
    
    int dx = (rand() % 3) - 1;  // -1, 0, или 1
    int dy = (rand() % 3) - 1;  // -1, 0, или 1
    
    int move_dist = getMoveDist();
    if (move_dist > 0) {
        dx *= move_dist;
        dy *= move_dist;
    }

    int newX = x + dx;
    int newY = y + dy;

    if (newX >= 0 && newX <= 100 && newY >= 0 && newY <= 100) {
        x = newX;
        y = newY;
    }
}

std::pair<int, int> NPC::rollDice() const {
    return {std::rand() % 6 + 1, std::rand() % 6 + 1};
}

double NPC::distance(const std::shared_ptr<NPC>& other) const {
    if (!other) return 0;
    int dx = x - other->x;
    int dy = y - other->y;
    return std::sqrt(dx * dx + dy * dy);
}