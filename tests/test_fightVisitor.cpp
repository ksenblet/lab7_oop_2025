#include <gtest/gtest.h>
#include <memory>

#include "toad.h"
#include "dragon.h"
#include "knight.h"
#include "fightVisitor.h"
#include "observer.h"

class MockObserver : public IFFightObserver {
public:
    int call_count = 0;
    std::shared_ptr<NPC> last_attacker;
    std::shared_ptr<NPC> last_defender;
    bool last_result = false;
    
    void onFight(const std::shared_ptr<NPC>& attacker, 
                 const std::shared_ptr<NPC>& defender, 
                 bool result) override {
        call_count++;
        last_attacker = attacker;
        last_defender = defender;
        last_result = result;
    }
    
    void reset() {
        call_count = 0;
        last_attacker.reset();
        last_defender.reset();
        last_result = false;
    }
};

class FightVisitorTest : public ::testing::Test {
protected:
    void SetUp() override {
        toad = std::make_shared<Toad>("Toad", 0, 0);
        dragon = std::make_shared<Dragon>("Dragon", 1, 1);
        knight = std::make_shared<Knight>("Knight", 2, 2);
        mock_observer = std::make_shared<MockObserver>();
    }
    
    std::shared_ptr<Toad> toad;
    std::shared_ptr<Dragon> dragon;
    std::shared_ptr<Knight> knight;
    std::shared_ptr<MockObserver> mock_observer;
};

// Тест 1: Жаба побеждает всех (всегда возвращает true)
TEST_F(FightVisitorTest, ToadAlwaysWins) {
    mock_observer->reset();
    auto visitor = std::make_shared<FightVisitor>(toad, mock_observer);
    
    // Жаба vs Дракон - жаба побеждает
    bool result = visitor->visit(dragon);
    EXPECT_TRUE(result);  // true = победа
    EXPECT_EQ(mock_observer->call_count, 1);
    EXPECT_TRUE(mock_observer->last_result);
    
    mock_observer->reset();
    // Жаба vs Рыцарь - жаба побеждает
    result = visitor->visit(knight);
    EXPECT_TRUE(result);
    EXPECT_EQ(mock_observer->call_count, 1);
    EXPECT_TRUE(mock_observer->last_result);
    
    mock_observer->reset();
    // Жаба vs Жаба - жаба побеждает
    auto another_toad = std::make_shared<Toad>("AnotherToad", 3, 3);
    result = visitor->visit(another_toad);
    EXPECT_TRUE(result);
    EXPECT_EQ(mock_observer->call_count, 1);
    EXPECT_TRUE(mock_observer->last_result);
}

// Тест 2: Дракон побеждает рыцаря
TEST_F(FightVisitorTest, DragonWinsVsKnight) {
    mock_observer->reset();
    auto visitor = std::make_shared<FightVisitor>(dragon, mock_observer);
    
    bool result = visitor->visit(knight);
    EXPECT_TRUE(result);  // Дракон побеждает рыцаря
    EXPECT_EQ(mock_observer->call_count, 1);
    EXPECT_TRUE(mock_observer->last_result);
}

// Тест 3: Дракон проигрывает жабе
TEST_F(FightVisitorTest, DragonLosesVsToad) {
    mock_observer->reset();
    auto visitor = std::make_shared<FightVisitor>(dragon, mock_observer);
    
    bool result = visitor->visit(toad);
    EXPECT_FALSE(result);  // Дракон проигрывает жабе
    EXPECT_EQ(mock_observer->call_count, 1);
    EXPECT_FALSE(mock_observer->last_result);
}

// Тест 4: Дракон vs Дракон - ничья
TEST_F(FightVisitorTest, DragonVsDragonDraw) {
    mock_observer->reset();
    auto another_dragon = std::make_shared<Dragon>("AnotherDragon", 3, 3);
    auto visitor = std::make_shared<FightVisitor>(dragon, mock_observer);
    
    bool result = visitor->visit(another_dragon);
    EXPECT_FALSE(result);  // Ничья
    EXPECT_EQ(mock_observer->call_count, 1);
    EXPECT_FALSE(mock_observer->last_result);
}

// Тест 5: Рыцарь побеждает дракона
TEST_F(FightVisitorTest, KnightWinsVsDragon) {
    mock_observer->reset();
    auto visitor = std::make_shared<FightVisitor>(knight, mock_observer);
    
    bool result = visitor->visit(dragon);
    EXPECT_TRUE(result);  // Рыцарь побеждает дракона
    EXPECT_EQ(mock_observer->call_count, 1);
    EXPECT_TRUE(mock_observer->last_result);
}

// Тест 6: Рыцарь проигрывает жабе
TEST_F(FightVisitorTest, KnightLosesVsToad) {
    mock_observer->reset();
    auto visitor = std::make_shared<FightVisitor>(knight, mock_observer);
    
    bool result = visitor->visit(toad);
    EXPECT_FALSE(result);  // Рыцарь проигрывает жабе
    EXPECT_EQ(mock_observer->call_count, 1);
    EXPECT_FALSE(mock_observer->last_result);
}

// Тест 7: Рыцарь vs Рыцарь - ничья
TEST_F(FightVisitorTest, KnightVsKnightDraw) {
    mock_observer->reset();
    auto another_knight = std::make_shared<Knight>("AnotherKnight", 3, 3);
    auto visitor = std::make_shared<FightVisitor>(knight, mock_observer);
    
    bool result = visitor->visit(another_knight);
    EXPECT_FALSE(result);  // Ничья
    EXPECT_EQ(mock_observer->call_count, 1);
    EXPECT_FALSE(mock_observer->last_result);
}

// Тест 8: GetAttacker работает правильно
TEST_F(FightVisitorTest, GetAttacker) {
    auto visitor = std::make_shared<FightVisitor>(toad, mock_observer);
    EXPECT_EQ(visitor->getAttacker(), toad);
    
    visitor = std::make_shared<FightVisitor>(dragon, mock_observer);
    EXPECT_EQ(visitor->getAttacker(), dragon);
    
    visitor = std::make_shared<FightVisitor>(knight, mock_observer);
    EXPECT_EQ(visitor->getAttacker(), knight);
}

// Тест 9: Visitor без наблюдателя
TEST_F(FightVisitorTest, VisitorWithoutObserver) {
    // Должен работать без крашей даже без наблюдателя
    auto visitor = std::make_shared<FightVisitor>(toad, nullptr);
    EXPECT_NO_THROW(visitor->visit(dragon));
    
    visitor = std::make_shared<FightVisitor>(dragon, nullptr);
    EXPECT_NO_THROW(visitor->visit(knight));
    
    visitor = std::make_shared<FightVisitor>(knight, nullptr);
    EXPECT_NO_THROW(visitor->visit(dragon));
}

// Тест 10: Проверка что observer вызывается даже при проигрыше
TEST_F(FightVisitorTest, ObserverCalledEvenWhenLosing) {
    mock_observer->reset();
    auto visitor = std::make_shared<FightVisitor>(dragon, mock_observer);
    
    bool result = visitor->visit(toad);  // Дракон атакует жабу и проигрывает
    
    EXPECT_FALSE(result);  // Проигрыш
    EXPECT_EQ(mock_observer->call_count, 1);
    EXPECT_EQ(mock_observer->last_attacker, dragon);
    EXPECT_EQ(mock_observer->last_defender, toad);
    EXPECT_FALSE(mock_observer->last_result);
}

// Тест 11: Проверка всех комбинаций боев (матрица результатов)
TEST_F(FightVisitorTest, AllFightCombinations) {
    // Матрица результатов боя (attacker vs defender):
    // ✓ = победа (true), ✗ = поражение/ничья (false)
    //
    //            | Toad | Dragon | Knight
    // -----------|------|--------|--------
    // Toad       |  ✓   |   ✓    |   ✓    (жаба всех побеждает)
    // Dragon     |  ✗   |   ✗    |   ✓    (дракон побеждает только рыцаря)
    // Knight     |  ✗   |   ✓    |   ✗    (рыцарь побеждает только дракона)
    
    // Toad vs все
    auto visitor1 = std::make_shared<FightVisitor>(toad, nullptr);
    EXPECT_TRUE(visitor1->visit(dragon));   // Toad vs Dragon: ✓
    EXPECT_TRUE(visitor1->visit(knight));   // Toad vs Knight: ✓
    
    // Dragon vs все
    auto visitor2 = std::make_shared<FightVisitor>(dragon, nullptr);
    EXPECT_FALSE(visitor2->visit(toad));    // Dragon vs Toad: ✗
    EXPECT_FALSE(visitor2->visit(dragon));  // Dragon vs Dragon: ✗ (ничья)
    EXPECT_TRUE(visitor2->visit(knight));   // Dragon vs Knight: ✓
    
    // Knight vs все
    auto visitor3 = std::make_shared<FightVisitor>(knight, nullptr);
    EXPECT_FALSE(visitor3->visit(toad));    // Knight vs Toad: ✗
    EXPECT_TRUE(visitor3->visit(dragon));   // Knight vs Dragon: ✓
    EXPECT_FALSE(visitor3->visit(knight));  // Knight vs Knight: ✗ (ничья)
}

// Тест 12: Проверка что observer вызывается при победе
TEST_F(FightVisitorTest, ObserverCalledWhenWinning) {
    mock_observer->reset();
    auto visitor = std::make_shared<FightVisitor>(toad, mock_observer);
    
    bool result = visitor->visit(dragon);  // Жаба атакует дракона и побеждает
    
    EXPECT_TRUE(result);  // Победа
    EXPECT_EQ(mock_observer->call_count, 1);
    EXPECT_EQ(mock_observer->last_attacker, toad);
    EXPECT_EQ(mock_observer->last_defender, dragon);
    EXPECT_TRUE(mock_observer->last_result);
}

// Тест 13: Проверка что методы fight вызываются правильно
TEST_F(FightVisitorTest, FightMethodsAreCalled) {
    // Этот тест проверяет что для каждой комбинации вызывается
    // соответствующий метод fight()
    
    auto visitor1 = std::make_shared<FightVisitor>(toad, mock_observer);
    auto visitor2 = std::make_shared<FightVisitor>(dragon, mock_observer);
    auto visitor3 = std::make_shared<FightVisitor>(knight, mock_observer);
    
    // Все вызовы должны работать без исключений
    EXPECT_NO_THROW(visitor1->visit(toad));
    EXPECT_NO_THROW(visitor1->visit(dragon));
    EXPECT_NO_THROW(visitor1->visit(knight));
    
    EXPECT_NO_THROW(visitor2->visit(toad));
    EXPECT_NO_THROW(visitor2->visit(dragon));
    EXPECT_NO_THROW(visitor2->visit(knight));
    
    EXPECT_NO_THROW(visitor3->visit(toad));
    EXPECT_NO_THROW(visitor3->visit(dragon));
    EXPECT_NO_THROW(visitor3->visit(knight));
}