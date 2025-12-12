#include <iostream>

#include "knight.h"
#include "fightVisitor.h"
#include "toad.h"   
#include "dragon.h"
#include "observer.h"

Knight::Knight(const std::string& name, int x, int y) : NPC(name, x, y) {}

bool Knight::accept(const std::shared_ptr<FightVisitor>& attacker) {
    return attacker->visit(std::dynamic_pointer_cast<Knight>(shared_from_this()));
}

bool Knight::fight(const std::shared_ptr<Toad>& other) {
    return false;  // Рыцарь проигрывает жабе
}

bool Knight::fight(const std::shared_ptr<Dragon>& other) {
    return true;  // Рыцарь побеждает дракона
}

bool Knight::fight(const std::shared_ptr<Knight>& other) {
    return false;  // Ничья
}