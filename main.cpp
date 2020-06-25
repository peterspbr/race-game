/*
Race game - Old style race game with modern atributes! 

Copyright(C) 2020 Peter S - m4sh1lo@protonmail.com
This project is copyrighted under Apache License Version 2.0, January 2004
You can read the license included in project or visit: http://www.apache.org/licenses/
*/

// Import system modules
#include <SFML/Graphics.hpp>
#include <math.h>

// Define namespaces
using namespace sf;
using namespace std;

// Game variables
int windowWidth = 1280;
int windowHeight = 720;
int roadWidth = 2000;
int segmentLenght = 200;

float camD = 0.84;

// Render quads for drawing objects
void drawQuad(RenderWindow &win, Color color, int x1, int y1, int w1, int x2, int y2, int w2)
{
    ConvexShape shape(4);
    shape.setFillColor(color);
    shape.setPoint(0, Vector2f(x1 - w1, y1));
    shape.setPoint(1, Vector2f(x2 - w2, y2));
    shape.setPoint(2, Vector2f(x2 + w2, y2));
    shape.setPoint(3, Vector2f(x1 + w1, y1));
    win.draw(shape);
}

// Determine the road tragetory, such as curves and hills
struct Line
{
    float x, y, z;
    float X, Y, Z;
    float curve, spriteX, clip, scale;
    Sprite sprite;

    Line(){spriteX = curve = x = y = z = 0;}

    // Setup simulated camera perpective projection
    void cameraProject(int camX, int camY, int camZ)
    {
        scale = camD / (z - camZ);
        X = (1 + scale * (x - camX)) * windowWidth / 2;
        Y = (1 - scale * (y - camY)) * windowHeight / 2;
        Z = scale * roadWidth * windowWidth / 2;
    }

    void drawSprite(RenderWindow &window)
    {
        Sprite s = sprite;
        int sWidth = s.getTextureRect().width;
        int sHeight = s.getTextureRect().height;

        float destX = X + scale * spriteX * windowWidth / 2;
        float destY = Y + 4;
        float destW = sWidth * Z / 266;
        float destH = sHeight * Z / 266;

        destX += destW * spriteX;
        destY += destH * (-1);

        float clipH = destY + destH - clip;

        if(clipH < 0) {clipH = 0;}
        if(clipH >= destH) {return;}

        s.setTextureRect(IntRect(0, 0, sWidth, sHeight - sHeight * clipH / destH));
        s.setScale(destW/sWidth, destH / sHeight);
        s.setPosition(destX, destY);
        window.draw(s);
    }
};

// The main function
int main()
{
    RenderWindow window(VideoMode(windowWidth,windowHeight), "Race game"); // Create the main window
    window.setFramerateLimit(60); // Limit the framerate to 60 FPS

    Texture texture[50];
    Sprite object[50];

    // Search for image files inside the assets folder
    for(int i=1; i <= 7; i++)
    {
        texture[i].loadFromFile("assets/" + to_string(i)+".png");
        texture[i].setSmooth(true);
        object[i].setTexture(texture[i]);
    }

    // Setup background texture
    Texture background;
    background.loadFromFile("assets/bg.png");
    background.setRepeated(true);

    // Setup background sprite
    Sprite bgSprite(background); // Set background texture to sprite
    bgSprite.setTextureRect(IntRect(0, 0, 5000, 411));
    bgSprite.setPosition(-2000, 0);

    vector<Line> lines;

    // Setup for road curves
    for(int i = 0; i < 1600; i++)
    {
        Line line;
        line.z = i*segmentLenght;

        if(i > 300 && i < 700)   {line.curve = 0.5f;}
        if(i > 1100)             {line.curve = -0.7;}
        if(i < 300 && i%20 == 0) {line.spriteX = -2.5f; line.sprite = object[5];}
        if(i%17 == 0)            {line.spriteX = 2.0f;  line.sprite = object[6];}
        if(i > 300 && i%20 == 0) {line.spriteX = -0.7f; line.sprite = object[4];}
        if(i > 300 && i%20 == 0) {line.spriteX = -1.2f; line.sprite = object[1];}
        if(i == 400)             {line.spriteX = -1.2f;  line.sprite = object[7];}
        if(i > 750)              {line.y = sin(i / 30.0f) * 1500;}

        lines.push_back(line);
    }

    int N = lines.size();
    int pos = 0; // Position of the camera
    int H = 1500; // Camera height

    float playerX = 0; // Player position in X axis

    // Main game loop
    while(window.isOpen())
    {
        Event event;
        while(window.pollEvent(event))
        {
            if(event.type == Event::Closed)
            {
                window.close();
            }
        }

        int speed = 200; // Car speed

        // Keyboard events
        if(Keyboard::isKeyPressed(Keyboard::Left)  || Keyboard::isKeyPressed(Keyboard::A))   {playerX += -1.0f / 32.0f;}
        if(Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::D))   {playerX +=  1.0f / 32.0f;}
        if(Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::S))    {speed = 0;}
        if(Keyboard::isKeyPressed(Keyboard::LShift))                                         {speed *= 2;}

        pos += speed; // Set the position according to the speed

        while(pos >= N * segmentLenght) {pos -= N * segmentLenght;} // Restart the track for create a loop simulation
        while(pos < 0)                  {pos += N * segmentLenght;} // Start car running

        window.clear(Color::Black); // Set the main window color to black
        window.draw(bgSprite); // Draw the background sprite

        int startPos = pos / segmentLenght; // Camera start position
        int camH = lines[startPos].y + H; // Initial camera height

        if(speed > 0) {bgSprite.move(-lines[startPos].curve * 2, 0);} // Move all the sprites in the scene
        if(speed < 0) {bgSprite.move( lines[startPos].curve * 2, 0);} // Stop all the sprites in the scene

        int maxY = windowHeight; // Determine a limit in the Y axis

        float x = 0, dx = 0;

        for(int n = startPos; n < startPos + 300; n++)
        {
            Line &l = lines[n%N];
            l.cameraProject(playerX * roadWidth - x, camH, startPos * segmentLenght - (n > N?N * segmentLenght:0));
            x += dx;
            dx += l.curve;

            l.clip = maxY;

            if(l.Y >= maxY) {continue;}

            maxY = l.Y;

            Color grass =  (n / 3)&2?Color(16, 200 / 4, 16):Color(0, 154 / 4, 0); // Set the color of the grass (default is dark green)
            Color rumble = (n / 3)&2?Color(255 / 4, 255 / 4, 255 / 4):Color(0, 0, 0); // Set the color of the rumble (default is white)
            Color road =   (n / 3)&2?Color(107 / 4, 107 / 4, 107 / 4):Color(105 / 4, 105 / 4, 105 / 4); // Set the color of the road (default is grey)

            Line p = lines[(n - 1)%N];

            drawQuad(window, grass, 0, p.Y, windowWidth, 0, l.Y, windowWidth); // Draw the grass
            drawQuad(window, rumble, p.X, p.Y, p.Z * 1.2f, l.X, l.Y, l.Z * 1.2f); // Draw the road rumble
            drawQuad(window, road, p.X, p.Y, p.Z, l.X, l.Y, l.Z); // Draw road
        }

        for(int n = startPos + 300; n > startPos; n--) {lines[n%N].drawSprite(window);} // Draw objects

        window.display();
    }

    return 0;
}