#include <iostream>                 //for input and output
#include <fstream>                  //for reading and writing files
#include <cstdlib>                  //for rand and srand
#include <ctime>                    //for time
#include <string>                   //for strings and to_string
#include <optional>                 //for sfml event handling
#include <SFML/Graphics.hpp>        //for sfml graphics

using namespace std;

//---------------------------------------------------------------- constants
const int rows = 20;                //number of board rows
const int cols = 10;                //number of board columns
const float cell = 30.f;            //size of each board cell
const float board_x = 20.f;         //horizontal board position
const float board_y = 20.f;         //vertical board position

const unsigned int WINDOW_W = 520;  //window width in pixels
const unsigned int WINDOW_H = 640;  //window height in pixels

const int FRAMES_START = 30;        //frames per row on level 1
const int FRAMES_STEP = 5;          //frames faster on each new level
const int FRAMES_MIN = 5;           //the fastest the piece can fall
const int LINES_PER_LEVEL = 10;     //lines needed for each new level
const int POINTS_FOR_ROWS[5] = {0, 100, 300, 500, 800}; //points for clearing 0 / 1 / 2 / 3 / 4 rows at once

const int MENU = 0;                 //game state: main menu
const int PLAYING = 1;              //game state: playing
const int GAME_OVER = 2;            //game state: game over screen

const int BUTTON_LEFT = 140;        //left edge of the menu buttons
const int BUTTON_RIGHT = 390;       //right edge of the menu buttons
const int PLAY_TOP = 285;           //top of the new game button
const int PLAY_BOTTOM = 360;        //bottom of the new game button
const int CONTINUE_TOP = 370;       //top of the continue button
const int CONTINUE_BOTTOM = 445;    //bottom of the continue button
const int EXIT_TOP = 455;           //top of the exit button
const int EXIT_BOTTOM = 530;        //bottom of the exit button

//---------------------------------------------------------------- game data
int board[rows][cols];              //2d array for the tetris board

char pieces[7][4][4];               //stores all 7 tetris pieces
char activepiece[4][4];             //stores the currently falling piece

int currentpiece = 0;               //index of the current piece
int nextpiece = -1;                 //index of the next piece to spawn
int holdpiece = -1;                 //index of the held piece (-1 means the hold box is empty)
bool holdused = false;              //checks whether hold was already used for this piece
int score = 0;                      //stores the player's score
int level = 1;                      //stores the current game level
int linescleared = 0;               //stores the total cleared lines
int framecount = 0;                 //counts frames for automatic falling

int gameState = MENU;               //stores the current game state
bool paused = false;                //checks whether the game is paused
bool nosavemessage = false;         //checks whether the no save message is displayed

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
    holdpiece = -1;                 //empties the hold box

    paused = false;                 //removes the paused state
    gameState = PLAYING;            //starts the playing state

    spawnpiece();                   //creates a new falling piece (can end the game if it does not fit)

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

    file << score << " " << linescleared << " " << level << endl; //line 1: saves score, lines and level
    file << nextpiece << endl;      //line 2: saves the next piece

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

    if (!(file >> score >> linescleared >> level)) //loads score, lines and level
    {
        return false;               //stops loading if the data is incorrect
    }

    int savedpiece;                 //stores the saved next piece

    if (!(file >> savedpiece))      //loads the saved next piece
    {
        return false;               //stops loading if the data is incorrect
    }

    if (savedpiece < 0 || savedpiece >= 7) //checks whether the piece index is valid
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

    paused = false;                 //removes the paused state
    framecount = 0;                 //resets the falling frame counter
    gameState = PLAYING;            //starts the playing state
    holdpiece = -1;                 //empties the hold box

    nextpiece = savedpiece;         //queues the saved next piece
    spawnpiece();                   //drops it from the top and picks a new random next piece

    if (gameState == GAME_OVER)     //checks whether the saved next piece could not fit
    {
        gameState = MENU;           //stays in the main menu (spawnpiece already cleared the save)
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
    colors[5] = sf::Color(255, 60, 170); //sets the fifth piece color (pink)
    colors[6] = sf::Color::Blue;     //sets the sixth piece color
    colors[7] = sf::Color(255, 255, 255); //sets the seventh piece color
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
            if (activepiece[r][c] == '#') //checks whether the current cell contains a block
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
            if (rotated[r][c] == '#') //checks actual blocks only
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


void holdcurrent()                    //BONUS: holds the current piece or swaps it with the held piece
{
    if (holdused)                     //hold can only be used once per piece
        return;                       //ignores the key if already used

    if (holdpiece == -1)              //checks whether the hold box is empty
    {
        holdpiece = currentpiece;     //saves the current piece into the hold box
        spawnpiece();                 //a new piece starts falling instead
    }
    else                              //the hold box already has a piece
    {
        int temp = currentpiece;      //remembers the current piece
        currentpiece = holdpiece;     //the held piece becomes the active piece
        holdpiece = temp;             //the old active piece goes into the hold box

        for (int r = 0; r < 4; r++)   //loops through the piece rows
        {
            for (int c = 0; c < 4; c++) //loops through the piece columns
            {
                activepiece[r][c] = pieces[currentpiece][r][c]; //copies the swapped piece
            }
        }

        piece_x = 3;                  //places the piece horizontally
        piece_y = 0;                  //places the piece at the top

        if (!canmove(0, 0))           //checks whether the swapped piece can fit
        {
            gameState = GAME_OVER;    //ends the game if the piece cannot fit
            clearsave();              //clears the saved game
        }
    }

    holdused = true;                  //blocks another hold until the next piece spawns
}


//---------------------------------------------------------------- input (one function per state)
bool insidebutton(sf::Vector2i mousepos, int top, int bottom) //checks whether the mouse is inside a menu button
{
    return mousepos.x >= BUTTON_LEFT && mousepos.x <= BUTTON_RIGHT && mousepos.y >= top && mousepos.y <= bottom;
}


void menuinput(sf::RenderWindow& window, sf::Clock& clock, const sf::Event& event) //handles input in the main menu
{
    if (!event.is<sf::Event::MouseButtonPressed>()) //only mouse clicks matter in the menu
        return;                         //ignores every other event

    sf::Vector2i mousepos = sf::Mouse::getPosition(window); //gets the mouse position

    if (insidebutton(mousepos, PLAY_TOP, PLAY_BOTTOM)) //checks the new game button
    {
        nosavemessage = false;          //hides the no save message
        resetgame(clock);               //starts a new game
    }

    if (insidebutton(mousepos, CONTINUE_TOP, CONTINUE_BOTTOM)) //checks the continue button
    {
        if (loadgame())                 //attempts to load the saved game
        {
            gameState = PLAYING;        //leaves the main menu
            nosavemessage = false;      //hides the no save message
            clock.restart();            //resets the falling timer
        }
        else
        {
            nosavemessage = true;       //displays the no save message
        }
    }

    if (insidebutton(mousepos, EXIT_TOP, EXIT_BOTTOM)) //checks the exit button
    {
        window.close();                 //closes the game window
    }
}


void playinginput(sf::Clock& clock, const sf::Event& event) //handles input while playing
{
    const auto* key = event.getIf<sf::Event::KeyPressed>(); //checks whether a key was pressed

    if (key == nullptr)                 //ignores events that are not key presses
        return;

    if (key->code == sf::Keyboard::Key::Escape) //checks the escape key
    {
        savegame();                     //saves the current game progress
        gameState = MENU;               //returns to the main menu
        paused = false;                 //removes the paused state
        nosavemessage = false;          //hides the no save message
        clock.restart();                //resets the falling timer
        return;
    }

    if (key->code == sf::Keyboard::Key::P) //checks the pause key
    {
        paused = !paused;               //toggles the pause state
    }

    if (paused)                         //ignores movement keys while paused
        return;

    if (key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::A) //checks left movement keys
    {
        if (canmove(-1, 0))             //checks whether the piece can move left
            piece_x--;                  //moves the piece left
    }

    if (key->code == sf::Keyboard::Key::Right || key->code == sf::Keyboard::Key::D) //checks right movement keys
    {
        if (canmove(1, 0))              //checks whether the piece can move right
            piece_x++;                  //moves the piece right
    }

    if (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::W) //checks rotation keys
    {
        rotatepiece();                  //rotates the active piece
    }

    if (key->code == sf::Keyboard::Key::Down || key->code == sf::Keyboard::Key::S) //checks downward movement keys
    {
        if (canmove(0, 1))              //checks whether the piece can move down
            piece_y++;                  //moves the piece down one row
    }

    if (key->code == sf::Keyboard::Key::Space) //checks the hard drop key
    {
        harddrop();                     //drops the piece directly down
    }

    if (key->code == sf::Keyboard::Key::C) //BONUS: checks the hold key
    {
        holdcurrent();                  //holds or swaps the current piece
    }
}


void gameoverinput(const sf::Event& event) //handles input on the game over screen
{
    const auto* key = event.getIf<sf::Event::KeyPressed>(); //checks whether a key was pressed

    if (key != nullptr && key->code == sf::Keyboard::Key::Enter) //checks the enter key
    {
        clearsave();                    //makes sure a finished game cannot be continued
        nosavemessage = false;          //hides the no save message
        gameState = MENU;               //returns to the main menu
    }
}


void handleinput(sf::RenderWindow& window, sf::Clock& clock) //handles keyboard and mouse input
{
    while (const std::optional event = window.pollEvent()) //checks for pending events
    {
        if (event->is<sf::Event::Closed>()) //checks whether the window was closed
            window.close();             //closes the game window

        switch (gameState)              //each state handles only its own input
        {
        case MENU:
            menuinput(window, clock, *event);
            break;
        case PLAYING:
            playinginput(clock, *event);
            break;
        case GAME_OVER:
            gameoverinput(*event);
            break;
        }
    }
}


void lockpiece()                      //locks the piece in its current position
{
    for (int r = 0; r < 4; r++)       //loops through the piece rows
    {
        for (int c = 0; c < 4; c++)   //loops through the piece columns
        {
            if (activepiece[r][c] == '#') //checks actual blocks only
            {
                board[piece_y + r][piece_x + c] = currentpiece + 1; //stores the piece on the board
            }
        }
    }
}


void clearlines()                     //removes completely filled rows
{
    int count = 0;                     //counts the rows cleared by this piece

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
            count++;                   //counts this cleared row
            linescleared++;            //increases the cleared line counter

            if (linescleared % LINES_PER_LEVEL == 0) //checks whether ten lines have been cleared
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

    score += POINTS_FOR_ROWS[count];   //adds the points once for all cleared rows
}


void spawnpiece()                     //spawns the queued piece and picks the next one
{
    if (nextpiece == -1)               //checks whether this is the very first spawn
        nextpiece = rand() % 7;        //picks the first queued piece

    currentpiece = nextpiece;          //the queued piece becomes the active piece
    nextpiece = rand() % 7;            //picks the new next piece for the preview

    holdused = false;                  //a new piece allows hold to be used again

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
        gameState = GAME_OVER;         //ends the game if the piece cannot fit
        clearsave();                   //clears the saved game
    }
}


//---------------------------------------------------------------- update
void updatefalling()                  //controls automatic piece falling
{
    int speed = FRAMES_START - (level - 1) * FRAMES_STEP; //calculates falling speed based on level

    if (speed < FRAMES_MIN)           //checks whether the speed is too high
    {
        speed = FRAMES_MIN;           //sets the minimum falling speed
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


void updategame()                     //updates the game for the current state
{
    switch (gameState)                 //each state does only its own work
    {
    case MENU:
        break;                         //nothing to update in the menu
    case PLAYING:
        if (!paused)                   //checks whether the game is paused
            updatefalling();           //updates the falling piece
        break;
    case GAME_OVER:
        break;                         //nothing to update after the game ends
    }
}


//---------------------------------------------------------------- drawing
void drawtext(sf::RenderWindow& window, const sf::Font& font, const string& words, int size, sf::Color color, float x, float y) //draws one line of text
{
    sf::Text text(font);               //creates the text
    text.setString(words);             //sets the words
    text.setCharacterSize(size);       //sets the text size
    text.setFillColor(color);          //sets the text color
    text.setPosition({x, y});          //sets the text position
    window.draw(text);                 //draws the text
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


void drawshadow(sf::RenderWindow& window, sf::Color colors[]) //BONUS: draws the outline where the piece will land
{
    int drop = 0;                      //how many rows the copy can move down

    while (canmove(0, drop + 1))       //keeps moving the copy down while it still fits
    {
        drop++;                        //moves the copy one row down
    }

    sf::RectangleShape outline({cell - 2, cell - 2}); //creates one outline block
    outline.setFillColor(sf::Color::Transparent);     //keeps the inside empty
    outline.setOutlineColor(colors[currentpiece + 1]); //uses the piece color for the outline
    outline.setOutlineThickness(-2.f); //draws the outline inside the block

    for (int r = 0; r < 4; r++)       //loops through the piece rows
    {
        for (int c = 0; c < 4; c++)   //loops through the piece columns
        {
            if (activepiece[r][c] == '#') //checks actual blocks only
            {
                outline.setPosition({ //sets the position of the shadow block
                    board_x + (piece_x + c) * cell + 1, //calculates the horizontal position
                    board_y + (piece_y + r + drop) * cell + 1 //calculates the landing position
                });

                window.draw(outline);  //draws the shadow block
            }
        }
    }
}


void drawpiece(sf::RenderWindow& window, sf::RectangleShape& block, sf::Color colors[]) //draws the active piece
{
    for (int r = 0; r < 4; r++)       //loops through the piece rows
    {
        for (int c = 0; c < 4; c++)   //loops through the piece columns
        {
            if (activepiece[r][c] == '#') //checks actual blocks only
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


void drawnext(sf::RenderWindow& window, const sf::Font& font, sf::Color colors[]) //draws the next piece preview
{
    if (nextpiece < 0 || nextpiece >= 7) //checks whether a next piece exists
        return;                        //skips drawing if no piece is queued

    drawtext(window, font, "Next piece:", 16, sf::Color::White, 356.f, 184.f); //draws the next piece label

    const float mini = 9.f;            //size of each preview cell
    const float box_cx = 470.f;        //horizontal center of the piece area
    const float box_cy = 195.f;        //vertical center of the 4th box

    int minr = 4, maxr = -1;           //tracks the top and bottom rows of the piece
    int minc = 4, maxc = -1;           //tracks the left and right columns of the piece

    for (int r = 0; r < 4; r++)       //loops through the piece rows
    {
        for (int c = 0; c < 4; c++)   //loops through the piece columns
        {
            if (pieces[nextpiece][r][c] == '#') //checks actual blocks only
            {
                if (r < minr) minr = r; //updates the top row
                if (r > maxr) maxr = r; //updates the bottom row
                if (c < minc) minc = c; //updates the left column
                if (c > maxc) maxc = c; //updates the right column
            }
        }
    }

    if (maxr == -1)                    //checks whether the piece has any blocks
        return;                        //skips drawing an empty piece

    float w = (maxc - minc + 1) * mini; //calculates the piece width
    float h = (maxr - minr + 1) * mini; //calculates the piece height

    float startx = box_cx - w / 2.f;   //centers the piece horizontally
    float starty = box_cy - h / 2.f;   //centers the piece vertically

    sf::RectangleShape backing({w + 6.f, h + 6.f}); //creates a dark backing behind the piece
    backing.setFillColor(colors[0]);   //uses the empty cell color
    backing.setPosition({startx - 3.f, starty - 3.f}); //positions the backing
    window.draw(backing);              //draws the backing

    sf::RectangleShape smallblock({mini - 1.f, mini - 1.f}); //creates a small block
    smallblock.setFillColor(colors[nextpiece + 1]); //uses the next piece's color

    for (int r = minr; r <= maxr; r++) //loops through the piece's used rows
    {
        for (int c = minc; c <= maxc; c++) //loops through the piece's used columns
        {
            if (pieces[nextpiece][r][c] == '#') //checks actual blocks only
            {
                smallblock.setPosition({startx + (c - minc) * mini, starty + (r - minr) * mini}); //positions the block
                window.draw(smallblock); //draws the preview block
            }
        }
    }
}


void drawhold(sf::RenderWindow& window, const sf::Font& font, sf::Color colors[]) //BONUS: draws the held piece in the side panel
{
    drawtext(window, font, "Hold (C):", 16, sf::Color::White, 356.f, 250.f); //draws the hold label

    if (holdpiece < 0 || holdpiece >= 7) //checks whether a piece is being held
        return;                        //skips drawing if the hold box is empty

    const float mini = 9.f;            //size of each preview cell
    const float box_cx = 470.f;        //horizontal center of the piece area
    const float box_cy = 261.f;        //vertical center of the hold box

    int minr = 4, maxr = -1;           //tracks the top and bottom rows of the piece
    int minc = 4, maxc = -1;           //tracks the left and right columns of the piece

    for (int r = 0; r < 4; r++)       //loops through the piece rows
    {
        for (int c = 0; c < 4; c++)   //loops through the piece columns
        {
            if (pieces[holdpiece][r][c] == '#') //checks actual blocks only
            {
                if (r < minr) minr = r; //updates the top row
                if (r > maxr) maxr = r; //updates the bottom row
                if (c < minc) minc = c; //updates the left column
                if (c > maxc) maxc = c; //updates the right column
            }
        }
    }

    if (maxr == -1)                    //checks whether the piece has any blocks
        return;                        //skips drawing an empty piece

    float w = (maxc - minc + 1) * mini; //calculates the piece width
    float h = (maxr - minr + 1) * mini; //calculates the piece height

    float startx = box_cx - w / 2.f;   //centers the piece horizontally
    float starty = box_cy - h / 2.f;   //centers the piece vertically

    sf::RectangleShape backing({w + 6.f, h + 6.f}); //creates a dark backing behind the piece
    backing.setFillColor(colors[0]);   //uses the empty cell color
    backing.setPosition({startx - 3.f, starty - 3.f}); //positions the backing
    window.draw(backing);              //draws the backing

    sf::RectangleShape smallblock({mini - 1.f, mini - 1.f}); //creates a small block
    smallblock.setFillColor(colors[holdpiece + 1]); //uses the held piece's color

    for (int r = minr; r <= maxr; r++) //loops through the piece's used rows
    {
        for (int c = minc; c <= maxc; c++) //loops through the piece's used columns
        {
            if (pieces[holdpiece][r][c] == '#') //checks actual blocks only
            {
                smallblock.setPosition({startx + (c - minc) * mini, starty + (r - minr) * mini}); //positions the block
                window.draw(smallblock); //draws the held block
            }
        }
    }
}


void drawplaying(sf::RenderWindow& window, const sf::Font& font, const sf::Sprite& gamesprite, sf::RectangleShape& block, sf::Color colors[]) //draws the board and the side panel
{
    window.draw(gamesprite);           //draws the gameplay background

    drawboard(window, block, colors);  //draws the game board

    if (gameState == PLAYING)          //BONUS: the shadow is only needed while playing
        drawshadow(window, colors);    //BONUS: draws the landing outline

    drawpiece(window, block, colors);  //draws the active piece

    drawtext(window, font, "Score: " + to_string(score), 24, sf::Color::White, 350.f, 30.f); //draws the score
    drawtext(window, font, "Level: " + to_string(level), 24, sf::Color::White, 350.f, 80.f); //draws the level
    drawtext(window, font, "Lines: " + to_string(linescleared), 24, sf::Color::White, 350.f, 130.f); //draws the cleared lines

    drawnext(window, font, colors);    //draws the next piece preview
    drawhold(window, font, colors);    //BONUS: draws the held piece
}


void drawgame(sf::RenderWindow& window, const sf::Font& font, const sf::Sprite& menusprite, const sf::Sprite& gamesprite, sf::RectangleShape& block, sf::Color colors[]) //draws the current state
{
    window.clear();                    //clears the previous frame

    switch (gameState)                 //each state draws only its own screen
    {
    case MENU:
        window.draw(menusprite);       //draws the main menu background

        if (nosavemessage)             //checks whether the no save message should appear
            drawtext(window, font, "No saved game!", 30, sf::Color::Black, 160.f, 540.f); //draws the no save message
        break;

    case PLAYING:
        drawplaying(window, font, gamesprite, block, colors); //draws the game

        if (paused)                    //checks whether the game is paused
            drawtext(window, font, "PAUSED", 40, sf::Color::Yellow, 105.f, 280.f); //displays the paused message
        break;

    case GAME_OVER:
    {
        drawplaying(window, font, gamesprite, block, colors); //draws the final board

        sf::RectangleShape panel({280.f, 150.f}); //creates a dark panel behind the game over text
        panel.setFillColor(sf::Color(0, 0, 0, 210)); //makes the panel dark and see-through
        panel.setPosition({30.f, 235.f}); //positions the panel on the board
        window.draw(panel);            //draws the panel

        drawtext(window, font, "GAME OVER!", 40, sf::Color::Red, 75.f, 245.f); //displays the game over message
        drawtext(window, font, "Final score: " + to_string(score), 24, sf::Color::White, 75.f, 305.f); //displays the final score
        drawtext(window, font, "Press Enter", 24, sf::Color::Yellow, 105.f, 345.f); //tells the player what to do next
        break;
    }
    }

    window.display();                  //displays the completed frame
}


//---------------------------------------------------------------- loading
bool loadpieces()                     //loads all seven tetris pieces
{
    ifstream file("../../../src/pieces.txt"); //opens the pieces file

    if (!file)                        //checks whether the file opened successfully
    {
        cout << "Could not open pieces.txt" << endl; //displays an error message
        return false;                 //stops loading if the file is unavailable
    }

    int piececount = 0;               //stores the number of pieces in the file

    if (!(file >> piececount) || piececount != 7) //reads the count line and checks that it is 7
    {
        cout << "pieces.txt must start with 7" << endl; //displays an error message
        return false;                 //stops loading if the count is wrong
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


bool loadresources(sf::Texture& menutexture, sf::Texture& gametexture, sf::Font& font) //loads the images and the font
{
    if (!menutexture.loadFromFile("../../../forest_menu.png")) //loads the menu background
    {
        cout << "Could not load forest background!" << endl; //displays an error message
        return false;                 //reports the failure
    }

    if (!gametexture.loadFromFile("../../../game_background.png")) //loads the gameplay background
    {
        cout << "Could not load game background!" << endl; //displays an error message
        return false;                 //reports the failure
    }

    if (!font.openFromFile("../../../arial.ttf")) //loads the Arial font
    {
        cout << "Could not load font!" << endl; //displays an error message
        return false;                 //reports the failure
    }

    return true;                      //confirms that everything loaded
}


void fitbackground(sf::Sprite& sprite, const sf::Texture& texture) //scales and centers a background in the window
{
    float scalex = float(WINDOW_W) / texture.getSize().x; //calculates horizontal scaling
    float scaley = float(WINDOW_H) / texture.getSize().y; //calculates vertical scaling

    float scale = (scalex > scaley) ? scalex : scaley; //selects the larger scaling value

    sprite.setScale({scale, scale});  //resizes the background

    float width = texture.getSize().x * scale;  //calculates the scaled background width
    float height = texture.getSize().y * scale; //calculates the scaled background height

    sprite.setPosition({(float(WINDOW_W) - width) / 2.f, (float(WINDOW_H) - height) / 2.f}); //centers the background
}


void showerror(sf::RenderWindow& window, const sf::Font& font, const string& message) //shows an error in the window until it is closed
{
    while (window.isOpen())           //keeps the error on screen
    {
        while (const std::optional event = window.pollEvent()) //checks for pending events
        {
            if (event->is<sf::Event::Closed>()) //checks whether the window was closed
                window.close();       //closes the game window
        }

        window.clear(sf::Color::Black); //clears the previous frame
        drawtext(window, font, message, 22, sf::Color::Red, 30.f, 280.f); //draws the error message
        window.display();             //displays the error
    }
}


int main()                            //main function where the game starts
{
    sf::RenderWindow window(sf::VideoMode({WINDOW_W, WINDOW_H}), "Tetris"); //creates the game window
    window.setFramerateLimit(60);     //limits the game to 60 frames per second

    sf::Texture menutexture;          //creates the main menu background texture
    sf::Texture gametexture;          //creates the gameplay background texture
    sf::Font font;                    //creates the game font

    if (!loadresources(menutexture, gametexture, font)) //loads the images and the font
        return 1;                     //ends the program if loading fails

    sf::Sprite menusprite(menutexture); //creates the menu background sprite
    sf::Sprite gamesprite(gametexture); //creates the gameplay background sprite
    fitbackground(menusprite, menutexture); //fits the menu background to the window
    fitbackground(gamesprite, gametexture); //fits the gameplay background to the window

    if (!loadpieces())                //loads all seven tetris pieces
    {
        showerror(window, font, "Could not open pieces.txt"); //shows the error in the window
        return 1;                     //ends the program
    }

    srand(time(0));                   //initializes random piece selection
    spawnpiece();                     //spawns the first random piece

    sf::Color colors[8];              //creates an array for the game colors
    initializecolors(colors);         //initializes all piece colors

    sf::RectangleShape block;         //creates a rectangle for drawing blocks
    initializeblock(block);           //sets the block dimensions

    sf::Clock clock;                  //creates a clock for game timing

    while (window.isOpen())           //keeps the game running while the window is open
    {
        handleinput(window, clock);   //1. reads the input
        updategame();                 //2. updates the game
        drawgame(window, font, menusprite, gamesprite, block, colors); //3. draws the game
    }

    return 0;                         //ends the program successfully
}