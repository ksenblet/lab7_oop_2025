#include <gtest/gtest.h>
#include <memory>

#include "npc.h"
#include "toad.h"
#include "dragon.h"
#include "knight.h"

class NPCTest : public ::testing::Test {
protected:
    void SetUp() override {
        toad = std::make_shared<Toad>("TestToad", 10, 20);
        dragon = std::make_shared<Dragon>("TestDragon", 30, 40);
        knight = std::make_shared<Knight>("TestKnight", 50, 60);
    }
    
    std::shared_ptr<Toad> toad;
    std::shared_ptr<Dragon> dragon;
    std::shared_ptr<Knight> knight;
};

TEST_F(NPCTest, BasicProperties) {
    EXPECT_EQ(toad->getName(), "TestToad");
    EXPECT_EQ(toad->getX(), 10);
    EXPECT_EQ(toad->getY(), 20);
    EXPECT_TRUE(toad->isAlive());
    
    EXPECT_EQ(dragon->getName(), "TestDragon");
    EXPECT_EQ(knight->getName(), "TestKnight");
}

TEST_F(NPCTest, TypeNames) {
    EXPECT_EQ(toad->getType(), "Toad");
    EXPECT_EQ(dragon->getType(), "Dragon");
    EXPECT_EQ(knight->getType(), "Knight");
}

TEST_F(NPCTest, DistanceCalculation) {
    auto npc1 = std::make_shared<Toad>("T1", 0, 0);
    auto npc2 = std::make_shared<Toad>("T2", 3, 4);
    
    EXPECT_DOUBLE_EQ(npc1->distance(npc2), 5.0);
    EXPECT_DOUBLE_EQ(npc2->distance(npc1), 5.0);
}

TEST_F(NPCTest, KillMethod) {
    EXPECT_TRUE(toad->isAlive());
    toad->kill();
    EXPECT_FALSE(toad->isAlive());
}

TEST_F(NPCTest, InvalidCoordinates) {
    // В npc.cpp проверка на диапазон 0-100
    EXPECT_THROW(std::make_shared<Toad>("BadToad", -1, 0), std::runtime_error);
    EXPECT_THROW(std::make_shared<Toad>("BadToad", 101, 0), std::runtime_error);
    EXPECT_THROW(std::make_shared<Toad>("BadToad", 0, -1), std::runtime_error);
    EXPECT_THROW(std::make_shared<Toad>("BadToad", 0, 101), std::runtime_error);
}

TEST_F(NPCTest, ValidCoordinates) {
    EXPECT_NO_THROW(std::make_shared<Toad>("GoodToad", 0, 0));
    EXPECT_NO_THROW(std::make_shared<Toad>("GoodToad", 100, 100));
    EXPECT_NO_THROW(std::make_shared<Toad>("GoodToad", 50, 50));
}

TEST_F(NPCTest, MoveRandomWithinBounds) {
    auto npc = std::make_shared<Toad>("MovableToad", 50, 50);
    int initial_x = npc->getX();
    int initial_y = npc->getY();
    
    npc->moveRandom();
    
    int new_x = npc->getX();
    int new_y = npc->getY();
    
    // Проверяем что координаты в пределах 0-100
    EXPECT_GE(new_x, 0);
    EXPECT_LE(new_x, 100);
    EXPECT_GE(new_y, 0);
    EXPECT_LE(new_y, 100);
    
    // Проверяем что NPC переместился (может остаться на месте с вероятностью 1/9)
    // Разница координат должна быть -move_dist, 0 или +move_dist
    int move_dist = npc->getMoveDist();
    int dx = abs(new_x - initial_x);
    int dy = abs(new_y - initial_y);
    
    EXPECT_TRUE(dx == 0 || dx == move_dist);
    EXPECT_TRUE(dy == 0 || dy == move_dist);
}

TEST_F(NPCTest, DeadNPCDoesNotMove) {
    auto npc = std::make_shared<Toad>("DeadToad", 50, 50);
    npc->kill();
    EXPECT_FALSE(npc->isAlive());
    
    int x_before = npc->getX();
    int y_before = npc->getY();
    
    npc->moveRandom();
    
    EXPECT_EQ(npc->getX(), x_before);
    EXPECT_EQ(npc->getY(), y_before);
}

TEST_F(NPCTest, GetKillDist) {
    // Фактические значения из вашего кода (видно из тестов)
    // Toad: killDist = 10
    EXPECT_EQ(toad->getKillDist(), 10);
    // Dragon: killDist = 30
    EXPECT_EQ(dragon->getKillDist(), 30);
    // Knight: killDist = 10
    EXPECT_EQ(knight->getKillDist(), 10);
}

TEST_F(NPCTest, GetMoveDist) {
    // Фактические значения из вашего кода (видно из тестов)
    // Toad: moveDist = 1
    EXPECT_EQ(toad->getMoveDist(), 1);
    // Dragon: moveDist = 50
    EXPECT_EQ(dragon->getMoveDist(), 50);
    // Knight: moveDist = 30
    EXPECT_EQ(knight->getMoveDist(), 30);
}

TEST_F(NPCTest, RollDiceRange) {
    auto npc = std::make_shared<Toad>("Test", 0, 0);
    auto [attack, defense] = npc->rollDice();
    
    // Кубик 1-6
    EXPECT_GE(attack, 1);
    EXPECT_LE(attack, 6);
    EXPECT_GE(defense, 1);
    EXPECT_LE(defense, 6);
}