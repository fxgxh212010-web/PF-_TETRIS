
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <ctime>       //for time
#include <cstdlib>     //for rand and srand
#include <optional>    //for sfml

using namespace std;

const int rows = 20;
const int cols = 10;
const float board_x = 20.f;
const float board_y = 20.f;
const float cell = 30.f;

int board[rows][cols];              //2d array for the tetris board

char pieces[7][4][4];               //stores all 7 pieces
char activepiece[4][4];             //stores the currently falling piece

int currpiece = 0;                  //index of the current piece

int piece_x = 3;                    //horizontal position
int piece_y = 0;                    //vertical position

int falltime = 0;                   //counter for automatic falling
 int fallspeed = 30;           //frames before falling

bool gameover = false;              //checks whether the game has ended

int score = 0;
int totalLines = 0;
int level = 1;
int nextpiece = -1;
void updateLevelAndSpeed(int cleared);
bool loadboard()                    //loads the game board
{
    ifstream file("../../../../src/board.txt");

    if (!file.is_open())
    {
        cout << "Board failed to load!";
        return false;
    }

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (!(file >> board[i][j]))
            {
                cout << "Board file has incorrect data!";
                return false;
            }
        }
    }

    file.close();
    return true;
}


bool loadpieces()                   //loads all 7 tetris pieces
{
    ifstream file("../../../../src/pieces.txt");

    if (!file.is_open())
    {
        cout << "Pieces failed to load!";
        return false;
    }

    for (int p = 0; p < 7; p++)
    {
        for (int r = 0; r < 4; r++)
        {
            for (int c = 0; c < 4; c++)
            {
                if (!(file >> pieces[p][r][c]))
                {
                    cout << "Pieces file has incorrect data!";
                    return false;
                }
            }
        }
    }

    file.close();
    return true;
}


bool canmove(int newrow, int newcol)    //checks whether the piece can move
{
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            if (activepiece[r][c] == '1')   //checks actual blocks only
            {
                int boardr = newrow + r;    //finds the new board row
                int boardc = newcol + c;    //finds the new board column

                if (boardr >= rows || boardc < 0 || boardc >= cols)
                    return false;           //checks board boundaries

                if (boardr >= 0 && board[boardr][boardc] != 0)
                    return false;           //checks collision with another block
            }
        }
    }

    return true;                            //piece can move
}


void lockpiece()                           //locks the piece in its current position
{
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            if (activepiece[r][c] == '1')
            {
                int boardr = piece_y + r;
                int boardc = piece_x + c;

                if (boardr >= 0 && boardr < rows && boardc >= 0 && boardc < cols)
                {
                    board[boardr][boardc] = 1;  //stores the piece permanently
                }
            }
        }
    }
}


void clearline()                            //clears completely filled rows
{
    int cleared = 0;
    for (int r = rows - 1; r >= 0; r--)
    {
        bool full = true;                   //assumes the row is completely filled

        for (int c = 0; c < cols; c++)
        {
            if (board[r][c] == 0)
            {
                full = false;               //row contains an empty cell
                break;
            }
        }

        if (full)
        {
            for (int move = r; move > 0; move--)
            {
                for (int c = 0; c < cols; c++)
                {
                    board[move][c] = board[move - 1][c]; //moves rows downward
                }
            }

            for (int c = 0; c < cols; c++)
            {
                board[0][c] = 0;            //clears the top row
            }

            r++;     
            cleared++;//checks the same row again
        }
    }

}


void spawnpiece()                           //spawns a random tetris piece
{
    currpiece = rand() % 7;                 //selects a random piece from 0 to 6
    if (nextpiece == -1)
        nextpiece = rand() % 7;
    currpiece = nextpiece;
    nextpiece = rand() % 7;
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            activepiece[r][c] = pieces[currpiece][r][c]; //copies the selected piece
        }
    }

    piece_x = 3;                            //places the piece at the top
    piece_y = 0;

    if (!canmove(piece_y, piece_x))
    {
        gameover = true;                    //ends the game if the piece cannot fit
        cout << "GAME OVER!" << endl;
    }
}


void updatefalling()                        //controls automatic falling
{
    falltime++;                             //increases the falling counter

    if (falltime >= fallspeed)              //checks whether enough frames have passed
    {
        falltime = 0;                       //resets the falling counter

        if (canmove(piece_y + 1, piece_x))
        {
            piece_y++;                      //moves the piece down one row
        }
        else
        {
            lockpiece();                    //stores the piece on the board
            clearline();                    //clears completed rows
            spawnpiece();                   //spawns the next random piece
        }
    }
}


void harddrop()                             //drops the piece directly to the bottom
{
    while (canmove(piece_y + 1, piece_x))
    {
        piece_y++;                          //moves the piece down until it cannot move
    }

    lockpiece();                            //locks the piece in its final position
    clearline();                            //clears completed rows
    spawnpiece();                           //creates the next random piece

    falltime = 0;                           //resets the falling timer
}


void drawboard(sf::RenderWindow& window, sf::RectangleShape& block, sf::Color colors[]) //draws the game board
{
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            if (board[r][c] == 0)
                block.setFillColor(colors[0]);
            else
                block.setFillColor(colors[1]);

            block.setPosition({
                board_x + c * cell + 1,
                board_y + r * cell + 1
            });

            window.draw(block);
        }
    }
}

bool isValidMove(int grid[rows][cols], int piece[4][4], int targetX, int targetY) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (piece[i][j] != 0) {
                int boardX = targetX + j;
                int boardY = targetY + i;

                if (boardX < 0 || boardX >= cols || boardY >= rows) {
                    return false;
                }

                if (boardY >= 0 && grid[boardY][boardX] != 0) {
                    return false;
                }
            }
        }
    }
    return true;
}

void rotatePiece(int grid[rows][cols], int piece[4][4], int &pieceX, int pieceY, int shapetype) {
    if (shapetype == 1) {
        return;
    }

    int temp[4][4] = {0};

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp[j][3 - i] = piece[i][j];
        }
    }

    int kickOffsets[3] = {0, -1, 1};

    for (int k = 0; k < 3; k++) {
        int testX = pieceX + kickOffsets[k];

        if (isValidMove(grid, temp, testX, pieceY)) {
            pieceX = testX;

            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    piece[i][j] = temp[i][j];
                }
            }
            return;
        }
    }
}

void drawpiece(sf::RenderWindow& window, sf::RectangleShape& block, sf::Color colors[]) //draws the currently falling piece
{
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            if (activepiece[r][c] == '1')
            {
                block.setFillColor(colors[1]);

                block.setPosition({
                    board_x + (piece_x + c) * cell + 1,
                    board_y + (piece_y + r) * cell + 1
                });

                window.draw(block);
            }
        }
    }
}
void updateLevelAndSpeed(int cleared)
{
    if (cleared > 0)
    {
        totalLines += cleared;
        score += cleared * 100 * level;
        level = (totalLines / 10) + 1;

        fallspeed = 30 - (level - 1) * 5;
        if (fallspeed < 5)
            fallspeed = 5;
    }
}

void drawSidePanel(sf::RenderWindow& window, sf::Font& font, sf::RectangleShape& block, sf::Color colors[])
{
    sf::Text info(font);
    info.setCharacterSize(20);
    info.setFillColor(sf::Color::White);

    info.setString("SCORE\n" + to_string(score) +
        "\n\nLINES\n" + to_string(totalLines) +
        "\n\nLEVEL\n" + to_string(level) +
        "\n\nNEXT");
    info.setPosition({ 350.f, 30.f });
    window.draw(info);

    // Draw next piece preview
    if (nextpiece >= 0 && nextpiece < 7)
    {
        for (int r = 0; r < 4; r++)
        {
            for (int c = 0; c < 4; c++)
            {
                if (pieces[nextpiece][r][c] == '1')
                {
                    block.setFillColor(colors[1]);
                    block.setPosition({ 350.f + c * cell, 280.f + r * cell });
                    window.draw(block);
                }
            }
        }
    }

    // Show GAME OVER text when the game ends
    if (gameover)  //checks is game end
    
    {
        sf::Text over(font);   //creates  game over message
        over.setCharacterSize(26);  //sets a larger font
        over.setFillColor(sf::Color::Red);  //game over in red
        over.setString("GAME OVER");   //sets the message text
        over.setPosition({ 340.f, 450.f });
        window.draw(over);
    }
}

int main()
{
    if (!loadboard())                       //loads the game board
        return -1;

    if (!loadpieces())                      //loads all 7 tetris pieces
        return -1;

    srand(static_cast<unsigned int>(time(0))); //initializes random piece selection

    spawnpiece();                           //spawns the first random piece

    sf::RenderWindow window(sf::VideoMode({520, 640}), "Tetris");
    window.setFramerateLimit(60);

    sf::RectangleShape block(sf::Vector2f(26, 26));
    block.setOutlineThickness(1);
    block.setOutlineColor(sf::Color(100, 100, 120));

    sf::Color colors[] = {
        sf::Color(35, 35, 55),              //empty cells
        sf::Color::Cyan                     //filled blocks
    };
    sf::Font font;  //creates a font 
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf"))  //loads the Arial font 
        cout << "Font failed to load!" << endl;  //prints an error if the font cannot be loaded
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) //checks whether a key was pressed
            {
                if (!gameover)
                {
                    if (key->code == sf::Keyboard::Key::Left) //moves the piece left
                    {
                        if (canmove(piece_y, piece_x - 1))
                            piece_x--;
                    }

                    if (key->code == sf::Keyboard::Key::Right) //moves the piece right
                    {
                        if (canmove(piece_y, piece_x + 1))
                            piece_x++;
                    }

                    if (key->code == sf::Keyboard::Key::Down) //moves the piece down
                    {
                        if (canmove(piece_y + 1, piece_x))
                            piece_y++;
                    }

                    if (key->code == sf::Keyboard::Key::Space) //drops the piece straight down
                    {
                        harddrop();
                    }
                }
            }
        }

        if (!gameover)
            updatefalling();                 //updates the falling piece

        window.clear(sf::Color(25, 25, 45));

        drawboard(window, block, colors);    //draws the board

        if (!gameover)
            drawpiece(window, block, colors); //draws the active piece
        drawSidePanel(window, font, block, colors); //draws the side panel every frame
        window.display();
    }

    return 0;
}
