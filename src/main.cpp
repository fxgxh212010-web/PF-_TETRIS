
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include<ctime> //for time
#include<cstdlib> //for rand and srand
using namespace std;

const int rows = 20;
const int cols = 10;
const float BOARD_X = 20.f;     //Required for drawing Board
const float BOARD_Y = 20.f;     //Required for drawing Board
const float CELL = 30.f;        //Required for drawing Board

int board[rows][cols];      //2D array which will work as board for tetris Pieces
char pieces[7][4][4];       //Stores all 7 pieces with 4x4 dimension
char activepiece[4][4];     //Stores the active piece with 4x4 dimension
int currpiece = 0;          //To store the index of active piece
int piece_x = 3;            //Controls the horizontal position of the piece
int piece_y = 0;            //Controls the vertical position of the piece

bool loadboard()            //Load the Game Board Where Pieces Fall
{
    ifstream file("../../../src/board.txt");     //Load Board File

    if (!file.is_open())     //If board doesn't open returns error
    {
        cout << "Board failed to load!";
        return false;
    }

    for (int i = 0; i < rows; i++)       //Stores values in Board.txt inside the 2D array.
    {
        for (int j = 0; j < cols; j++)
            file >> board[i][j];
    }

    file.close();           //Closes the Board.txt file
    return true;        //Let's the code run if the board loads successfully
}
// checks if the piece is allowed to move to newRow, newCol
bool canMove(int newRow, int newCol)
{
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            if (pieces[currentPiece][r][c] == 1) // only check actual blocks
            {
                int boardR = newRow + r;//find the block new row
                int boardC = newCol + c;//find board new column

bool loadPieces()                                     //Loads all Tetris pieces from the pieces.txt
{
    ifstream file("../../../src/pieces.txt");         //Load pieces file

    if (!file.is_open())                              //Checks whether the file opened successfully
    {
        cout << "Pieces failed to load!";             //Displays an error message
        return false;                                 //Stops loading if the file fails to open
    }

    for (int p = 0; p < 7; p++)                      //Goes through all 7 pieces
    {
        for (int r = 0; r < 4; r++)                  //Goes through the 4 rows of each piece
        {
            for (int c = 0; c < 4; c++)              //Goes through the 4 columns of each piece
            {
                file >> pieces[p][r][c];             //Stores each piece into the array
            }
        }
    }

    file.close();                                     //Closes the pieces.txt file
    return true;                                      //Let's the code run if the board loads successfully
}

void drawBoard(sf::RenderWindow& window, sf::RectangleShape& block, sf::Color colors[])     //Draws the Board
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
                BOARD_X + c * CELL + 1,
                BOARD_Y + r * CELL + 1
            });

            window.draw(block);
        }
    }
}

void drawPiece(sf::RenderWindow& window, sf::RectangleShape& block, sf::Color colors[]) //Draws the active piece
{
    for (int r = 0; r < 4; r++)                       //Loops through all 4 rows
    {
        for (int c = 0; c < 4; c++)                   //Loops through all 4 columns
        {
            if (activepiece[r][c] == '1')             //Checks whether the current cell contains a block
            {
                block.setFillColor(colors[1]);        //Sets the piece colour to cyan

                block.setPosition({                         //Sets the position of the current block
                    BOARD_X + (piece_x + c) * CELL + 1,      //Calculates the horizontal position
                    BOARD_Y + (piece_y + r) * CELL + 1       //Calculates the vertical position
                });

                window.draw(block);                   //Draws the block on the game window
            }
        }
    }
}

int main()
{
    if (!loadboard())           //Attempts to load the Board
        return -1;              //Stops the cpp if the Board faildsto load

    if (!loadPieces())          //Attempts to load all Tetris pieces
    return -1;                  //Stops the cpp code if pieces cannot be loaded

    currpiece = 0;                 // Selects the first Tetris piece

    for (int r = 0; r < 4; r++)      // Loops through all 4 rows
    {
        for (int c = 0; c < 4; c++)  // Loops through all 4 columns
        {
            activepiece[r][c] = pieces[currpiece][r][c]; // Copies the selected piece into activePiece
        }
    }

    sf::RenderWindow window(sf::VideoMode({520, 640}), "Tetris");
    window.setFramerateLimit(60);

    sf::RectangleShape block(sf::Vector2f(26, 26));
    block.setOutlineThickness(1);
    block.setOutlineColor(sf::Color(100, 100, 120));

    sf::Color colors[] = {
        sf::Color(35, 35, 55),  // Empty cells
        sf::Color::Cyan         // Filled blocks
    };

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        
if (const auto* key = event->getIf<sf::Event::KeyPressed>())// check if a key was pressed
            {
                if (key->code == sf::Keyboard::Key::Space)// if the key is Spacebar
                    hardDrop();//Drops straight
            }
        }
            fallTime++;//adding 1 to counter
        if (fallTime >= fallSpeed)//if 30 frames have passed.
        {
            fallTime = 0;// reset counter back to 0

            if (canMove(pieceRow + 1, pieceCol))// check if the piece can move down 1 row
                pieceRow++;//move it down
            else
                lockPiece();//lock in place
        }
        window.clear(sf::Color(25, 25, 45));

        drawBoard(window, block, colors);       //Draws the Board
        drawPiece(window, block, colors);       //Draws the active Tetris piece
        window.display();
    }

    return 0;
}
