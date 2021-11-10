#include<stdio.h>
#include<time.h>
#include<math.h>

//Coordinate system measured from top left. X value increases downwards and Y increases to the right.
// William Trottier 11/4/2021

#define WIDTH 120
#define HEIGHT 30
#define FPS 60
#define SECONDS 10
#define GRAVITY 120.0 //units per second squared
#define BOUNCEDAMPER 0.08

#define RADIUS 3 //radius excludes center point (ie. 0 = 1 pixel "circle") measured as horizontal units.

//The "Circle" is treated as a rectangle for physics since no oblique bounces occur
#define BOXHEIGHT RADIUS + 1
#define BOXWIDTH RADIUS * 2 + 1

#define RADIUSDAMPER -0.07 //Only affects visuals, has no affect on physics.

//Velocities measured in units / second. Positions measured in units
#define INITIALXVEL 0
#define INITIALYVEL 90
#define INITIALXPOS BOXHEIGHT / 2.0 + 2
#define INITIALYPOS WIDTH / 2.0

#define NULLSPACE " "
#define FILLED "█"
#define SHADE "▓"
#define COLORCODE 95
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
    for (int x = 0; x < HEIGHT; x++)
    {
        for (int y = 0; y < WIDTH; y++)
        {
            switch (c[x][y])
            {
                case 0:
                    printf("%s", NULLSPACE);
                    break;
                case 1:
                    printf("\033[0;%im", COLORCODE); //changes text color
                    printf("%s", FILLED);
                    printf("\033[0m"); //resets to default text color
                    break;
                case 2:
                    printf("\033[0;%im", COLORCODE);
                    printf("%s", SHADE);
                    printf("\033[0m");
                    break;
            }
        }   
        printf("\n");
    }
}

//Moves the cursor from the bottom left of the canvas to the top left of the canvas to reprint and sets every element of input "c" to empty. 
void ResetCanvas(enum boardFill c[HEIGHT][WIDTH])
{
    printf("\x1b[%iA", HEIGHT); //escape character to move cursor up by "HEIGHT"
    for (int x = 0; x < HEIGHT; x++)
        for (int y = 0; y < WIDTH; y++)
            c[x][y] = empty;
}

//Stalls program for "ms" micro seconds.
//NOTE: different systems may use milliseconds for measured time
void Wait(int ms)
{
    clock_t startTime = clock();

    while (clock() < startTime + ns)
    {
        //wait
    }
}

//Acceleration is constant. Updates velocity by acceleration, bounces if wall is encountered and updates position by "velocity"
void UpdatePhysics(double pos[2], double vel[2])
{
    //update velocity
    vel[0] = vel[0] + (double)GRAVITY / FPS;

    //Check if desired X position is out of bounds
    float expectedPos[2] = {pos[0] + ((double)vel[0] / FPS), pos[1] + ((double)vel[1] / FPS)};
    float halvedBoxHeight = BOXHEIGHT / 2.0;
    float halvedBoxWidth = BOXWIDTH / 2.0;
    int expectedY = round(expectedPos[0]);
    int expectedX = round(expectedPos[1]);

    if ((expectedY + halvedBoxHeight) >= HEIGHT)
    {
        //Bouncing off bottom
        vel[0] = vel[0] - (double)GRAVITY / FPS; //This is important to ensure there is no creation of energy when hitting the ground
        vel[0] *= (-1 + BOUNCEDAMPER);
    }
    if ((expectedX - halvedBoxWidth < 0) || (expectedX + halvedBoxWidth >= WIDTH))
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

    for (int rowIndex = rounded[0] - floor(BOXHEIGHT / 2); rowIndex <= rounded[0] + floor(BOXHEIGHT / 2); rowIndex++)
    {
        for (int columnIndex = rounded[1] - floor(BOXWIDTH / 2); columnIndex <= rounded[1] + floor(BOXWIDTH / 2); columnIndex++)
        {
            int testPoint[2] = {rowIndex, columnIndex};
            if (rowIndex >= 0)
            {
                double result = InsideOval(rounded, testPoint, (BOXWIDTH/2) * (1 - RADIUSDAMPER), (BOXHEIGHT/2) * (1 - RADIUSDAMPER));
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
    //Create canvas and fill with spaces.
    enum boardFill canvas[HEIGHT][WIDTH];
    for (int x = 0; x < HEIGHT; x++)
        for (int y = 0; y < WIDTH; y++)
            canvas[x][y] = empty;

    double boxPosition[2] = {INITIALXPOS, INITIALYPOS};
    double boxVelocity[2] = {INITIALXVEL, INITIALYVEL};

    for (int i = 0; i < (SECONDS * FPS); i++) //loop through all frames
    {
        UpdatePhysics(boxPosition, boxVelocity);
        UpdateCirclePosition(boxPosition, canvas);
        PrintCanvas(canvas);
        Wait((int)(1000000 / FPS)); //
        if (i != (SECONDS * FPS - 1)) ResetCanvas(canvas); //Don't clear canvas on last frame for aesthetics.
    }
    printf("\n");
    return 0;
}