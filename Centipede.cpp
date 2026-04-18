#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <ctime>
#include <string>  

using namespace std;

// Screen dimensions
const int resolutionX = 960;
const int resolutionY = 960;
const int boxPixelsX = 32;
const int boxPixelsY = 32;
const int gameRows = resolutionX / boxPixelsX;    // Total rows on grid
const int gameColumns = resolutionY / boxPixelsY; // Total columns on grid

// Grid and other constants
int gameGrid[gameRows][gameColumns] = {};
const int x = 0;
const int y = 1;
const int existsFlag = 2;

// For centipede
static bool exist[12] = { true, true, true, true, true, true, true, true, true, true, true, true };

// Mark which segments are heads of each sub-centipede
static bool isHead[12] = { true, false, false, false, false, false, false, false, false, false, false, false };
// Direction array for each head (true = left, false = right)
static bool moveLeft[12] = { true, false, false, false, false, false, false, false, false, false, false, false };

// Scoring
int score = 0;

// Global variable to hold the reason for loss
string lossReason = "";

// Game states
enum GameState { MENU, PLAYING, GAMEOVER };

// Function Declarations
void PlayerInput(float player[], float movementSpeed);
void BulletInput(float player[], float bullet[], sf::Clock& bulletClock);
void drawPlayer(sf::RenderWindow& window, float player[], sf::Sprite& playerSprite);
void moveBullet(float bullet[], sf::Clock& bulletClock);
void drawBullet(sf::RenderWindow& window, float bullet[], sf::Sprite& bulletSprite);
void drawMushroom(sf::RenderWindow& window, sf::Sprite& mushroomSprite, sf::Sprite& dmushroomSprite,
    sf::Sprite& pmushroomSprite, sf::Sprite& dpmushroomSprite);
void drawCentipede(sf::RenderWindow& window, float centipede[12][2], sf::Sprite& centipedeSprite,
    sf::Sprite& hcentipedeSprite);
void centipedemovement(float centipede[12][2], float movementspeed);
void centibullet(float centipede[12][2], float bullet[]);
bool checkLoss(float player[], float centipede[12][2]);
bool checkWin(float centipede[12][2]);

bool within(float bx, float by, float cx, float cy, float w, float h)
{
    return (bx >= cx && bx <= cx + w && by >= cy && by <= cy + h);
}

int main()
{
    srand(static_cast<unsigned int>(time(0)));

    // Create the window
    sf::RenderWindow window(sf::VideoMode(resolutionX, resolutionY), "Centipede by MTQ", sf::Style::Close | sf::Style::Titlebar);
    window.setPosition(sf::Vector2i(100, 0));

    // Load background
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    if (!backgroundTexture.loadFromFile("Textures/background.png")) {
        cerr << "Error loading background texture" << endl;
        return 1;
    }
    backgroundSprite.setTexture(backgroundTexture);
    backgroundSprite.setColor(sf::Color(255, 255, 255, 51)); // ~20% alpha

    // Load font (for score, menus, etc.)
    sf::Font font;
    if (!font.loadFromFile("fonts/arial.ttf")) {
        cerr << "Error loading font" << endl;
        return 1;
    }

    // Score display
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);

    // Background tab behind score
    sf::RectangleShape scoreTab(sf::Vector2f(static_cast<float>(resolutionX), 40.f));
    scoreTab.setFillColor(sf::Color(0, 0, 0, 150));

    // Menu text
    sf::Text menuText("Click or Press Enter to Start", font, 40);
    menuText.setFillColor(sf::Color::White);

    // End-game text
    sf::Text gameOverText("", font, 40);
    gameOverText.setFillColor(sf::Color::White);

    // Center text helper
    auto centerText = [&](sf::Text& txt, float posY)
        {
            sf::FloatRect tRect = txt.getLocalBounds();
            txt.setOrigin(tRect.left + tRect.width * 0.5f, tRect.top + tRect.height * 0.5f);
            txt.setPosition(static_cast<float>(resolutionX) * 0.5f, posY);
        };

    // Set initial positions for each text
    centerText(menuText, 400.f);
    centerText(gameOverText, 400.f);

    // Player setup
    float player[2] = {
        (static_cast<float>(gameColumns) / 2.f) * boxPixelsX,
        (static_cast<float>(gameColumns) * 3.f / 4.f) * boxPixelsY
    };
    sf::Texture playerTexture;
    sf::Sprite playerSprite;
    if (!playerTexture.loadFromFile("Textures/player.png")) {
        cerr << "Error loading player texture" << endl;
        return 1;
    }
    playerSprite.setTexture(playerTexture);
    playerSprite.setTextureRect(sf::IntRect(0, 0, boxPixelsX, boxPixelsY));
    float movementSpeed = 0.5f;

    // Bullet setup
    float bullet[3] = { 0.f, 0.f, false };
    sf::Clock bulletClock;
    sf::Texture bulletTexture;
    sf::Sprite bulletSprite;
    if (!bulletTexture.loadFromFile("Textures/bullet.png")) {
        cerr << "Error loading bullet texture" << endl;
        return 1;
    }
    bulletSprite.setTexture(bulletTexture);
    bulletSprite.setTextureRect(sf::IntRect(0, 0, boxPixelsX, boxPixelsY));

    // ------------------ AUDIO SETUP ------------------
    sf::Music gameMusic;
    if (!gameMusic.openFromFile("Music/field_of_hopes.ogg")) {
        cerr << "Error loading audio" << endl;
    }
    gameMusic.setLoop(true);
    // Note: We do not start playing here. Music is controlled based on game state.
    // -------------------------------------------------

    // Centipede setup
    float centipede[12][2];

    // Random side from top (below 40px score tab)
    // Random X within gameplay
    float randomX = static_cast<float>(rand() % (gameRows - 1)) * boxPixelsX;
    centipede[0][x] = randomX;
    centipede[0][y] = 40.0f;

    // Random direction for the head: true = left, false = right
    moveLeft[0] = (rand() % 2 == 0);

    // Place body behind the head
    for (int i = 1; i < 12; i++) {
        if (moveLeft[0]) {
            // Body extends to the right
            centipede[i][x] = centipede[i - 1][x] + boxPixelsX;
        }
        else {
            // Body extends to the left
            centipede[i][x] = centipede[i - 1][x] - boxPixelsX;
        }
        centipede[i][y] = centipede[0][y];
    }

    sf::Texture centipedeTexture;
    sf::Sprite centipedeSprite;
    if (!centipedeTexture.loadFromFile("Textures/centipede.png")) {
        cerr << "Error loading centipede texture" << endl;
        return 1;
    }
    centipedeSprite.setTexture(centipedeTexture);
    centipedeSprite.setTextureRect(sf::IntRect(0, 0, boxPixelsX, boxPixelsY));
    float movementspeed = 0.08f; // Slower centipede for easier testing

    // Centipede head
    sf::Texture hcentipedeTexture;
    sf::Sprite hcentipedeSprite;
    if (!hcentipedeTexture.loadFromFile("Textures/head.png")) {
        cerr << "Error loading centipede head texture" << endl;
        return 1;
    }
    hcentipedeSprite.setTexture(hcentipedeTexture);
    hcentipedeSprite.setTextureRect(sf::IntRect(0, 0, boxPixelsX, boxPixelsY));

    // Mushroom data
    sf::Texture mushroomTextureObj, dmushroomTextureObj, pmushroomTextureObj, dpmushroomTextureObj;
    sf::Sprite mushroomSprite, dmushroomSprite, pmushroomSprite, dpmushroomSprite;
    if (!mushroomTextureObj.loadFromFile("Textures/mushroom.png")) {
        cerr << "Error loading mushroom texture" << endl;
        return 1;
    }
    if (!dmushroomTextureObj.loadFromFile("Textures/mushroom.png")) {
        cerr << "Error loading damaged mushroom texture" << endl;
        return 1;
    }
    if (!pmushroomTextureObj.loadFromFile("Textures/mushroom.png")) {
        cerr << "Error loading poisonous mushroom texture" << endl;
        return 1;
    }
    if (!dpmushroomTextureObj.loadFromFile("Textures/mushroom.png")) {
        cerr << "Error loading damaged poisonous mushroom texture" << endl;
        return 1;
    }
    mushroomSprite.setTexture(mushroomTextureObj);
    dmushroomSprite.setTexture(dmushroomTextureObj);
    pmushroomSprite.setTexture(pmushroomTextureObj);
    dpmushroomSprite.setTexture(dpmushroomTextureObj);

    mushroomSprite.setTextureRect(sf::IntRect(0, 0, boxPixelsX, boxPixelsY));
    dmushroomSprite.setTextureRect(sf::IntRect(64, 0, boxPixelsX, boxPixelsY));
    pmushroomSprite.setTextureRect(sf::IntRect(0, 32, boxPixelsX, boxPixelsY));
    dpmushroomSprite.setTextureRect(sf::IntRect(64, 32, boxPixelsX, boxPixelsY));

    // Random mushrooms, skipping row 0
    {
        int mcount = 20 + (rand() % 10);
        for (int i = 0; i < mcount; i++) {
            int m = (rand() % (gameRows - 1)) + 1; // ensures row is 1..(gameRows-1)
            int n = rand() % (gameColumns - 5) + 1;
            gameGrid[m][n] = 10;
        }
    }

    // Game state
    GameState currentState = MENU;
    bool isWin = false;

    while (window.isOpen())
    {
        sf::Event e;
        while (window.pollEvent(e))
        {
            if (e.type == sf::Event::Closed) {
                window.close();
            }
            if (currentState == MENU) {
                if ((e.type == sf::Event::MouseButtonPressed) ||
                    (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Enter)) {
                    currentState = PLAYING;
                }
            }
        }

        // Control music based on game state.
        if (currentState == PLAYING) {
            if (gameMusic.getStatus() != sf::Music::Playing)
                gameMusic.play();
        }
        else {
            if (gameMusic.getStatus() == sf::Music::Playing)
                gameMusic.stop();
        }

        window.clear();
        window.draw(backgroundSprite);

        if (currentState == MENU) {
            centerText(menuText, resolutionY / 2.f);
            window.draw(menuText);
        }
        else if (currentState == PLAYING) {
            scoreTab.setPosition(0.f, 0.f);
            window.draw(scoreTab);

            scoreText.setString("Score: " + to_string(score));
            centerText(scoreText, 20.f);
            window.draw(scoreText);

            if (checkLoss(player, centipede)) {
                currentState = GAMEOVER;
                isWin = false;
            }
            if (checkWin(centipede)) {
                currentState = GAMEOVER;
                isWin = true;
            }

            // Draw game objects
            drawPlayer(window, player, playerSprite);
            drawMushroom(window, mushroomSprite, dmushroomSprite, pmushroomSprite, dpmushroomSprite);

            // Move centipede & bullet
            centipedemovement(centipede, movementspeed);
            centibullet(centipede, bullet);

            drawCentipede(window, centipede, centipedeSprite, hcentipedeSprite);

            // Move bullet if active
            if (bullet[existsFlag]) {
                moveBullet(bullet, bulletClock);
                drawBullet(window, bullet, bulletSprite);
            }

            // Capture input
            PlayerInput(player, movementSpeed);
            BulletInput(player, bullet, bulletClock);
        }
        else if (currentState == GAMEOVER) {
            // Display final score along with loss reason if lost
            if (isWin) {
                gameOverText.setString("You won!\nScore: " + to_string(score));
            }
            else {
                gameOverText.setString("Game Over - " + lossReason + "\nScore: " + to_string(score));
            }
            centerText(gameOverText, resolutionY / 2.f);
            window.draw(gameOverText);
        }

        window.display();
    }

    return 0;
}

//////////////////////////////////////////////////////////
//               SUPPORTING FUNCTIONS                   //
//////////////////////////////////////////////////////////

bool checkLoss(float player[], float centipede[12][2])
{
    // Check if player collides with any poisonous mushroom
    int px = static_cast<int>(player[x] / boxPixelsX);
    int py = static_cast<int>(player[y] / boxPixelsY);
    if (px >= 0 && px < gameRows && py >= 0 && py < gameColumns) {
        if (gameGrid[px][py] == 30 || gameGrid[px][py] == 40) {
            lossReason = "Player touched poisonous mushroom!";
            cout << "Game Over - " << lossReason << "\n";
            return true;
        }
    }

    // Collision with any centipede head
    for (int i = 0; i < 12; i++) {
        if (exist[i] && isHead[i]) {
            int headX = static_cast<int>(centipede[i][x] / boxPixelsX);
            int headY = static_cast<int>(centipede[i][y] / boxPixelsY);
            if (px == headX && py == headY) {
                lossReason = "Player touched the centipede";
                cout << "Game Over - " << lossReason << "\n";
                return true;
            }
        }
    }

    return false;
}

bool checkWin(float centipede[12][2])
{
    for (int i = 0; i < 12; i++) {
        if (exist[i]) return false;
    }
    cout << "All centipede segments destroyed!\n";
    return true;
}

void centipedemovement(float centipede[12][2], float movementspeed)
{
    // Each "head" drives a sub-centipede chain.
    static float delayCounter[12] = { 0.f };
    static float oldPos[12][2];

    for (int i = 0; i < 12; i++) {
        oldPos[i][x] = centipede[i][x];
        oldPos[i][y] = centipede[i][y];
    }

    // Move each head
    for (int i = 0; i < 12; i++) {
        if (!exist[i] || !isHead[i]) continue;

        delayCounter[i] += movementspeed;
        if (delayCounter[i] < 4.f) continue; // Pace movement
        delayCounter[i] = 0.f;

        // Move left or right
        if (moveLeft[i]) centipede[i][x] -= boxPixelsX;
        else centipede[i][x] += boxPixelsX;

        // Check if we need to bounce or drop down
        int hx = static_cast<int>(centipede[i][x] / boxPixelsX);
        int hy = static_cast<int>(centipede[i][y] / boxPixelsY);
        if (centipede[i][x] <= 0.f
            || centipede[i][x] >= (resolutionX - boxPixelsX)
            || (hx >= 0 && hx < gameRows && hy >= 1 && hy < gameColumns
                && (gameGrid[hx][hy] == 10 || gameGrid[hx][hy] == 20)))
        {
            moveLeft[i] = !moveLeft[i];
            centipede[i][y] += boxPixelsY;
        }

        // Shift body behind head
        int prevSeg = i;
        for (int seg = i + 1; seg < 12; seg++) {
            if (!exist[seg] || isHead[seg]) break;
            centipede[seg][x] = oldPos[prevSeg][x];
            centipede[seg][y] = oldPos[prevSeg][y];
            prevSeg = seg;
        }
    }
}

void centibullet(float centipede[12][2], float bullet[])
{
    if (!bullet[existsFlag]) return;

    for (int i = 0; i < 12; i++) {
        if (!exist[i]) continue;

        if (within(bullet[x], bullet[y], centipede[i][x], centipede[i][y], boxPixelsX, boxPixelsY)) {
            // Bullet hits segment i
            bullet[existsFlag] = false;

            // Head destroyed
            if (i == 0 && isHead[0]) {
                exist[0] = false;
                isHead[0] = false;
                score += 20; // Head worth more

                for (int j = 1; j < 12; j++) {
                    if (exist[j]) {
                        isHead[j] = true;
                        moveLeft[j] = moveLeft[0];
                        centipede[j][x] = centipede[0][x];
                        centipede[j][y] = centipede[0][y];
                        break;
                    }
                }
            }
            else {
                // Body destroyed
                exist[i] = false;
                isHead[i] = false;
                score += 10;

                // Place mushroom in the destroyed segment's location
                int sx = static_cast<int>(centipede[i][x] / boxPixelsX);
                int sy = static_cast<int>(centipede[i][y] / boxPixelsY);

                if (sx >= 0 && sx < gameRows && sy >= 0 && sy < gameColumns) {
                    // If segment is in player's zone (y >= 800), create or damage a poisonous mushroom
                    if (centipede[i][y] >= 800.f) {
                        if (gameGrid[sx][sy] == 30) gameGrid[sx][sy] = 40; // Damaged poisonous
                        else gameGrid[sx][sy] = 30;                       // Poisonous
                    }
                    else {
                        // Normal or damaged standard mushroom
                        if (gameGrid[sx][sy] == 30) gameGrid[sx][sy] = 40;
                        else gameGrid[sx][sy] = 10;
                    }
                }

                // Make next segment a new head with reversed direction
                if (i < 11 && exist[i + 1]) {
                    isHead[i + 1] = true;
                    moveLeft[i + 1] = !moveLeft[i + 1];
                    centipede[i + 1][x] = centipede[i][x];
                    centipede[i + 1][y] = centipede[i][y];
                }
            }
            break;
        }
    }
}

void drawCentipede(sf::RenderWindow& window, float centipede[12][2], sf::Sprite& centipedeSprite,
    sf::Sprite& hcentipedeSprite)
{
    for (int i = 0; i < 12; i++) {
        if (!exist[i]) continue;
        if (isHead[i]) {
            hcentipedeSprite.setPosition(centipede[i][x], centipede[i][y]);
            window.draw(hcentipedeSprite);
        }
        else {
            centipedeSprite.setPosition(centipede[i][x], centipede[i][y]);
            window.draw(centipedeSprite);
        }
    }
}

void drawMushroom(sf::RenderWindow& window, sf::Sprite& mushroomSprite, sf::Sprite& dmushroomSprite,
    sf::Sprite& pmushroomSprite, sf::Sprite& dpmushroomSprite)
{
    for (int m = 0; m < gameRows; m++) {
        for (int n = 0; n < gameColumns; n++) {
            float posX = static_cast<float>(m * boxPixelsX);
            float posY = static_cast<float>(n * boxPixelsY);

            if (gameGrid[m][n] == 10) {
                mushroomSprite.setPosition(posX, posY);
                window.draw(mushroomSprite);
            }
            else if (gameGrid[m][n] == 20) {
                dmushroomSprite.setPosition(posX, posY);
                window.draw(dmushroomSprite);
            }
            else if (gameGrid[m][n] == 30) {
                pmushroomSprite.setPosition(posX, posY);
                window.draw(pmushroomSprite);
            }
            else if (gameGrid[m][n] == 40) {
                dpmushroomSprite.setPosition(posX, posY);
                window.draw(dpmushroomSprite);
            }
        }
    }
}

void drawPlayer(sf::RenderWindow& window, float player[], sf::Sprite& playerSprite)
{
    playerSprite.setPosition(player[x], player[y]);
    window.draw(playerSprite);
}

void moveBullet(float bullet[], sf::Clock& bulletClock)
{
    if (bulletClock.getElapsedTime().asMilliseconds() < 3) return;
    bulletClock.restart();

    bullet[y] -= 5;

    // Collide with mushrooms
    int bx = static_cast<int>(bullet[x] / boxPixelsX);
    int by = static_cast<int>(bullet[y] / boxPixelsY);
    if (bx >= 0 && bx < gameRows && by >= 0 && by < gameColumns) {
        if (gameGrid[bx][by] == 10) {
            bullet[existsFlag] = false;
            gameGrid[bx][by] = 20; // Damaged
        }
        else if (gameGrid[bx][by] == 20) {
            bullet[existsFlag] = false;
            gameGrid[bx][by] = 0;  // Destroyed
            score += 1;            // Destroying a mushroom yields 1 point
        }
    }
    // Off-screen
    if (bullet[y] < -boxPixelsY) {
        bullet[existsFlag] = false;
    }
}

void drawBullet(sf::RenderWindow& window, float bullet[], sf::Sprite& bulletSprite)
{
    bulletSprite.setPosition(bullet[x], bullet[y]);
    window.draw(bulletSprite);
}

void PlayerInput(float player[], float movementSpeed)
{
    float newX = player[x], newY = player[y];

    // Proposed movement
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && player[x] > 0)
        newX -= movementSpeed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && player[x] < (960 - 32))
        newX += movementSpeed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && player[y] > 800)
        newY -= movementSpeed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && player[y] < (960 - 32))
        newY += movementSpeed;

    // Check corners for mushrooms
    auto canMove = [&](float px, float py)
        {
            int tileX = static_cast<int>(px / boxPixelsX);
            int tileY = static_cast<int>(py / boxPixelsY);
            if (tileX < 0 || tileX >= gameRows || tileY < 0 || tileY >= gameColumns) {
                return false;
            }
            int tileVal = gameGrid[tileX][tileY];
            return (tileVal != 10 && tileVal != 20 && tileVal != 30 && tileVal != 40);
        };

    float leftX = newX;
    float rightX = newX + boxPixelsX - 1;
    float topY = newY;
    float bottomY = newY + boxPixelsY - 1;

    if (canMove(leftX, topY) && canMove(rightX, topY)
        && canMove(leftX, bottomY) && canMove(rightX, bottomY))
    {
        player[x] = newX;
        player[y] = newY;
    }
}

void BulletInput(float player[], float bullet[], sf::Clock& bulletClock)
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && !bullet[existsFlag]) {
        bullet[x] = player[x];
        bullet[y] = player[y] - boxPixelsY;
        bullet[existsFlag] = true;
        bulletClock.restart();
    }
}