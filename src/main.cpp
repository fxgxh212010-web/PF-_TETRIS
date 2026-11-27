#include <iostream>                 //for input and output
#include <fstream>                  //for reading and writing files
#include <cstdlib>                  //for rand and srand
#include <ctime>                    //for time
#include <optional>                 //for sfml event handling
#include <SFML/Graphics.hpp>        //for sfml graphics

using namespace std;

const int rows = 20;                //number of board rows
const int cols = 10;                //number of board columns
const float cell = 30.f;            //size of each board cell
const float board_x = 20.f;         //horizontal board position
const float board_y = 20.f;         //vertical board position

int board[rows][cols];              //2d array for the tetris board

char pieces[7][4][4];               //stores all 7 tetris pieces
char activepiece[4][4];             //stores the currently falling piece

int currentpiece = 0;               //index of the current piece
int score = 0;                      //stores the player's score
int level = 1;                      //stores the current game level
int linescleared = 0;               //stores the total cleared lines
int framecount = 0;                 //counts frames for automatic falling

bool gameover = false;              //checks whether the game has ended
bool paused = false;                //checks whether the game is paused

int piece_x = 3;                    //horizontal position of the active piece
int piece_y = 0;                    //vertical position of the active piece

bool loadboard();                   //loads the game board
void harddrop();                    //drops the piece directly down
void lockpiece();                   //locks the piece in its current position
void clearlines();                  //clears completely filled rows
void spawnpiece();                  //spawns a new random piece
void resetgame(sf::Clock& clock);   //resets the game
void savegame();                    //saves the current game
bool loadgame();                    //loads a previously saved game
bool canmove(int dx, int dy);       //checks whether the piece can move


bool loadboard()                    //loads the game board from the file
{
    ifstream file("../../../src/board.txt"); //opens the board file
    if (!file)
        file.open("../../../board.txt");

    if (!file)                      //checks whether the file opened successfully
    {
        cout << "Could not open board.txt" << endl; //displays an error message
        return false;               //stops loading if the file is unavailable
    }

    for (int r = 0; r < rows; r++)   //loops through all board rows
    {
        for (int c = 0; c < cols; c++) //loops through all board columns
        {
            file >> board[r][c];    //loads each cell value into the board
        }
    }

    file.close();                   //closes the board file
    return true;                    //confirms successful loading
}


void resetgame(sf::Clock& clock)    //resets the game for a new session
{
    if (!loadboard())               //loads the empty board from the file
        return;                     //stops resetting if loading fails

    score = 0;                      //resets the player's score
    level = 1;                      //resets the game level
    linescleared = 0;               //resets the cleared line counter

    gameover = false;               //removes the game over state
    paused = false;                 //removes the paused state

    spawnpiece();                   //creates a new falling piece

    clock.restart();                //resets the falling timer
    framecount = 0;                 //resets the frame counter
}


void savegame()                     //saves the current game progress
{
    ofstream file("savegame.txt");  //opens the save file

    if (!file)                      //checks whether the file opened successfully
    {
        cout << "Could not save game!" << endl; //displays an error message
        return;                     //stops saving if the file is unavailable
    }

    file << score << " " << level << endl; //saves the score and level
    file << linescleared << " " << currentpiece << endl; //saves lines and piece index

    for (int r = 0; r < rows; r++)  //loops through all board rows
    {
        for (int c = 0; c < cols; c++) //loops through all board columns
        {
            file << board[r][c] << " "; //saves each board cell
        }

        file << endl;               //moves to the next line in the save file
    }

    file.close();                   //closes the save file
}


void clearsave()                    //clears the previously saved game
{
    ofstream file("savegame.txt");  //opens the save file in overwrite mode
    file.close();                   //closes the file after clearing its contents
}


bool loadgame()                     //loads a previously saved game
{
    ifstream file("savegame.txt");  //opens the saved game file

    if (!file)                      //checks whether a save file exists
    {
        cout << "No saved game found!" << endl; //displays an error message
        return false;               //returns false if no save exists
    }

    if (!(file >> score >> level))  //loads the saved score and level
    {
        return false;               //stops loading if the data is incorrect
    }

    if (!(file >> linescleared >> currentpiece)) //loads saved lines and piece index
    {
        return false;               //stops loading if the data is incorrect
    }

    if (currentpiece < 0 || currentpiece >= 7) //checks whether the piece index is valid
    {
        return false;               //rejects an invalid piece index
    }

    for (int r = 0; r < rows; r++)  //loops through all board rows
    {
        for (int c = 0; c < cols; c++) //loops through all board columns
        {
            if (!(file >> board[r][c])) //loads each saved board cell
            {
                return false;       //stops loading if the board data is incomplete
            }
        }
    }

    file.close();                   //closes the save file

    for (int r = 0; r < 4; r++)     //loops through the piece rows
    {
        for (int c = 0; c < 4; c++) //loops through the piece columns
        {
            activepiece[r][c] = pieces[currentpiece][r][c]; //restores the saved piece shape
        }
    }

    piece_x = 3;                    //places the restored piece horizontally
    piece_y = 0;                    //places the restored piece at the top

    gameover = false;               //removes the game over state
    paused = false;                 //removes the paused state
    framecount = 0;                 //resets the falling frame counter

    if (!canmove(0, 0))             //checks whether the restored piece can fit
    {
        gameover = true;            //ends the game if the piece cannot fit
        clearsave();                //clears the invalid saved game
        return false;               //reports that loading failed
    }

    return true;                    //confirms successful loading
}


void initializecolors(sf::Color colors[]) //initializes the colors of the pieces
{
    colors[0] = sf::Color(25, 25, 45); //sets the empty cell color
    colors[1] = sf::Color::Cyan;     //sets the first piece color
    colors[2] = sf::Color::Yellow;   //sets the second piece color
    colors[3] = sf::Color(170, 90, 255); //sets the third piece color
    colors[4] = sf::Color::Green;    //sets the fourth piece color
    colors[5] = sf::Color::Red;      //sets the fifth piece color
    colors[6] = sf::Color::Blue;     //sets the sixth piece color
    colors[7] = sf::Color(255, 150, 0); //sets the seventh piece color
}


void initializeblock(sf::RectangleShape& block) //initializes the size of each block
{
    block.setSize({cell - 2, cell - 2}); //sets the block dimensions
}


bool canmove(int dx, int dy)         //checks whether the piece can move
{
    for (int r = 0; r < 4; r++)      //loops through the piece rows
    {
        for (int c = 0; c < 4; c++) //loops through the piece columns
        {
            if (activepiece[r][c] == '1') //checks whether the current cell contains a block
            {
                int new_x = piece_x + c + dx; //calculates the new horizontal position
                int new_y = piece_y + r + dy; //calculates the new vertical position

                if (new_x < 0 || new_x >= cols || new_y >= rows) //checks board boundaries
                    return false;       //prevents movement outside the board

                if (new_y >= 0 && board[new_y][new_x] != 0) //checks collision with locked blocks
                    return false;       //prevents movement into occupied cells
            }
        }
    }

    return true;                        //confirms that the movement is valid
}


void rotatepiece()                    //rotates the current piece clockwise
{
    char rotated[4][4];                //temporary array for the rotated piece

    for (int r = 0; r < 4; r++)       //loops through the original piece rows
    {
        for (int c = 0; c < 4; c++)   //loops through the original piece columns
        {
            rotated[r][c] = activepiece[3 - c][r]; //rotates the piece clockwise
        }
    }

    for (int r = 0; r < 4; r++)       //checks the rotated piece rows
    {
        for (int c = 0; c < 4; c++)   //checks the rotated piece columns
        {
            if (rotated[r][c] == '1') //checks actual blocks only
            {
                int x = piece_x + c;  //calculates the block's horizontal position
                int y = piece_y + r;  //calculates the block's vertical position

                if (x < 0 || x >= cols || y >= rows) //checks board boundaries
                    return;             //cancels rotation if the block is outside

                if (y >= 0 && board[y][x] != 0) //checks collision with locked blocks
                    return;             //cancels rotation if the space is occupied
            }
        }
    }

    for (int r = 0; r < 4; r++)       //copies the rotated piece rows
    {
        for (int c = 0; c < 4; c++)   //copies the rotated piece columns
        {
            activepiece[r][c] = rotated[r][c]; //updates the active piece
        }
    }
}


void harddrop()                       //drops the piece directly to the bottom
{
    while (canmove(0, 1))             //checks whether the piece can move down
    {
        piece_y++;                    //moves the piece down one row
    }

    lockpiece();                      //locks the piece in its final position
    clearlines();                     //clears completed rows
    spawnpiece();                     //creates the next piece
}


void handleinput(sf::RenderWindow& window, sf::Clock& clock) //handles keyboard input
{
    while (const std::optional event = window.pollEvent()) //checks for pending events
    {
        if (event->is<sf::Event::Closed>()) //checks whether the window was closed
            window.close();             //closes the game window

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) //gets the key that was pressed
        {
            if (key->code == sf::Keyboard::Key::Escape) //checks the escape key
            {
                if (!gameover)          //checks whether the game is still active
                    savegame();         //saves the current game progress

                window.close();         //closes the game window
            }

            if (!gameover && key->code == sf::Keyboard::Key::P) //checks the pause key
            {
                paused = !paused;       //toggles the pause state
            }

            if (!paused && !gameover && (key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::A)) //checks left movement keys
            {
                if (canmove(-1, 0))     //checks whether the piece can move left
                    piece_x--;          //moves the piece left
            }

            if (!paused && !gameover && (key->code == sf::Keyboard::Key::Right || key->code == sf::Keyboard::Key::D)) //checks right movement keys
            {
                if (canmove(1, 0))      //checks whether the piece can move right
                    piece_x++;          //moves the piece right
            }

            if (!paused && !gameover && (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W)) //checks rotation keys
            {
                rotatepiece();          //rotates the active piece clockwise
            }

            if (!paused && !gameover && (key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S)) //checks downward movement keys
            {
                if (canmove(0, 1))      //checks whether the piece can move down
                    piece_y++;          //moves the piece down one row
            }

            if (!paused && !gameover && key->code == sf::Keyboard::Key::Space) //checks the hard drop key
            {
                harddrop();             //drops the piece directly down
            }

            if (gameover && key->code == sf::Keyboard::Key::R) //checks the restart key
            {
                resetgame(clock);       //starts a new game
            }
        }
    }
}


void lockpiece()                      //locks the piece in its current position
{
    for (int r = 0; r < 4; r++)       //loops through the piece rows
    {
        for (int c = 0; c < 4; c++)   //loops through the piece columns
        {
            if (activepiece[r][c] == '1') //checks actual blocks only
            {
                board[piece_y + r][piece_x + c] = currentpiece + 1; //stores the piece on the board
            }
        }
    }
}


void clearlines()                     //removes completely filled rows
{
    for (int r = rows - 1; r >= 0; r--) //starts checking from the bottom row
    {
        bool full = true;              //assumes the row is completely filled

        for (int c = 0; c < cols; c++) //checks every cell in the row
        {
            if (board[r][c] == 0)     //checks whether a cell is empty
            {
                full = false;          //marks the row as incomplete
                break;                 //stops checking the current row
            }
        }

        if (full)                      //checks whether the row is complete
        {
            score += 100;              //increases the score
            linescleared++;            //increases the cleared line counter

            if (linescleared % 10 == 0) //checks whether ten lines have been cleared
            {
                level++;               //increases the game level
            }

            for (int i = r; i > 0; i--) //moves all rows above downward
            {
                for (int c = 0; c < cols; c++) //loops through every column
                {
                    board[i][c] = board[i - 1][c]; //copies the row above
                }
            }

            for (int c = 0; c < cols; c++) //loops through the top row
            {
                board[0][c] = 0;        //clears the top row
            }

            r++;                        //checks the same row again after shifting
        }
    }
}


void spawnpiece()                     //spawns a random tetris piece
{
    currentpiece = rand() % 7;         //selects a random piece from 0 to 6

    for (int r = 0; r < 4; r++)       //loops through the piece rows
    {
        for (int c = 0; c < 4; c++)   //loops through the piece columns
        {
            activepiece[r][c] = pieces[currentpiece][r][c]; //copies the selected piece
        }
    }

    piece_x = 3;                       //places the piece horizontally
    piece_y = 0;                       //places the piece at the top

    if (!canmove(0, 0))                //checks whether the new piece can fit
    {
        gameover = true;               //ends the game if the piece cannot fit
        clearsave();                   //clears the saved game
    }
}


void updatefalling()                  //controls automatic piece falling
{
    int speed = 30 - (level - 1) * 5;  //calculates falling speed based on level

    if (speed < 5)                    //checks whether the speed is too high
    {
        speed = 5;                    //sets the minimum falling speed
    }

    framecount++;                     //increases the frame counter

    if (framecount >= speed)          //checks whether enough frames have passed
    {
        if (canmove(0, 1))             //checks whether the piece can move down
        {
            piece_y++;                 //moves the piece down one row
        }
        else
        {
            lockpiece();               //locks the piece on the board
            clearlines();              //clears completed rows
            spawnpiece();              //creates the next piece
        }

        framecount = 0;                //resets the frame counter
    }
}


void drawboard(sf::RenderWindow& window, sf::RectangleShape& block, sf::Color colors[]) //draws the game board
{
    for (int r = 0; r < rows; r++)    //loops through all board rows
    {
        for (int c = 0; c < cols; c++) //loops through all board columns
        {
            block.setFillColor(colors[board[r][c]]); //sets the color of each cell

            block.setPosition({       //sets the position of the current block
                board_x + c * cell + 1, //calculates the horizontal position
                board_y + r * cell + 1  //calculates the vertical position
            });

            window.draw(block);        //draws the current cell
        }
    }
}


void drawpiece(sf::RenderWindow& window, sf::RectangleShape& block, sf::Color colors[]) //draws the active piece
{
    for (int r = 0; r < 4; r++)       //loops through the piece rows
    {
        for (int c = 0; c < 4; c++)   //loops through the piece columns
        {
            if (activepiece[r][c] == '1') //checks actual blocks only
            {
                block.setFillColor(colors[currentpiece + 1]); //sets the current piece color

                block.setPosition({   //sets the position of the current block
                    board_x + (piece_x + c) * cell + 1, //calculates the horizontal position
                    board_y + (piece_y + r) * cell + 1  //calculates the vertical position
                });

                window.draw(block);    //draws the current block
            }
        }
    }
}


bool loadpieces()                     //loads all seven tetris pieces
{
    ifstream file("../../../src/pieces.txt"); //opens the pieces file
    if (!file)
        file.open("../../../pieces.txt");

    if (!file)                        //checks whether the file opened successfully
    {
        cout << "Could not open pieces.txt" << endl; //displays an error message
        return false;                 //stops loading if the file is unavailable
    }

    for (int p = 0; p < 7; p++)       //loops through all seven pieces
    {
        for (int r = 0; r < 4; r++)   //loops through the piece rows
        {
            for (int c = 0; c < 4; c++) //loops through the piece columns
            {
                file >> pieces[p][r][c]; //loads each piece cell
            }
        }
    }

    file.close();                     //closes the pieces file
    return true;                      //confirms successful loading
}


int main()                            //main function where the game starts
{
    sf::RenderWindow window(sf::VideoMode({520, 640}), "Tetris"); //creates the game window

    window.setFramerateLimit(60);     //limits the game to 60 frames per second

    if (!loadboard())                 //loads the game board
        return 1;                     //ends the program if loading fails

    if (!loadpieces())                //loads all seven tetris pieces
        return 1;                     //ends the program if loading fails

    srand(static_cast<unsigned>(time(0))); //initializes random piece selection
    spawnpiece();                     //spawns the first random piece

    sf::Color colors[8];              //creates an array for the game colors
    initializecolors(colors);         //initializes all piece colors

    sf::RectangleShape block;         //creates a rectangle for drawing blocks
    initializeblock(block);           //sets the block dimensions

    sf::Clock clock;                  //creates a clock for game timing
    sf::Font font;                    //creates the game font

    if (!font.openFromFile("../../../arial.ttf")) //loads the Arial font
    {
        if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) //loads Windows system Arial font if local file is missing
        {
            cout << "Could not load font!" << endl; //displays an error message
            return 1;                 //ends the program if loading fails
        }
    }

    sf::Text scoretext(font);         //creates the score text
    scoretext.setCharacterSize(24);   //sets the score text size
    scoretext.setFillColor(sf::Color::White); //sets the score text color
    scoretext.setPosition({350.f, 30.f}); //sets the score text position

    sf::Text leveltext(font);         //creates the level text
    leveltext.setCharacterSize(24);   //sets the level text size
    leveltext.setFillColor(sf::Color::White); //sets the level text color
    leveltext.setPosition({350.f, 80.f}); //sets the level text position

    sf::Text linestext(font);         //creates the cleared lines text
    linestext.setCharacterSize(24);   //sets the lines text size
    linestext.setFillColor(sf::Color::White); //sets the lines text color
    linestext.setPosition({350.f, 130.f}); //sets the lines text position

    sf::Text gameovertext(font);      //creates the game over message
    gameovertext.setString("GAME OVER!"); //sets the game over message
    gameovertext.setCharacterSize(40); //sets the game over text size
    gameovertext.setFillColor(sf::Color::Red); //sets the game over text color
    gameovertext.setStyle(sf::Text::Bold); //makes the game over message bold and clear
    gameovertext.setOutlineColor(sf::Color::Black); //adds a black outline for contrast
    gameovertext.setOutlineThickness(2.f); //sets the outline thickness
    gameovertext.setPosition({45.f, 280.f}); //centers the game over message over the board

    sf::Text pausedtext(font);        //creates the paused message
    pausedtext.setString("PAUSED");   //sets the paused message
    pausedtext.setCharacterSize(40);  //sets the paused text size
    pausedtext.setFillColor(sf::Color::Yellow); //sets the paused text color
    pausedtext.setPosition({170.f, 280.f}); //sets the paused text position

    while (window.isOpen())           //keeps the game running while the window is open
    {
        handleinput(window, clock);   //handles keyboard input

        if (paused)                   //checks whether the game is paused
        {
            clock.restart();          //resets the clock while the game is paused
        }

        if (!gameover && !paused)     //checks whether gameplay should continue
        {
            updatefalling();          //updates the falling piece
        }

        window.clear();               //clears the previous frame

        drawboard(window, block, colors); //draws the game board
        drawpiece(window, block, colors); //draws the active piece

        scoretext.setString("Score: " + to_string(score)); //updates the score display
        leveltext.setString("Level: " + to_string(level)); //updates the level display
        linestext.setString("Lines: " + to_string(linescleared)); //updates the lines display

        window.draw(scoretext);       //draws the score
        window.draw(leveltext);       //draws the level
        window.draw(linestext);       //draws the cleared lines

        if (gameover)                 //checks whether the game has ended
        {
            window.draw(gameovertext); //displays the game over message
        }

        if (paused && !gameover)      //checks whether the game is paused
        {
            window.draw(pausedtext);  //displays the paused message
        }

        window.display();             //displays the completed game frame
    }

    return 0;                         //ends the program successfully
}