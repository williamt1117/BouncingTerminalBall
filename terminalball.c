#include <stdlib.h>
#include <ncurses.h>
#include<stdio.h>
#include<time.h>
#include<math.h>
#include<locale.h>

//Coordinate system measured from top left. X value increases downwards and Y increases to the right.
// William Trottier 11/4/2021

#define WIDTH 120
#define HEIGHT 30
#define FPS 60
#define GRAVITY 170.0 //units per second squared
#define BOUNCEDAMPER 0.2

#define RADIUS 3 //radius excludes center point (ie. 0 = 1 pixel "circle") measured as horizontal units.

//The "Circle" is treated as a rectangle for physics since no oblique bounces occur
#define COLLIDERHEIGHT RADIUS + 1
#define COLLIDERWIDTH RADIUS * 2 + 1

#define RADIUSDAMPER -0.07 //Only affects visuals, has no affect on physics.

//Velocities measured in units / second. Positions measured in units
#define INITIALXVEL 0
#define INITIALYVEL 90
#define INITIALXPOS COLLIDERHEIGHT / 2.0 + 2
#define INITIALYPOS WIDTH / 2.0

//user input velocity changes
#define HORIZONTALVELOCITYCHANGE 10
#define VERTICALVELOCITYCHANGE 30

#define NULLSPACE " "
#define FILLED "█" //
#define SHADE "▓" //
#define COLORFORE COLOR_MAGENTA
#define COLORBACK COLOR_BLACK
//Color Codes:
// 30 BLACK, 31 RED, 32 GREEN, 33 YELLOW, 34 BLUE, 35 PURPLE, 36 CYAN, 37 WHITE
// 90 - 97 are the lighter versions of the above (respectively)

enum boardFill
{
    empty = 0,
    fill = 1,
    shade = 2
};

//Takes a 2d array of enum Boardfill, "c" and prints every element with a \n between lines
void PrintCanvas(enum boardFill c[HEIGHT][WIDTH])
{
    init_pair(1, COLORFORE, COLORBACK);
    for (int x = 0; x < HEIGHT; x++)
    {
        for (int y = 0; y < WIDTH; y++)
        {
            attrset(COLOR_PAIR(1));
            switch (c[x][y])
            {
                case 0:
               
                    printw(" ");
                    break;
                case 1:
                    printw("%s", FILLED);
                    break;
                case 2:
                    printw("%s", SHADE);
                    break;
            }
            attroff(COLOR_PAIR(1));
        }   
        printw("\n");
    }
    for (int i = 0; i < WIDTH; i++)
        printw("%s", "="); //print ground
}

//Moves the cursor from the bottom left of the canvas to the top left of the canvas to reprint and sets every element of input "c" to empty. 
void ResetCanvas(enum boardFill c[HEIGHT][WIDTH])
{
    move(1, 0);
    for (int x = 0; x < HEIGHT; x++)
        for (int y = 0; y < WIDTH; y++)
            c[x][y] = empty;
}

//Acceleration is constant. Updates velocity by acceleration, bounces if wall is encountered and updates position by "velocity"
void UpdatePhysics(double pos[2], double vel[2])
{
    //update velocity
    vel[0] = vel[0] + (double)GRAVITY / FPS;

    //Check if desired X position is out of bounds
    float expectedPos[2] = {pos[0] + ((double)vel[0] / FPS), pos[1] + ((double)vel[1] / FPS)};
    float halvedHeight = COLLIDERHEIGHT / 2.0;
    float halvedWidth = COLLIDERWIDTH / 2.0;
    int expectedY = round(expectedPos[0]);
    int expectedX = round(expectedPos[1]);

    if ((expectedY + halvedHeight) >= HEIGHT)
    {
        //Bouncing off bottom
        vel[0] = vel[0] - (double)GRAVITY / FPS; //This is important to ensure there is no creation of energy when hitting the ground
        vel[0] *= (-1 + BOUNCEDAMPER);
    }
    if ((expectedX - halvedWidth < 0) || (expectedX + halvedWidth >= WIDTH))
    {
        //Bouncing off left or right
        vel[1] *= -1;
    }

    double finalPosition[2] = {pos[0] + ((double)vel[0] / FPS), pos[1] + ((double)vel[1] / FPS)}; 
    pos[0] = finalPosition[0];
    pos[1] = finalPosition[1];
}

//Given an origin of the oval (ovalOrigin), a width and a height of the oval, and a point, returns a strength value ranging from 0 to infinity.
//If the point is inside the oval it returns a value < 1. If the point is on the surface of the oval it returns 1. If the point is outside the oval it returns a value > 1.
//NOTE: this function is exponential and NOT linear (ex. A point one height above the oval would return a value of 4)
double InsideOval(int point[2], int ovalOrigin[2], double width, double height)
{
    int relativeVector[2] = {point[0] - ovalOrigin[0], point[1] - ovalOrigin[1]};
    double result = pow(relativeVector[1], 2)/pow(width, 2) + pow(relativeVector[0], 2)/pow(height, 2);
    return result;
}

//Given a position and a canvas, rounds the position to a point on the canvas and updates the canvas to be filled according to the strength of the oval at all points contained by the circle.
void UpdateCirclePosition (double pos[2], enum boardFill c[HEIGHT][WIDTH])
{
    int rounded[2] = {round(pos[0]), round(pos[1])};

    for (int rowIndex = rounded[0] - floor(COLLIDERHEIGHT / 2); rowIndex <= rounded[0] + floor(COLLIDERHEIGHT / 2); rowIndex++)
    {
        for (int columnIndex = rounded[1] - floor(COLLIDERWIDTH / 2); columnIndex <= rounded[1] + floor(COLLIDERWIDTH / 2); columnIndex++)
        {
            int testPoint[2] = {rowIndex, columnIndex};
            if (rowIndex >= 0)
            {
                double result = InsideOval(rounded, testPoint, (COLLIDERWIDTH/2) * (1 - RADIUSDAMPER), (COLLIDERHEIGHT/2) * (1 - RADIUSDAMPER));
                if (result <= 0.75)
                {
                    c[rowIndex][columnIndex] = fill;
                }
                else if (result <= 1)
                {
                    c[rowIndex][columnIndex] = shade;
                }
            }
        }
    }
}

int main()
{
    WINDOW* my_win;
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    start_color();
    cbreak();
    timeout(1000 / FPS - 5); //approx. 10ms to finish calculations

    printw("Bouncing Terminal Ball - Use [W] [A] [S] [D] to control the ball. Q to exit.\n");
    
    //Create canvas and fill with spaces.
    enum boardFill canvas[HEIGHT][WIDTH];
    for (int x = 0; x < HEIGHT; x++)
        for (int y = 0; y < WIDTH; y++)
            canvas[x][y] = empty;

    double position[2] = {INITIALXPOS, INITIALYPOS};
    double velocity[2] = {INITIALXVEL, INITIALYVEL};

    while (1) //loop through all frames
    {
        UpdatePhysics(position, velocity);
        UpdateCirclePosition(position, canvas);
        PrintCanvas(canvas);
        refresh();
        //napms(1000 / FPS);
        char ch = getch();
        if (ch == -1)
        {
            napms(1000 / FPS - 5); //approx. 10ms to finish calculations
        }
        else
        {
            //valid character pressed
            if (ch == 'a')
            {
                velocity[1] -= HORIZONTALVELOCITYCHANGE;
            }
            else if (ch == 'd')
            {
                velocity[1] += HORIZONTALVELOCITYCHANGE;
            }
            else if (ch == 'w')
            {
                velocity[0] -= VERTICALVELOCITYCHANGE;
            }
            else if (ch == 's')
            {
                velocity[0] += VERTICALVELOCITYCHANGE;
            }
            else if (ch == 'q')
            {
                endwin();
                return 0; //exit program
            }
        }
        ResetCanvas(canvas);
    }
    printf("\n");
    getch();
    endwin();
    return 0;
}
