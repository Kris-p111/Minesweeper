//#include <SFML/Graphics.hpp>
//#include <time.h>
//using namespace sf;
//
//int main()
//{
//    srand(time(0));
//
//    RenderWindow app(VideoMode(400, 400), "Minesweeper!");
//
//    int w=32;
//    int grid[12][12];
//    int sgrid[12][12]; //for showing
//
//    Texture t;
//    t.loadFromFile("files/images/tile_hidden.png");
//    Sprite s(t);
//
//    for (int i=1;i<=10;i++)
//        for (int j=1;j<=10;j++)
//        {
//            sgrid[i][j]=10;
//            if (rand()%5==0)  grid[i][j]=9;
//            else grid[i][j]=0;
//        }
//
//    for (int i=1;i<=10;i++)
//        for (int j=1;j<=10;j++)
//        {
//            int n=0;
//            if (grid[i][j]==9) continue;
//            if (grid[i+1][j]==9) n++;
//            if (grid[i][j+1]==9) n++;
//            if (grid[i-1][j]==9) n++;
//            if (grid[i][j-1]==9) n++;
//            if (grid[i+1][j+1]==9) n++;
//            if (grid[i-1][j-1]==9) n++;
//            if (grid[i-1][j+1]==9) n++;
//            if (grid[i+1][j-1]==9) n++;
//            grid[i][j]=n;
//        }
//
//    while (app.isOpen())
//    {
//        Vector2i pos = Mouse::getPosition(app);
//        int x = pos.x/w;
//        int y = pos.y/w;
//
//        Event e;
//        while (app.pollEvent(e))
//        {
//            if (e.type == Event::Closed)
//                app.close();
//
//            if (e.type == Event::MouseButtonPressed)
//                if (e.key.code == Mouse::Left) sgrid[x][y]=grid[x][y];
//                else if (e.key.code == Mouse::Right) sgrid[x][y]=11;
//        }
//
//        app.clear(Color::White);
//
//        for (int i=1;i<=10;i++)
//            for (int j=1;j<=10;j++)
//            {
//                if (sgrid[x][y]==9) sgrid[i][j]=grid[i][j];
//                s.setTextureRect(IntRect(sgrid[i][j]*w,0,w,w));
//                s.setPosition(i*w, j*w);
//                app.draw(s);
//            }
//
//        app.display();
//    }
//
//    return 0;
//}



// class Timer {
//     int min;
//     int sec;
//     vector<Sprite> minSprites;
//     vector<Sprite> secSprites;
//
// public:
//     Timer() : min(0), sec(0) {}
//     Timer(int totalSeconds) {
//         min = totalSeconds / 60;
//         sec = totalSeconds % 60;
//     }
//
//     void setUpSprites(map<int, Texture>& digits) {
//         minSprites.resize(2);
//         secSprites.resize(2);
//
//         updateSprites(digits); // Initial setup
//     }
//
//     void updateTime(int totalSeconds) {
//         min = totalSeconds / 60;
//         sec = totalSeconds % 60;
//     }
//
//     void updateSprites(map<int, Texture>& digits) {
//         int minTens = min / 10;
//         int minOnes = min % 10;
//         int secTens = sec / 10;
//         int secOnes = sec % 10;
//
//         minSprites[0].setTexture(digits[minTens]);
//         minSprites[1].setTexture(digits[minOnes]);
//         secSprites[0].setTexture(digits[secTens]);
//         secSprites[1].setTexture(digits[secOnes]);
//
//         minSprites[0].setPosition(10, 10);
//         minSprites[1].setPosition(30, 10);
//         secSprites[0].setPosition(60, 10);
//         secSprites[1].setPosition(80, 10);
//     }
//
//     vector<Sprite> getSprites() {
//         vector<Sprite> all = minSprites;
//         all.insert(all.end(), secSprites.begin(), secSprites.end());
//         return all;
//     }
// };

// class Leaderboard {
//     int width, height;
//     string filename;
//     vector<Player*> players;
// public:
//     Leaderboard();
//     Leaderboard(int w, int h, string f) : width(w), height(h), filename(f) {}
//     void readFromFile(const string& filename) {
//         ifstream file(filename);
//         if (file.is_open()) {
//             string n;
//             int t;
//             int i = 0;
//             while (file >> n >> t) {
//                 players[i]->name = n;
//                 players[i]->time = static_cast<chrono::duration<double>>(t);
//                 i++;
//             }
//         }
//     }
//     void addWinner(int time, string name) {
//
//     }
//     // void drawBoard() {
//     //     Text text;
//     //     text.setFont(font);
//     //     text.setCharacterSize(20);
//     //     text.setFillColor(Color::White);
//     //     text.setPosition(50, 50);
//     //     text.setString(createWinnerText());
//     //     window.draw(text);
//     // }
// };