
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>

using namespace std;

const int rows = 20;
const int cols = 10;
const float BOARD_X = 20.f;     //Required for drawing Board
const float BOARD_Y = 20.f;     //Required for drawing Board
const float CELL = 30.f;        //Required for drawing Board

int board[rows][cols];      //2D array which will work as board for tetris Pieces

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

void drawBoard(sf::RenderWindow& window, sf::RectangleShape& block, sf::Color colors[])     //Draws the Board
{
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            block.setFillColor(colors[board[r][c]]);

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
        }

        window.clear(sf::Color(25, 25, 45));

        drawBoard(window, block, colors);
        window.display();
    }

    return 0;
}
