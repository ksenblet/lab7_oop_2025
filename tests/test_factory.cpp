#include <gtest/gtest.h>
#include <sstream>
#include <memory>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>

#include "factory.h"
#include "toad.h"
#include "dragon.h"
#include "knight.h"

class FactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаем тестовые потоки с координатами в диапазоне 0-100
        toad_stream = std::make_unique<std::istringstream>("Toad Toad1 10 20");
        dragon_stream = std::make_unique<std::istringstream>("Dragon Dragon1 30 40");
        knight_stream = std::make_unique<std::istringstream>("Knight Knight1 50 60");
        
        // Инициализируем генератор случайных чисел
        std::srand(static_cast<unsigned>(std::time(nullptr)));
    }
    
    void TearDown() override {
        // Удаляем тестовые файлы если они существуют
        for (const auto& filename : temp_files_to_remove) {
            std::remove(filename.c_str());
        }
    }
    
    std::string createUniqueFilename(const std::string& prefix) {
        std::string filename = prefix + "_" + std::to_string(std::rand()) + ".txt";
        temp_files_to_remove.push_back(filename);
        return filename;
    }
    
    std::unique_ptr<std::istringstream> toad_stream;
    std::unique_ptr<std::istringstream> dragon_stream;
    std::unique_ptr<std::istringstream> knight_stream;
    std::vector<std::string> temp_files_to_remove;
};

TEST_F(FactoryTest, CreateByType) {
    auto toad = NPCFactory::create(NpcType::Toad, "FactoryToad", 1, 2);
    auto dragon = NPCFactory::create(NpcType::Dragon, "FactoryDragon", 3, 4);
    auto knight = NPCFactory::create(NpcType::Knight, "FactoryKnight", 5, 6);
    
    ASSERT_NE(toad, nullptr);
    ASSERT_NE(dragon, nullptr);
    ASSERT_NE(knight, nullptr);
    
    EXPECT_EQ(toad->getType(), "Toad");
    EXPECT_EQ(dragon->getType(), "Dragon");
    EXPECT_EQ(knight->getType(), "Knight");
    
    EXPECT_EQ(toad->getName(), "FactoryToad");
    EXPECT_EQ(toad->getX(), 1);
    EXPECT_EQ(toad->getY(), 2);
}

TEST_F(FactoryTest, CreateFromStream) {
    auto toad = NPCFactory::create(*toad_stream);
    auto dragon = NPCFactory::create(*dragon_stream);
    auto knight = NPCFactory::create(*knight_stream);
    
    ASSERT_NE(toad, nullptr);
    ASSERT_NE(dragon, nullptr);
    ASSERT_NE(knight, nullptr);
    
    EXPECT_EQ(toad->getName(), "Toad1");
    EXPECT_EQ(toad->getX(), 10);
    EXPECT_EQ(toad->getY(), 20);
    
    EXPECT_EQ(dragon->getName(), "Dragon1");
    EXPECT_EQ(knight->getName(), "Knight1");
}

TEST_F(FactoryTest, SaveToStream) {
    auto toad = std::make_shared<Toad>("SavedToad", 10, 20);
    auto dragon = std::make_shared<Dragon>("SavedDragon", 30, 40);
    
    std::ostringstream output;
    NPCFactory::save(toad, output);
    NPCFactory::save(dragon, output);
    
    std::string result = output.str();
    EXPECT_NE(result.find("Toad SavedToad 10 20"), std::string::npos);
    EXPECT_NE(result.find("Dragon SavedDragon 30 40"), std::string::npos);
}

TEST_F(FactoryTest, InvalidType) {
    auto invalid = NPCFactory::create(static_cast<NpcType>(999), "Invalid", 0, 0);
    EXPECT_EQ(invalid, nullptr);
}

TEST_F(FactoryTest, InvalidStreamData) {
    std::istringstream bad_stream1("InvalidType Name 10 20");
    std::istringstream bad_stream2("Toad OnlyName");
    std::istringstream bad_stream3("");
    
    auto npc1 = NPCFactory::create(bad_stream1);
    auto npc2 = NPCFactory::create(bad_stream2);
    auto npc3 = NPCFactory::create(bad_stream3);
    
    EXPECT_EQ(npc1, nullptr);
    EXPECT_EQ(npc2, nullptr);
    EXPECT_EQ(npc3, nullptr);
}

TEST_F(FactoryTest, RoundTripSaveLoad) {
    // Создаем NPC с координатами в диапазоне 0-100
    auto original = std::make_shared<Toad>("RoundTrip", 12, 34);
    
    // Сохраняем в поток
    std::ostringstream save_stream;
    NPCFactory::save(original, save_stream);
    
    // Загружаем из потока
    std::istringstream load_stream(save_stream.str());
    auto loaded = NPCFactory::create(load_stream);
    
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->getType(), "Toad");
    EXPECT_EQ(loaded->getName(), "RoundTrip");
    EXPECT_EQ(loaded->getX(), 12);
    EXPECT_EQ(loaded->getY(), 34);
}

TEST_F(FactoryTest, LoadFromFileWithMultipleNPCs) {
    // Создаем уникальное имя файла
    std::string test_filename = createUniqueFilename("test_npcs");
    
    // Создаем файл для теста с координатами 0-100
    std::ofstream create_file(test_filename);
    ASSERT_TRUE(create_file.is_open()) << "Cannot create file: " << test_filename;
    
    create_file << "Toad Toad_Ivan 10 20\n";
    create_file << "Dragon Dragon_Petr 30 40\n";
    create_file << "Knight Knight_Vasya 50 60\n";
    create_file << "Toad Toad_Masha 70 80\n";
    create_file << "Dragon Dragon_Olya 90 10\n";
    create_file << "Knight Knight_Kolya 20 30\n";
    create_file.close();
    
    // Открываем файл и загружаем NPC
    std::ifstream input_file(test_filename);
    ASSERT_TRUE(input_file.is_open()) << "Cannot open file: " << test_filename;
    
    std::vector<std::shared_ptr<NPC>> loaded_npcs;
    std::string line;
    
    while (std::getline(input_file, line)) {
        std::istringstream line_stream(line);
        auto npc = NPCFactory::create(line_stream);
        if (npc) {
            loaded_npcs.push_back(npc);
        }
    }
    
    input_file.close();
    
    // Проверяем, что все NPC загрузились
    ASSERT_EQ(loaded_npcs.size(), 6);
    
    // Проверяем типы и данные NPC
    EXPECT_EQ(loaded_npcs[0]->getType(), "Toad");
    EXPECT_EQ(loaded_npcs[0]->getName(), "Toad_Ivan");
    EXPECT_EQ(loaded_npcs[0]->getX(), 10);
    EXPECT_EQ(loaded_npcs[0]->getY(), 20);
    
    EXPECT_EQ(loaded_npcs[1]->getType(), "Dragon");
    EXPECT_EQ(loaded_npcs[1]->getName(), "Dragon_Petr");
    EXPECT_EQ(loaded_npcs[1]->getX(), 30);
    EXPECT_EQ(loaded_npcs[1]->getY(), 40);
    
    EXPECT_EQ(loaded_npcs[2]->getType(), "Knight");
    EXPECT_EQ(loaded_npcs[2]->getName(), "Knight_Vasya");
    EXPECT_EQ(loaded_npcs[2]->getX(), 50);
    EXPECT_EQ(loaded_npcs[2]->getY(), 60);
}

TEST_F(FactoryTest, SaveMultipleNPCsToFileAndReload) {
    // Создаем тестовые NPC с координатами 0-100
    std::vector<std::shared_ptr<NPC>> original_npcs = {
        std::make_shared<Toad>("TestToad1", 11, 22),
        std::make_shared<Dragon>("TestDragon1", 33, 44),
        std::make_shared<Knight>("TestKnight1", 55, 66),
        std::make_shared<Toad>("TestToad2", 77, 88),
        std::make_shared<Dragon>("TestDragon2", 99, 10)
    };
    
    // Создаем уникальное имя файла
    std::string temp_filename = createUniqueFilename("temp_npcs");
    
    // Сохраняем в файл
    std::ofstream out_file(temp_filename);
    ASSERT_TRUE(out_file.is_open()) << "Failed to create file: " << temp_filename;
    
    for (const auto& npc : original_npcs) {
        NPCFactory::save(npc, out_file);
    }
    out_file.close();
    
    // Загружаем обратно
    std::ifstream in_file(temp_filename);
    ASSERT_TRUE(in_file.is_open()) << "Failed to open file for reading: " << temp_filename;
    
    std::vector<std::shared_ptr<NPC>> loaded_npcs;
    std::string line;
    
    while (std::getline(in_file, line)) {
        std::istringstream line_stream(line);
        auto npc = NPCFactory::create(line_stream);
        if (npc) {
            loaded_npcs.push_back(npc);
        }
    }
    in_file.close();
    
    // Проверяем что загрузилось столько же NPC
    ASSERT_EQ(original_npcs.size(), loaded_npcs.size());
    
    // Проверяем что данные совпадают
    for (size_t i = 0; i < original_npcs.size(); ++i) {
        EXPECT_EQ(original_npcs[i]->getType(), loaded_npcs[i]->getType());
        EXPECT_EQ(original_npcs[i]->getName(), loaded_npcs[i]->getName());
        EXPECT_EQ(original_npcs[i]->getX(), loaded_npcs[i]->getX());
        EXPECT_EQ(original_npcs[i]->getY(), loaded_npcs[i]->getY());
    }
}