
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
// 7 tetris shapes, 1 is a block and 0 is empty
int pieces[7][4][4] = {
    { {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0} }, // I shape
    { {0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} }, // O shape
    { {0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} }, // T shape
    { {0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0} }, // S shape
    { {1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} }, // Z shape
    { {1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} }, // J shape
    { {0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} }  // L
};

int currentPiece = 0;// stores which piece is falling now
int nextPiece = 0;// stores the next piece waiting
int pieceRow = 0; // starts at the top
int pieceCol = 3; // starts near the middle

int fallTime = 0;// counts how many frames have passed
int fallSpeed = 30; // drops 1 row after every30 frames on level 1
bool loadboard()            // Load the Game Board Where Pieces Fall
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

                if (boardR >= rows) // hit the bottom
                    return false;

                if (boardC < 0 || boardC >= cols) // hit left or right wall then stop
                    return false;

                if (boardR >= 0 && board[boardR][boardC] != 0) // hit another locked block
                    return false;
            }
        }
    }
    return true;
}

// copies the piece into the board and spawns the next one
void lockPiece() //for locking the piece
{
    for (int r = 0; r < 4; r++)// runs through the rows
    {
        for (int c = 0; c < 4; c++) // runs through the columns
        {
            if (pieces[currentPiece][r][c] == 1)//copy the actual blocks
            {// find where this block sits on the main board or not 
                int boardR = pieceRow + r;
                int boardC = pieceCol + c;

                if (boardR >= 0 && boardR < rows && boardC >= 0 && boardC < cols)
                {
                    board[boardR][boardC] = currentPiece + 1; // +1 because 0 is empty
                }
            }
        }
    }

    // bring in the next piece at the top
    currentPiece = nextPiece;
    nextPiece = rand() % 7; //for random next piece
    pieceRow = 0;
    pieceCol = 3;
}

// sends the piece straight to the bottom and locks it
void hardDrop()
{
    while (canMove(pieceRow + 1, pieceCol))
    {
        pieceRow++; // keep moving down until it hits something like another piece
    }
    lockPiece();
    fallTime = 0; // reset timer for the new piece
}

// draws the falling piece on top of the board
void drawPiece(sf::RenderWindow& window, sf::RectangleShape& block, sf::Color colors[])
{
    block.setFillColor(colors[ 1]);

    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            if (pieces[currentPiece][r][c] == 1)
            {
                block.setPosition({
                    BOARD_X + (pieceCol + c) * CELL + 1,
                    BOARD_Y + (pieceRow + r) * CELL + 1
                });

                window.draw(block);
            }
        }
    }
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

int main()
{
    srand(time(0)); // so we get different random pieces each time
    currentPiece = rand() % 7;
    nextPiece = rand() % 7;
    if (!loadboard())
        return -1;

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

        drawBoard(window, block, colors);
        drawPiece(window, block, colors);
        window.display();
    }

    return 0;
}
