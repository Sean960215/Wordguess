/*
    Word Warrior - C++ RPG Typing Game using rlutil
    Author: AI Assistant
    Note: Requires "rlutil.h" in the same directory.
*/

#include "rlutil.h"
#include <iostream>
#include <string>
#include <vector>
#include <ctime>   // 用於亂數種子
#include <cstdlib> // 用於 rand()
#include <fstream>
#include <sstream>
#include <chrono>

using namespace std;

// ==========================================
// 資料模型 (Data Models)
// ==========================================

// 單字結構
struct Word {
    string english;
    string chinese;
};

// 內建一個小型題庫 (實際專案可改為讀取 txt 檔)
vector<Word> dictionary = {
    {"apple", "蘋果"}, {"banana", "香蕉"}, {"orange", "橘子"},
    {"computer", "電腦"}, {"keyboard", "鍵盤"}, {"mouse", "滑鼠"},
    {"program", "程式"}, {"developer", "開發者"}, {"algorithm", "演算法"},
    {"university", "大學"}, {"student", "學生"}, {"library", "圖書館"},
    {"adventure", "冒險"}, {"warrior", "戰士"}, {"dragon", "龍"}
};

// 技能資料
struct Skill {
    string name;
    int cooldownMs;
    int durationMs; // 0 表示瞬發
    std::chrono::steady_clock::time_point nextReady;
    std::chrono::steady_clock::time_point activeUntil;

    Skill(string n, int cd, int dur)
        : name(n), cooldownMs(cd), durationMs(dur) {
        nextReady = std::chrono::steady_clock::now();
        activeUntil = std::chrono::steady_clock::time_point::min();
    }

    bool isReady(const std::chrono::steady_clock::time_point& now) const {
        return now >= nextReady;
    }

    bool isActive(const std::chrono::steady_clock::time_point& now) const {
        return durationMs > 0 && now < activeUntil;
    }

    int remainingMs(const std::chrono::steady_clock::time_point& now) const {
        if (isReady(now)) return 0;
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(nextReady - now).count();
        return static_cast<int>(diff);
    }
};

// 基本實體類別 (玩家與怪物的共同特徵)
class Entity {
public:
    string name;
    int hp;
    int maxHp;

    Entity(string n, int h) : name(n), maxHp(h), hp(h) {}

    // 受傷方法
    void takeDamage(int dmg) {
        hp -= dmg;
        if (hp < 0) hp = 0;
    }

    // 檢查是否死亡
    bool isDead() { return hp <= 0; }
};

// 怪物類別 (繼承自 Entity)
class Monster : public Entity {
public:
    int colorId; // 怪物顏色

    Monster(string n, int h, int c) : Entity(n, h), colorId(c) {}

    // 繪製怪物的方法
    void draw(int x, int y) {
        rlutil::setColor(colorId);
        rlutil::locate(x, y);     cout << "  /---\\   ";
        rlutil::locate(x, y + 1); cout << " | o o |  ";
        rlutil::locate(x, y + 2); cout << "  \\_^_/   ";
        rlutil::locate(x, y + 3); cout << " (" << name << ")";
        rlutil::resetColor();
    }
};

// 簡化模式：純文字互動，不使用游標控制
int runSimpleModeCLI() {
    Entity player("Hero", 50);
    int score = 0;
    int level = 1;

    auto newMonsterForLevel = [&](int lvl) {
        int newHp = 10 + (lvl * 5);
        string newName;
        if (lvl % 3 == 0) { newName = "Dragon Boss"; newHp += 20; }
        else if (lvl % 2 == 0) { newName = "Goblin"; }
        else { newName = "Slime"; }
        return Monster(newName, newHp, 0);
    };

    Monster monster = newMonsterForLevel(level);
    Word current = dictionary[rand() % dictionary.size()];

    cout << "=== WORD WARRIOR (Simple Mode) ===\n";
    cout << "Type your answer and press Enter. Type 'esc' to quit.\n\n";

    string input;
    while (!player.isDead()) {
        cout << "Level " << level << "  Score " << score << "  ";
        cout << "Monster: " << monster.name << " HP " << monster.hp << "/" << monster.maxHp << "\n";
        cout << "Question: " << current.chinese << " (" << current.english.length() << " letters)\n";
        cout << "> ";
        std::getline(cin, input);
        if (input == "esc" || input == "ESC") break;
        for (auto &ch : input) ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
        if (input == current.english) {
            int dmg = 5 + (rand()%5);
            monster.takeDamage(dmg);
            score += dmg * 10;
            cout << "Hit! Dealt " << dmg << " damage.\n\n";
            if (!monster.isDead()) current = dictionary[rand()%dictionary.size()];
        } else {
            int dmg = 2 + (rand()%3);
            player.takeDamage(dmg);
            cout << "Miss! You took " << dmg << " damage.\n\n";
        }
        if (monster.isDead()) {
            level++; score += 100; player.hp = player.maxHp;
            monster = newMonsterForLevel(level);
            current = dictionary[rand()%dictionary.size()];
        }
    }
    cout << "Game Over. Final Score: " << score << "  Max Level: " << level << "\n";
    return 0;
}

// ==========================================
// 介面輔助函式 (UI Helpers)
// ==========================================

// 繪製血條的通用函式
void drawHPBar(int x, int y, int current, int max, int color) {
    rlutil::locate(x, y);
    rlutil::setColor(color);
    cout << "[";
    // 計算血條長度 (總長 10 格)
    int filled = (int)((double)current / max * 10);
    for (int i = 0; i < 10; i++) {
        if (i < filled) cout << "█"; // 實心方塊
        else cout << "-";
    }
    cout << "] " << current << "/" << max << "  ";
    rlutil::resetColor();
}

// 清除特定區域的文字 (防止畫面閃爍用)
void clearArea(int x, int y, int length) {
    rlutil::locate(x, y);
    for (int i = 0; i < length; i++) cout << " ";
}

// 螢幕寬度（失敗回傳 100）
int screenWidth() {
    int w = rlutil::tcols();
    if (w <= 0) w = 100;
    return w;
}

int screenHeight() {
    int h = rlutil::trows();
    if (h <= 0) h = 25;
    return h;
}

// 在特定位置輸出並用空白補齊，避免殘影
void printLineAt(int x, int y, const string& text, int width = 0) {
    if (width <= 0) width = screenWidth();
    rlutil::locate(x, y);
    cout << text;
    int pad = width - (int)text.size();
    if (pad > 0) cout << string(pad, ' ');
}

// 以背景色鋪滿一整列（從 x=1 開始）
void fillRow(int y, int bgColor, int width = 0) {
    if (width <= 0) width = screenWidth();
    rlutil::locate(1, y);
    rlutil::setBackgroundColor(bgColor);
    cout << string(width, ' ');
    rlutil::resetColor();
}

// 填滿整個畫面背景色
void fillScreen(int bgColor) {
    int rows = screenHeight();
    int cols = screenWidth();
    for (int y = 1; y <= rows; ++y) {
        rlutil::locate(1, y);
        rlutil::setBackgroundColor(bgColor);
        cout << string(cols, ' ');
    }
    rlutil::resetColor();
}

// 嘗試讀取外部詞庫 (每行: english<space>中文)
void loadDictionaryFromFile(const string& path) {
    ifstream fin(path);
    if (!fin) return;

    vector<Word> loaded;
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string en, zh;
        ss >> en;
        getline(ss, zh);
        if (!en.empty()) {
            // 去除前導空白
            size_t pos = zh.find_first_not_of(" \t");
            if (pos != string::npos) zh = zh.substr(pos);
            loaded.push_back({en, zh.empty() ? "?" : zh});
        }
    }
    if (!loaded.empty()) {
        dictionary = loaded;
    }
}

// 建立提示字串：顯示母音，其餘用 '_'
string vowelHint(const string& word) {
    string out;
    for (char c : word) {
        char lc = static_cast<char>(tolower(c));
        if (lc=='a'||lc=='e'||lc=='i'||lc=='o'||lc=='u') out.push_back(c);
        else out.push_back('_');
    }
    return out;
}

// ==========================================
// 遊戲主邏輯 (Game Engine)
// ==========================================

int main(int argc, char** argv) {
    // --- 初始化設定 ---
    srand(time(0));            // 設定亂數種子
    // 先判定簡化模式（避免先進入游標/顏色操作）
    bool simpleMode = false; // 在傳統 cmd 走相容路徑（阻塞讀取）
    for (int i = 1; i < argc; ++i) {
        string a = argv[i];
        if (a == "--simple" || a == "-s") simpleMode = true;
    }
    if (!simpleMode) {
#ifdef _WIN32
        // 沒有 Windows Terminal / VS Code 等環境痕跡時，啟用相容模式
        if (!getenv("WT_SESSION") && !getenv("TERM_PROGRAM")) simpleMode = true;
#endif
    }

    if (simpleMode) {
        loadDictionaryFromFile("words.txt");
        runSimpleModeCLI();
        return 0;
    }

    // 進入進階模式才使用 rlutil 畫面控制
    rlutil::saveDefaultColor();// 儲存預設顏色設定
    rlutil::hidecursor();      // 隱藏游標
    rlutil::cls();             // 清除螢幕

    loadDictionaryFromFile("words.txt"); // 如果存在則覆蓋內建詞庫
    Entity player("Hero", 50); // 玩家初始血量 50
    int score = 0;
    int level = 1;
    bool exitGame = false;     // 允許玩家提前退出

    // 技能 (1 顯示母音, 2 冰凍怪攻擊 3 秒, 3 斬擊傷害)
    Skill skillReveal("Reveal Vowels", 8000, 4000);
    Skill skillFreeze("Freeze", 10000, 3000);
    Skill skillSlash("Slash", 6000, 0);

    // 初始怪物
    Monster currentMonster("Slime", 10, rlutil::LIGHTGREEN);
    Word currentTarget = dictionary[rand() % dictionary.size()];
    string inputBuffer = "";   // 玩家目前輸入的字串
    string feedback = "";      // 顯示答對/答錯的提示訊息

    enum GameState { STATE_START, STATE_PLAY, STATE_GAMEOVER };
    GameState state = STATE_START;

    auto lastFrame = std::chrono::steady_clock::now();
    string lastQuestionLine = ""; // 用於減少重繪閃爍
    string lastHintLine = "";

    // --- 遊戲主迴圈 ---
    while (!player.isDead() && !exitGame) {
        auto now = std::chrono::steady_clock::now();
        auto deltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFrame).count();
        lastFrame = now;
        
        // 1. 繪製靜態介面 (整個畫面重畫)
        rlutil::resetColor();       // 先恢復預設，避免前景色殘留
        rlutil::setColor(rlutil::WHITE);
        rlutil::cls();
        // 黑色背景鋪底
        fillScreen(rlutil::BLACK);
        
        // 頂部資訊列
        rlutil::setColor(rlutil::YELLOW);
        cout << "=== WORD WARRIOR RPG ===" << endl;
        cout << "LEVEL: " << level << "   SCORE: " << score << endl;
        cout << "========================" << endl;

        if (state == STATE_START) {
            rlutil::setColor(rlutil::WHITE);
            printLineAt(4, 5, "Press ENTER to start, ESC to quit");
            printLineAt(4, 7, "Controls: Enter=confirm, Backspace=delete, 1/2/3 skills");
            cout.flush();

            int key = rlutil::getkey(); // 阻塞等待，避免不同終端 kbhit 行為差異
            if (key == rlutil::KEY_ESCAPE) { exitGame = true; }
            else if (key == rlutil::KEY_ENTER) { state = STATE_PLAY; feedback = ""; inputBuffer.clear(); }
            continue;
        }

        // 繪製玩家狀態 (左側)
        rlutil::locate(2, 5); cout << "PLAYER: " << player.name;
        drawHPBar(2, 6, player.hp, player.maxHp, rlutil::LIGHTCYAN);

        // 繪製怪物狀態 (右側)
        currentMonster.draw(40, 4);
        drawHPBar(40, 8, currentMonster.hp, currentMonster.maxHp, rlutil::LIGHTRED);

        // 中間分隔線
        rlutil::setColor(rlutil::GREY);
        printLineAt(1, 12, string(screenWidth(), '-'));

        // 底部互動區（帶底色列）
        fillRow(14, rlutil::LIGHTBLUE);
        rlutil::setColor(rlutil::WHITE);
        printLineAt(5, 14, "Monster Question: " + currentTarget.chinese + " (" + to_string((int)currentTarget.english.length()) + " letters)");

        fillRow(15, rlutil::BLUE);
        rlutil::setColor(rlutil::WHITE);
        printLineAt(5, 15, "Hint: (press 1 to reveal vowels)");

        // 戰鬥迴圈
        while (!currentMonster.isDead() && !player.isDead() && !exitGame) {
            now = std::chrono::steady_clock::now();

            // 更新題目與提示（僅在變更時重繪，減少閃爍）
            string questionLine = "Monster Question: " + currentTarget.chinese + " (" + to_string(currentTarget.english.length()) + " letters)";
            if (questionLine != lastQuestionLine) {
                fillRow(14, rlutil::LIGHTBLUE);
                rlutil::setColor(rlutil::WHITE);
                printLineAt(5, 14, questionLine);
                lastQuestionLine = questionLine;
            }

            string hintText;
            if (skillReveal.isActive(now)) {
                hintText = "Hint: " + vowelHint(currentTarget.english);
            } else {
                hintText = "Hint: (press 1 to reveal vowels)";
            }
            if (hintText != lastHintLine) {
                fillRow(15, rlutil::BLUE);
                rlutil::setColor(rlutil::WHITE);
                printLineAt(5, 15, hintText);
                lastHintLine = hintText;
            }

            // 顯示玩家目前的輸入
            fillRow(16, rlutil::LIGHTBLUE);
            rlutil::setColor(rlutil::WHITE);
            printLineAt(5, 16, "Your Input: " + inputBuffer);

            // 顯示回饋訊息
            fillRow(18, rlutil::LIGHTBLUE);
            printLineAt(5, 18, feedback);

            // 顯示技能冷卻狀態
            rlutil::setColor(rlutil::WHITE);
            auto cd1 = skillReveal.isReady(now) ? 0 : skillReveal.remainingMs(now);
            auto cd2 = skillFreeze.isReady(now) ? 0 : skillFreeze.remainingMs(now);
            auto cd3 = skillSlash.isReady(now) ? 0 : skillSlash.remainingMs(now);
            string cdLine = "[1]Reveal " + (cd1==0?"Ready":to_string(cd1/1000.0f)+"s") +
                            "   [2]Freeze " + (cd2==0?"Ready":to_string(cd2/1000.0f)+"s") +
                            "   [3]Slash " + (cd3==0?"Ready":to_string(cd3/1000.0f)+"s");
            fillRow(20, rlutil::LIGHTBLUE);
            printLineAt(5, 20, cdLine);

            // 強制刷新輸出，避免在部分終端緩衝未顯示
            cout.flush();

            if (!simpleMode) {
                // 非阻塞按鍵模式（Windows Terminal / VS Code）
                if (kbhit()) {
                int key = rlutil::getkey();

                if (key == rlutil::KEY_ESCAPE) {
                    exitGame = true;
                    break;
                } else if (key == rlutil::KEY_ENTER) {
                    // 確認答案
                    if (inputBuffer == currentTarget.english) {
                        int damage = 5 + (rand() % 5);
                        currentMonster.takeDamage(damage);
                        score += damage * 10;
                        feedback = "Hit! Dealt " + to_string(damage) + " damage!";
                        drawHPBar(40, 8, currentMonster.hp, currentMonster.maxHp, rlutil::LIGHTRED);
                    } else {
                        int damage = 2 + (rand() % 3);
                        if (!skillFreeze.isActive(now)) {
                            player.takeDamage(damage);
                            feedback = "Miss! You took " + to_string(damage) + " damage!";
                        } else {
                            feedback = "Miss! Frozen shielded the hit.";
                        }
                        drawHPBar(2, 6, player.hp, player.maxHp, rlutil::LIGHTCYAN);
                    }
                    // 無論對錯，換下一個單字（若怪物已死則外層會產生新怪與新字）
                    if (!currentMonster.isDead()) {
                        currentTarget = dictionary[rand() % dictionary.size()];
                    }
                    inputBuffer.clear();
                } else if (key == 8 || key == 127) {
                    if (!inputBuffer.empty()) inputBuffer.pop_back();
                } else if (key == '1') {
                    if (skillReveal.isReady(now)) {
                        skillReveal.activeUntil = now + std::chrono::milliseconds(skillReveal.durationMs);
                        skillReveal.nextReady = now + std::chrono::milliseconds(skillReveal.cooldownMs);
                        feedback = "Skill 1: Reveal vowels!";
                    }
                } else if (key == '2') {
                    if (skillFreeze.isReady(now)) {
                        skillFreeze.activeUntil = now + std::chrono::milliseconds(skillFreeze.durationMs);
                        skillFreeze.nextReady = now + std::chrono::milliseconds(skillFreeze.cooldownMs);
                        feedback = "Skill 2: Monster frozen!";
                    }
                } else if (key == '3') {
                    if (skillSlash.isReady(now)) {
                        int damage = 8 + (rand() % 5);
                        currentMonster.takeDamage(damage);
                        score += damage * 12;
                        feedback = "Skill 3: Slash dealt " + to_string(damage) + "!";
                        skillSlash.nextReady = now + std::chrono::milliseconds(skillSlash.cooldownMs);
                        drawHPBar(40, 8, currentMonster.hp, currentMonster.maxHp, rlutil::LIGHTRED);
                        if (currentMonster.isDead()) break;
                    }
                } else if (key >= 'a' && key <= 'z') {
                    if (inputBuffer.length() < 20) inputBuffer.push_back(static_cast<char>(key));
                } else if (key >= 'A' && key <= 'Z') {
                    if (inputBuffer.length() < 20) inputBuffer.push_back(static_cast<char>(tolower(key)));
                }
                }
            } else {
                // 簡化模式：使用阻塞輸入一整行
                rlutil::locate(17, 16);
                std::getline(cin, inputBuffer);
                if (inputBuffer == "esc" || inputBuffer == "ESC") { exitGame = true; break; }
                for (auto &ch : inputBuffer) ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));

                if (inputBuffer == currentTarget.english) {
                    int damage = 5 + (rand() % 5);
                    currentMonster.takeDamage(damage);
                    score += damage * 10;
                    feedback = "Hit! Dealt " + to_string(damage) + " damage!";
                    drawHPBar(40, 8, currentMonster.hp, currentMonster.maxHp, rlutil::LIGHTRED);
                } else {
                    int damage = 2 + (rand() % 3);
                    player.takeDamage(damage);
                    feedback = "Miss! You took " + to_string(damage) + " damage!";
                    drawHPBar(2, 6, player.hp, player.maxHp, rlutil::LIGHTCYAN);
                }
                if (!currentMonster.isDead()) currentTarget = dictionary[rand() % dictionary.size()];
                inputBuffer.clear();
            }

            if (!simpleMode) rlutil::msleep(40);
        }

        // --- 戰鬥結束判定 ---
        if (player.isDead()) {
            break; // 玩家死亡，跳出主迴圈
        }

        if (currentMonster.isDead()) {
            // 怪物死亡，升級並產生新怪物
            level++;
            score += 100; // 擊殺獎勵
            player.hp = player.maxHp; // 玩家補滿血
            feedback = "Monster defeated! Level Up! HP Restored.";

            // 產生更強的怪物
            int newHp = 10 + (level * 5);
            string newName;
            int newColor;

            // 簡單的怪物生成邏輯
            if (level % 3 == 0) {
                 newName = "Dragon Boss"; newColor = rlutil::LIGHTRED; newHp += 20;
            } else if (level % 2 == 0) {
                 newName = "Goblin"; newColor = rlutil::BROWN;
            } else {
                 newName = "Slime"; newColor = rlutil::LIGHTGREEN;
            }
            
            currentMonster = Monster(newName, newHp, newColor);
            currentTarget = dictionary[rand() % dictionary.size()];
            inputBuffer = "";
            // (外層迴圈會再次執行 rlutil::cls() 重繪整個介面)
        }
    }

    // --- 遊戲結束畫面 ---
    rlutil::cls();
    rlutil::setColor(rlutil::LIGHTRED);
    rlutil::locate(30, 10); cout << "===========================";
    rlutil::locate(30, 11); cout << "   G A M E   O V E R   ";
    rlutil::locate(30, 12); cout << "===========================";
    rlutil::setColor(rlutil::YELLOW);
    rlutil::locate(32, 14); cout << "Final Score: " << score;
    rlutil::locate(32, 15); cout << "Max Level:   " << level;
    
    rlutil::locate(1, 20);
    rlutil::showcursor(); // 恢復游標
    rlutil::resetColor(); // 恢復顏色
    cout << "Press Enter to exit...";
    cin.get(); // 等待玩家按鍵

    return 0;
}
