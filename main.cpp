#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>
#include <chrono>
#include <sstream>
#include <iomanip>
using namespace sf;
using namespace std;
//using Clock = chrono::steady_clock;
#define DELETE_KEY 8
#define ENTER_KEY 13

enum GameState {
    WelcomeScreen,
    GameScreen,
    Leaderboard
};

struct Player {
    string name;
    chrono::duration<double> time;
    Player() : time(0) {};
    Player(string n) : name(n),  time(0) {};
    int timeAsInt() const {
        return static_cast<int>(time.count());
    }
    bool operator<(const Player& other) const {
        return time < other.time;
    }
    bool operator>(const Player& other) const {
        return time > other.time;
    }
    bool operator==(const Player& other) const {
        return name == other.name && time == other.time;
    }
};

class Tile {
    int type;
    bool shown;
    bool flagged;
    Sprite sprite;
    Texture texture;
public:
    Tile() : type(0), shown(false), flagged(false) {}
    Tile(int t) : type(t), shown(false), flagged(false) {}
    void setTile(int type) {
        this->type = type;
        this->shown = false;
        this->flagged = false;
    }
    void setSprite(Sprite s) {
        this->sprite = s;
    }
    void setTexture(Texture t) {
        this->texture = t;
        sprite.setTexture(this->texture);
    }
    void setTexture(map<int, Texture>& t) {
        if (t.find(type) != t.end()) {
            this->texture = t[type];
            sprite.setTexture(t[type]);
        }
    }
    void setPosition(float x, float y) {
        sprite.setPosition(x, y);
    }
    int getTile() {
        return type;
    }

    Sprite& getSprite() {
        return sprite;
    }

    bool isRevealed() {
        return shown;
    }

    bool isFlagged() {
        return flagged;
    }

    void reveal() {
        shown = true;
    }

    void flag() {
        flagged = true;
    }

    void unflag() {
        flagged = false;
    }

    void resetTile() {
        shown = false;
        flagged = false;
    }
};

map<int, Texture> loadDigitTextures() {
    map<int, Texture> digits;
    Texture tex;
    for (int i = 0; i <= 9; i++) {
        tex.loadFromFile("files/images/number_" + to_string(i) + ".png");
        digits[i] = tex;
    }
    return digits;
}

class Board {
    int width, height, mines;
    vector<vector<Tile>> tiles;

public:
    Board();
    Board(int w, int h, int m) : width(w), height(h), mines(m) {
        tiles.resize(width, vector<Tile>(height));
    }

    void fillBoard() {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                tiles[i][j] = Tile(0);
            }
        }
    }

    void numberBoard() {
        std::vector<std::pair<int, int>> positions;
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                positions.push_back({i, j});
            }
        }

        std::random_shuffle(positions.begin(), positions.end());

        for (int i = 0; i < mines; i++) {
            int x = positions[i].first;
            int y = positions[i].second;
            tiles[x][y] = Tile(9); // 9 = mine
        }

        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {

                if (tiles[i][j].getTile()==9){continue;}
                int n = 0;

                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        if (dx == 0 && dy == 0) continue;

                        int ni = i + dx;
                        int nj = j + dy;

                        if (ni >= 0 && ni < width && nj >= 0 && nj < height) {
                            if (tiles[ni][nj].getTile() == 9) {
                                n++;
                            }
                        }
                    }
                }

                tiles[i][j].setTile(n);
            }
        }
    }

    void revealBoard() {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                tiles[i][j].reveal();
            }
        }
    }

    void revealTile(int x, int y) {
        tiles[x][y].reveal();
    }

    void resetBoard() {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                tiles[i][j].resetTile();
            }
        }
    }

    void endGame() {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                if (tiles[i][j].getTile() == 9) {
                    tiles[i][j].reveal();
                }
            }
        }
    }

    void flagTile(int x, int y) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            tiles[x][y].flag();
        }
    }

    void unflagTile(int x, int y) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            tiles[x][y].unflag();
        }
    }

    void debug() {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                if (tiles[i][j].getTile() == 9) {
                    tiles[i][j].reveal();
                }
            }
        }
    }

    void undebug() {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                if (tiles[i][j].getTile() == 9) {
                    tiles[i][j].resetTile();
                }
            }
        }
    }

    void revealEmpty(int x, int y) {
        if (x < 0 || y < 0 || x >= width || y >= height) return;

        Tile& tile = tiles[x][y];
        if (tile.isRevealed() || tile.isFlagged()) return;

        tile.reveal();

        if (tile.getTile() == 0) {
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx != 0 || dy != 0)
                        revealEmpty(x + dx, y + dy);
                }
            }
        }
    }

    void drawBoard(RenderWindow &window) {
        static Texture hidden;
        static Texture revealed;
        static Texture flag;
        static Texture mine;
        static Texture numbers[8];

        static bool texturesLoaded = false;
        if (!texturesLoaded) {
            hidden.loadFromFile("files/images/tile_hidden.png");
            revealed.loadFromFile("files/images/tile_revealed.png");
            flag.loadFromFile("files/images/flag.png");
            mine.loadFromFile("files/images/mine.png");
            for (int i = 0; i < 8; ++i) {
                numbers[i].loadFromFile("files/images/number_" + std::to_string(i + 1) + ".png");
            }
            texturesLoaded = true;
        }

        for (int i = 0; i < width; ++i) {
            for (int j = 0; j < height; ++j) {
                Tile& tile = tiles[i][j];
                Sprite s;

                if (!tile.isRevealed()) {
                    s.setTexture(hidden);
                } else {
                    s.setTexture(revealed);
                }
                s.setPosition(i * 32, j * 32);
                window.draw(s);

                if (!tile.isRevealed() && tile.isFlagged()) {
                    Sprite f;
                    f.setTexture(flag);
                    f.setPosition(i*32, j*32);
                    window.draw(f);
                }
                if (tile.isRevealed()) {
                    int val = tile.getTile();
                    if (val >= 1 && val <= 8) {
                        Sprite n;
                        n.setTexture(numbers[val - 1]);
                        n.setPosition(i*32, j*32);
                        window.draw(n);
                    } else if (val == 9) {
                        Sprite m;
                        m.setTexture(mine);
                        m.setPosition(i*32, j*32);
                        window.draw(m);
                    }
                }
            }
        }
    }

    void drawPause(RenderWindow& window) {
        static Texture revealed;
        revealed.loadFromFile("files/images/tile_revealed.png");
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                Tile& tile = tiles[i][j];
                Sprite s;
                s.setTexture(revealed);
                s.setPosition(i * 32, j * 32);
                window.draw(s);
            }
        }
    }


    vector<vector<Tile>> getTiles() {
        return tiles;
    }

    Tile getTile(int x, int y) {
        return tiles[x][y];
    }

    bool checkWin() {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                Tile& tile = tiles[i][j];
                if (tiles[i][j].getTile() != 9 && !tiles[i][j].isRevealed()) {
                    return false;
                }
            }
        } return true;
    }

    void printBoard() {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                cout << tiles[i][j].getTile() << " ";
            }
        }
    }
};

class Button {
    Sprite s;
    int width, height;
    float x, y;
    public:
    Button() {}
    Button(Texture& texture) {
        s.setTexture(texture);
    }
    void setButtonPosition(float x, float y) {
        s.setPosition(x, y);
    }
    void setButtonTexture(Texture& texture) {
        this->s.setTexture(texture);
    }
    bool withinBtnPosition(float x, float y) {
        FloatRect bounds = s.getGlobalBounds();
        return bounds.contains(Vector2f(x, y));
    }
    Sprite getSprite() {
        return s;
    }
    void drawButton(RenderWindow &window) {
        window.draw(s);
    }
};

void setText(sf::Text &text, float x, float y){
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width/2.0f,
    textRect.top + textRect.height/2.0f);
    text.setPosition(sf::Vector2f(x, y));
}

vector<string> readConfig(const string& path) {
    ifstream file(path);
    vector<string> windowSize;
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            windowSize.push_back(line);
        }
    } else {
        cout << "Error opening file" << endl;
    }
    file.close();
    return windowSize;
}

chrono::duration<double> string_to_duration_double(const string& time_str) {
    size_t colon_pos = time_str.find(':');
    if (colon_pos == string::npos) {
        return chrono::duration<double>(0);
    }

    int minutes = stoi(time_str.substr(0, colon_pos));
    int seconds = stoi(time_str.substr(colon_pos + 1));
    return chrono::duration<double>(minutes * 60 + seconds);
}

string format_duration(chrono::duration<double> dur) {
    int total_seconds = static_cast<int>(dur.count());
    int minutes = total_seconds / 60;
    int seconds = total_seconds % 60;
    stringstream ss;
    ss << setfill('0') << setw(2) << minutes << ":"
       << setfill('0') << setw(2) << seconds;
    return ss.str();
}

struct LeaderBoard {
    int width, height;
    string filename;
    vector<Player> players;
    string leaderboardText;
    LeaderBoard() : width(0), height(0), filename("") {}
    LeaderBoard(int w, int h, string f) : width(w), height(h), filename(f) {}
    vector<Player> readFromFile(const string& filename) {
        vector<Player> players;
        ifstream file(filename);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                if (line.empty()) continue;
                stringstream ss(line);
                string time, name;
                getline(ss, time, ',');
                ss >> ws >> name;  // skip space, then get name

                Player p(name);
                p.time = string_to_duration_double(time);
                players.push_back(p);
            }
            file.close();
        }
        sort(players.begin(), players.end());
        return players;
    }

    string getLeaderboardText(){
        leaderboardText.clear();
        int rank = 1;
        for (int i=0; i<players.size(); i++) {
            leaderboardText += to_string(rank) + ".\t" + format_duration(players[i].time) + "\t" +
                               players[i].name + "\n\n";
            rank++;
        }
        return leaderboardText;
    }

    void writeToFile(const string& filename) {
        ofstream file(filename);
        if (file.is_open()) {
            for (const Player& p : players) {
                file << format_duration(p.time) << ", " << p.name << endl;
            }
            file.close();
        }
    }
    void addWinner(chrono::duration<double>(time), const string& name) {
        players = readFromFile(filename);
        Player winner(name);
        winner.time = time;
        players.push_back(winner);
        sort(players.begin(), players.end());

        if (players.size() > 5) {
            players.resize(5);
        }
        writeToFile(filename);
    }
};

void drawTwoDigitNumber(RenderWindow& window, Sprite& digit, int value, float x, float y) {
    if (value < 0) value = 0;
    if (value > 99) value = 99;

    string numStr;

    if (value < 10) {
        numStr = "0" + to_string(value);
    } else {
        numStr = to_string(value);
    }

    for (int i = 0; i < 2; i++) {
        int digitVal = numStr[i] - '0';
        digit.setTextureRect(IntRect(21 * digitVal, 0, 21, 32));
        digit.setPosition(x + 21 * i, y);
        window.draw(digit);
    }
}

void drawCounter(RenderWindow& window, Sprite& digit, int counter, int rowCount) {
    int xStart = 33;
    int yStart = 32 * (rowCount + 0.5f) + 16;

    std::string numStr;
    bool isNegative = false;

    if (counter < 0) {
        isNegative = true;
        counter = -counter;
    }

    numStr = std::to_string(counter);

    while (numStr.length() < 3) {
        numStr = "0" + numStr;
    }

    int digitIndex = 0;

    if (isNegative) {
        digit.setTextureRect(IntRect(210, 0, 21, 32)); // Adjust if '-' is elsewhere
        digit.setPosition(xStart, yStart);
        window.draw(digit);
        digitIndex++;
    }

    for (char c : numStr) {
        int n = c - '0';
        digit.setTextureRect(IntRect(21 * n, 0, 21, 32));
        digit.setPosition(xStart + 21 * digitIndex, yStart);
        window.draw(digit);
        digitIndex++;
    }
}


int main() {
    GameState curr_screen = GameState::WelcomeScreen;
    vector<string> windowSize = readConfig("files/config.cfg");
    int colCount = stoi(windowSize[0]);
    int rowCount = stoi(windowSize[1]);
    int mineCount = stoi(windowSize[2]);
    int width = colCount * 32;
    int height = (rowCount * 32)+100;
    int flagCount = mineCount;
    RenderWindow window(VideoMode(width, height), "Minesweeper");

    Clock blinkClock;
    bool cursorVisible = true;
    Player player;
    Board board(colCount, rowCount, mineCount);
    Clock clock;
    int pausedTime = 0;
    int finalTime = 0;
    Time pauseStartTime;
    bool pauseCaptured = false;
    bool boardfilled = false;
    bool gameOver = false;
    bool gameWon = false;
    bool gamePaused = false;
    bool displayLeaderboard = false;
    bool debuggerPressed = false;

    Texture digits;
    digits.loadFromFile("files/images/digits.png");
    Sprite digit;
    digit.setTexture(digits);

    Texture happy;
    happy.loadFromFile("files/images/face_happy.png");
    Button happyFace(happy);

    Texture sad;
    sad.loadFromFile("files/images/face_lose.png");

    Texture victory;
    victory.loadFromFile("files/images/face_win.png");

    Texture deb;
    deb.loadFromFile("files/images/debug.png");
    Button debugger(deb);

    Texture pause;
    pause.loadFromFile("files/images/pause.png");
    Button wait(pause);

    Texture play;
    play.loadFromFile("files/images/play.png");

    Texture lead;
    lead.loadFromFile("files/images/leaderboard.png");
    Button leaderBoard(lead);

    Font arial;
    arial.loadFromFile("arial.ttf");

    Text text;
    text.setFont(arial);
    text.setString("WELCOME TO MINESWEEPER!");
    text.setCharacterSize(24);
    text.setFillColor(Color::White);
    text.setStyle(Text::Bold | Text::Underlined);
    setText(text, width/2, (height/2)-150);

    Text name;
    name.setFont(arial);
    name.setString("Enter your name: ");
    name.setCharacterSize(20);
    name.setFillColor(Color::White);
    name.setStyle(Text::Bold);
    setText(name, width/2, (height/2)-75);

    Text username;
    username.setFont(arial);
    username.setCharacterSize(18);
    username.setFillColor(Color::Yellow);
    username.setStyle(Text::Bold);
    setText(username, width/2, (height/2)-45);
    string display;

    Text leadTitle;
    leadTitle.setFont(arial);
    leadTitle.setCharacterSize(20);
    leadTitle.setString("LEADERBOARD");
    leadTitle.setStyle(Text::Bold | Text::Underlined);
    leadTitle.setFillColor(Color::White);
    setText(leadTitle, ((rowCount* 16) + 50)/2, ((colCount * 16)/2)-120);

    LeaderBoard l((rowCount* 16 + 50)/2, (colCount * 16)/2 +20, "files/leaderboard.txt");
    Text leaderboardDisplay;
    leaderboardDisplay.setFont(arial);
    leaderboardDisplay.setCharacterSize(18);
    leaderboardDisplay.setFillColor(Color::White);
    leaderboardDisplay.setStyle(Text::Bold);

    RectangleShape cursor;
    cursor.setSize(Vector2f(2, 18));
    cursor.setFillColor(Color::White);
    cursor.setPosition(Vector2f(width/2, (height/2) - 45));



    while(window.isOpen()) {
        Event evnt;
        while(window.pollEvent(evnt)) {
            switch (evnt.type)
            {
                case Event::Closed:
                    window.close();
                break;
                case Event::TextEntered:
                    if (curr_screen == WelcomeScreen) {
                        if (display.length() > 0 && evnt.text.unicode == 13) {
                            curr_screen = GameState::GameScreen;
                            clock.restart();
                        }

                        if ((evnt.text.unicode >= 65 && evnt.text.unicode <= 90) ||
                            (evnt.text.unicode >= 97 && evnt.text.unicode <= 122)) {
                            display += static_cast<char>(evnt.text.unicode);
                            }
                        else if (evnt.text.unicode == 8) {
                            if (!display.empty()) {
                                display = display.substr(0, display.length() - 1);
                            }
                        }
                        else if (evnt.text.unicode == 32) {
                            display += " ";
                        }

                        if (display.length() > 10) {
                            display = display.substr(0, 10);
                        }

                        display[0] = toupper(display[0]);
                        for (int i = 1; i < display.length(); i++) {
                            display[i] = tolower(display[i]);
                        }
                        username.setString(display);
                        FloatRect usernameRect = username.getLocalBounds();
                        setText(username, width/2, (height/2) - 45);
                        cursor.setPosition(Vector2f(username.getGlobalBounds().left + username.getGlobalBounds().width + 1, username.getPosition().y - 9));
                        player.name = display;
                    }
                break;
                case Event::MouseButtonPressed:
                    if (curr_screen == GameScreen) {
                        if (Mouse::isButtonPressed(Mouse::Left)) {
                            int tileX = Mouse::getPosition(window).x/32;
                            int tileY = Mouse::getPosition(window).y/32;
                            int onboardX = Mouse::getPosition(window).x;
                            int onboardY = Mouse::getPosition(window).y;
                            if (!board.getTile(tileX, tileY).isFlagged()) {
                                if (board.getTile(tileX, tileY).getTile() == 9) {
                                    board.endGame();
                                    happyFace.setButtonTexture(sad);
                                    gameOver = true;
                                } else if (board.getTile(tileX, tileY).getTile() == 0) {
                                    board.revealEmpty(tileX, tileY);
                                } else {
                                    board.revealTile(tileX, tileY);
                                }
                            }
                            if (happyFace.withinBtnPosition(onboardX, onboardY)) {
                                board.resetBoard();
                                pausedTime = 0;
                                happyFace.setButtonTexture(happy);
                            }
                            if ((gamePaused == false) && wait.withinBtnPosition(onboardX, onboardY)) {
                                gamePaused = true;
                                wait.setButtonTexture(play);
                                pauseStartTime = clock.getElapsedTime();
                            }
                            else if ((gamePaused == true) && wait.withinBtnPosition(onboardX, onboardY)) {
                                gamePaused = false;
                                wait.setButtonTexture(pause);
                                pausedTime += (clock.getElapsedTime() - pauseStartTime).asSeconds();
                            }
                            if (!debuggerPressed && debugger.withinBtnPosition(onboardX, onboardY)) {
                                board.debug();
                                debuggerPressed = true;
                            }
                            else if (debuggerPressed && debugger.withinBtnPosition(onboardX, onboardY)) {
                                board.undebug();
                                debuggerPressed = false;
                            }
                            if (leaderBoard.withinBtnPosition(onboardX, onboardY)) {
                                displayLeaderboard = true;
                            }

                        }
                        else if (Mouse::isButtonPressed(Mouse::Right)) {
                            int tileX = Mouse::getPosition(window).x/32;
                            int tileY = Mouse::getPosition(window).y/32;
                            if (board.getTile(tileX, tileY).isFlagged()==false) {
                                board.flagTile(tileX, tileY);
                                flagCount--;
                            }
                            else if (board.getTile(tileX, tileY).isFlagged()==true) {
                                board.unflagTile(tileX, tileY);
                                flagCount++;
                            }
                        }
                    }
                break;
            }
        }


        if (curr_screen == WelcomeScreen) {
            if (blinkClock.getElapsedTime().asSeconds() >= 0.5f) {
                cursorVisible = !cursorVisible;
                blinkClock.restart();
            }
            setText(username, width/2, (height/2) - 45);
            window.clear(Color::Blue);
            window.draw(text);
            window.draw(name);
            window.draw(username);
            if (cursorVisible) {
                window.draw(cursor);
            }
        } else if (curr_screen == GameScreen) {
            if (!boardfilled) {
                board.fillBoard();
                board.numberBoard();
                boardfilled = true;
            }
            window.clear(Color::White);
            board.drawBoard(window);
            happyFace.setButtonPosition(((colCount/2.0)*32)-32, 32*(rowCount+0.5));
            happyFace.drawButton(window);
            debugger.setButtonPosition((colCount * 32)- 304, 32 * (rowCount + 0.5));
            debugger.drawButton(window);
            wait.setButtonPosition((colCount * 32) - 240, 32 * (rowCount + 0.5));
            wait.drawButton(window);

            drawCounter(window, digit, flagCount, rowCount);

            float minX = (colCount * 32) - 97;
            float secX = (colCount * 32) - 54;
            int time = 0;

            if (gameOver) {
                time = static_cast<int>(pauseStartTime.asSeconds()) - pausedTime+1;
            }
            else if (gamePaused) {
                if (!pauseCaptured) {
                    pauseStartTime = clock.getElapsedTime();
                    pauseCaptured = true;
                }
                time = static_cast<int>(pauseStartTime.asSeconds()) - pausedTime;
            }
            else if (board.checkWin()) {
                if (!gameWon) {
                    finalTime = static_cast<int>(clock.getElapsedTime().asSeconds()) - pausedTime;
                    player.time = chrono::duration<double>(finalTime);

                    gameWon = true;
                    happyFace.setButtonTexture(victory);
                    displayLeaderboard = true;

                    l.readFromFile("files/leaderboard.txt");
                    l.addWinner(player.time, player.name + "*");
                }

                time = finalTime;
            }
            else {
                if (pauseCaptured) {
                    pausedTime += static_cast<int>((clock.getElapsedTime() - pauseStartTime).asSeconds());
                    pauseCaptured = false;
                }
                time = static_cast<int>(clock.getElapsedTime().asSeconds()) - pausedTime;
            }

            int minutes = time / 60;
            int seconds = time % 60;

            drawTwoDigitNumber(window, digit, minutes, minX, 32 * (rowCount + 0.5f) + 16);
            drawTwoDigitNumber(window, digit, seconds, secX, 32 * (rowCount + 0.5f) + 16);

            leaderBoard.setButtonPosition((colCount * 32) - 176, 32 * (rowCount + 0.5));
            leaderBoard.drawButton(window);
            if (displayLeaderboard == true) {
                l.players = l.readFromFile("files/leaderboard.txt");
                string result = l.getLeaderboardText();
                leaderboardDisplay.setString(result);
                cout << "result: " << result << endl;
                RenderWindow leadWindow(VideoMode((rowCount* 16) + 50, (colCount * 16)), "Minesweeper");
                while (leadWindow.isOpen()) {
                    Event event;
                    while (leadWindow.pollEvent(event)) {
                        switch (event.type) {
                            case Event::Closed:
                                leadWindow.close();
                                displayLeaderboard = false;
                                break;
                        }
                    }
                    leadWindow.clear(Color::Blue);
                    leadWindow.draw(leadTitle);
                    setText(leaderboardDisplay, (rowCount* 16 + 50)/2, (colCount * 16)/2 +20);
                    leadWindow.draw(leaderboardDisplay);
                    leadWindow.display();
                }
            }
        }
        window.display();
    } return 0;
}


