#include <iostream>
#include <fstream>
#include <memory>
#include <set>
#include <random>
#include <string>
#include <sstream>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <cstdlib>
#include <ctime>

#include "npc.h"
#include "factory.h"
#include "observer.h"
#include "fightVisitor.h"

using set_t = std::set<std::shared_ptr<NPC>>;

const int MAP_WIDTH = 100;        
const int MAP_HEIGHT = 100;       
const int GAME_DURATION = 30;     
const int INITIAL_NPC_COUNT = 50; 

std::shared_mutex game_world_mutex; 
set_t game_world;                 

std::atomic<bool> game_running{true}; 
std::mutex cout_mutex;              // для защиты вывода

// для хран задач
std::vector<std::pair<std::shared_ptr<NPC>, std::shared_ptr<NPC>>> fight_tasks;
std::mutex tasks_mutex;            

std::string generateName(const std::string& type, int n) {
    return type + "_" + std::to_string(n);
}

void safePrint(const std::string& mess) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << mess << std::endl;
}

void movementThread() {
    while (game_running) {
        std::vector<std::shared_ptr<NPC>> alive_npcs;
        
        {
            std::shared_lock<std::shared_mutex> lock(game_world_mutex);
            for (const auto& npc : game_world) {
                if (npc->isAlive()) {
                    alive_npcs.push_back(npc);
                }
            }
        }
    
        for (auto& npc : alive_npcs) {
            npc->moveRandom(); 
        }
        
        std::vector<std::pair<std::shared_ptr<NPC>, std::shared_ptr<NPC>>> new_fights;
        
        for (size_t i = 0; i < alive_npcs.size(); ++i) {
            if (!alive_npcs[i]->isAlive()) continue;
            
            for (size_t j = i + 1; j < alive_npcs.size(); ++j) {
                if (!alive_npcs[j]->isAlive()) continue;
                
                double distance = alive_npcs[i]->distance(alive_npcs[j]);
                int kill_distance_i = alive_npcs[i]->getKillDist();
                int kill_distance_j = alive_npcs[j]->getKillDist();
                
                if (distance <= kill_distance_i || distance <= kill_distance_j) {
                    if (distance <= kill_distance_i && distance <= kill_distance_j) {
                        if (std::rand() % 2 == 0) {
                            new_fights.push_back({alive_npcs[i], alive_npcs[j]});
                        } else {
                            new_fights.push_back({alive_npcs[j], alive_npcs[i]});
                        }
                    } else if (distance <= kill_distance_i) {
                        new_fights.push_back({alive_npcs[i], alive_npcs[j]});
                    } else {
                        new_fights.push_back({alive_npcs[j], alive_npcs[i]});
                    }
                }
            }
        }
        
        if (!new_fights.empty()) {
            std::lock_guard<std::mutex> lock(tasks_mutex);
            fight_tasks.insert(fight_tasks.end(), new_fights.begin(), new_fights.end());
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void fightThread(const std::shared_ptr<IFFightObserver>& observer) {
    while (game_running) {
        std::vector<std::pair<std::shared_ptr<NPC>, std::shared_ptr<NPC>>> local_tasks;
        
        {
            std::lock_guard<std::mutex> lock(tasks_mutex);
            local_tasks.swap(fight_tasks);
        }
        
        for (auto& [attacker, defender] : local_tasks) {
            if (!attacker->isAlive() || !defender->isAlive()) continue;
            
            auto visitor = std::make_shared<FightVisitor>(attacker, observer);
            bool canAttack = defender->accept(visitor);
            
            if (canAttack) {
                auto [attack_power, defense_power] = attacker->rollDice();
                
                if (attack_power > defense_power) {
                    defender->kill();
                    
                    std::unique_lock<std::shared_mutex> lock(game_world_mutex);
                    game_world.erase(defender);
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void renderThread() {
    auto start_time = std::chrono::steady_clock::now();
    
    while (game_running) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
        
        if (elapsed >= GAME_DURATION) {
            game_running = false;
            break;
        }
        
        std::vector<std::string> map(MAP_HEIGHT, std::string(MAP_WIDTH, '.'));
        int alive_count = 0;
        
        {
            std::shared_lock<std::shared_mutex> lock(game_world_mutex);
            
            for (const auto& npc : game_world) {
                if (npc->isAlive()) {
                    alive_count++;
                    int x = npc->getX();
                    int y = npc->getY();
                    
                    if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
                        char symbol = '.';
                        std::string type = npc->getType();
                        
                        if (type == "Toad") symbol = 'T';
                        else if (type == "Dragon") symbol = 'D';
                        else if (type == "Knight") symbol = 'K';
                        
                        map[y][x] = symbol;
                    }
                }
            }
        }
        
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "--------- NPC BATTLE --------" << std::endl;
            std::cout << "Time: " << elapsed << "/" << GAME_DURATION << "s | Alive: " << alive_count 
                      << " | Pending fights: " << fight_tasks.size() << std::endl;
            std::cout << "Map: " << MAP_WIDTH << "x" << MAP_HEIGHT << std::endl;
            std::cout << "T=Toad(1/10) D=Dragon(50/30) K=Knight(30/10)" << std::endl;
            std::cout << std::endl;
            
            int display_height = std::min(30, MAP_HEIGHT);
            int display_width = std::min(80, MAP_WIDTH);
            
            for (int y = 0; y < display_height; y++) {
                for (int x = 0; x < display_width; x++) {
                    std::cout << map[y][x];
                }
                std::cout << std::endl;
            }
            
            std::cout << std::endl;
            std::cout << "------------------------------" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    safePrint("     Starting game...");

    auto console_logger = std::make_shared<TextObserver>();
    {
        std::unique_lock<std::shared_mutex> lock(game_world_mutex);
        
        for (int i = 0; i < INITIAL_NPC_COUNT; ++i) {
            NpcType type = static_cast<NpcType>(std::rand() % 3);
            std::string name;
            std::string type_name;
            
            switch (type) {
                case NpcType::Toad:
                    type_name = "Toad";
                    break;
                case NpcType::Dragon:
                    type_name = "Dragon";
                    break;
                case NpcType::Knight:
                    type_name = "Knight";
                    break;
            }
            name = generateName(type_name, i);
            
            int x = std::rand() % MAP_WIDTH;
            int y = std::rand() % MAP_HEIGHT;
            
            auto npc = NPCFactory::create(type, name, x, y);
            if (npc) {
                game_world.insert(npc);
            }
        }
    }
    
    safePrint("Created " + std::to_string(INITIAL_NPC_COUNT) + " NPCs");
    safePrint("Game duration: " + std::to_string(GAME_DURATION) + " seconds");
    safePrint("Map size: " + std::to_string(MAP_WIDTH) + "x" + std::to_string(MAP_HEIGHT));
    safePrint("Starting threads...");
    
    std::thread movement_thread(movementThread);
    std::thread fight_thread(fightThread, console_logger);
    std::thread render_thread(renderThread);
    
    // ждем завершения потока отрисовки 
    render_thread.join();

    game_running = false;
    
    movement_thread.join();
    fight_thread.join();
    
    safePrint("\n     --- GAME OVER ---     ");
    safePrint("Survivors after " + std::to_string(GAME_DURATION) + " sec:");
    
    {
        std::shared_lock<std::shared_mutex> lock(game_world_mutex);
        int survivor_count = 0;
        
        for (const auto& npc : game_world) {
            if (npc->isAlive()) {
                survivor_count++;
                safePrint("  " + npc->getType() + " \"" + npc->getName() + 
                          "\" at (" + std::to_string(npc->getX()) + 
                          ", " + std::to_string(npc->getY()) + ")");
            }
        }
        
        safePrint("Total survivors: " + std::to_string(survivor_count) + 
                  "/" + std::to_string(INITIAL_NPC_COUNT));
    }
    return 0;
}