#include <iostream>

#include "toad.h"
#include "fightVisitor.h"
#include "dragon.h"
#include "knight.h"
#include "observer.h"

Toad::Toad(const std::string& name, int x, int y) : NPC(name, x, y) {}

bool Toad::accept(const std::shared_ptr<FightVisitor>& attacker) {
    return attacker->visit(std::dynamic_pointer_cast<Toad>(shared_from_this()));
}

bool Toad::fight(const std::shared_ptr<Toad>& other) {
    return true;  // Жаба побеждает жабу
}

bool Toad::fight(const std::shared_ptr<Dragon>& other) {
    return true;  // Жаба побеждает дракона
}

bool Toad::fight(const std::shared_ptr<Knight>& other) {
    return true;  // Жаба побеждает рыцаря
}