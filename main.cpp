#include <SFML/Graphics.hpp>
#include <math.h>

using namespace sf;
using namespace std;

int windowWidth = 1280;
int windowHeight = 720;
int roadWidth = 2000;
int segmentLenght = 200;

float camD = 0.84;

void drawQuad(RenderWindow &win, Color color, int x1, int y1, int h1, int x2, int y2, int h2)
{
    ConvexShape shape(4);
    shape.setFillColor(color);
    shape.setPoint(0, Vector2f(x1 - h1, y1));
    shape.setPoint(1, Vector2f(x2 - h2, y2));
    shape.setPoint(2, Vector2f(x2 + h2, y2));
    shape.setPoint(3, Vector2f(x1 + h1, y1));
    win.draw(shape);
}

struct Line
{
    float x, y, z;
    float X, Y, Z;
    float curve, spriteX, clip, scale;
    Sprite sprite;

    Line(){spriteX=curve=x=y=z=0;}

    void cameraProject(int camX, int camY, int camZ)
    {
        scale = camD/(z - camZ);
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

int main()
{
    RenderWindow window(VideoMode(windowWidth,windowHeight), "Race game");
    window.setFramerateLimit(60);

    Texture texture[50];
    Sprite object[50];

    for(int i=1; i <= 7; i++)
    {
        texture[i].loadFromFile("assets/" + to_string(i)+".png");
        texture[i].setSmooth(true);
        object[i].setTexture(texture[i]);
    }

    Texture background;
    background.loadFromFile("assets/bg.png");
    background.setRepeated(true);
    Sprite bgSprite(background);
    bgSprite.setTextureRect(IntRect(0, 0, 5000, 411));
    bgSprite.setPosition(-2000, 0);

    vector<Line> lines;

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
    int pos = 0;
    int H = 1500 * 2;

    float playerX = 0;

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

        int speed = 200;

        if(Keyboard::isKeyPressed(Keyboard::Left)  || Keyboard::isKeyPressed(Keyboard::A))   {playerX += -1.0f / 32.0f;}
        if(Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::D))   {playerX +=  1.0f / 32.0f;}
        if(Keyboard::isKeyPressed(Keyboard::LShift))                                         {speed *= 2;}

        pos += speed;

        while(pos >= N * segmentLenght) {pos -= N * segmentLenght;}
        while(pos < 0)                  {pos += N * segmentLenght;}

        window.clear(Color::Black);
        window.draw(bgSprite);
        int startPos = pos / segmentLenght;
        int camH = lines[startPos].y + H;

        if(speed > 0) {bgSprite.move(-lines[startPos].curve * 2, 0);}
        if(speed < 0) {bgSprite.move( lines[startPos].curve * 2, 0);}

        int maxY = windowHeight;

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

            Color grass =  (n / 3)&2?Color(16, 200 / 4, 16):Color(0, 154 / 4, 0);
            Color rumble = (n / 3)&2?Color(255 / 4, 255 / 4, 255 / 4):Color(0, 0, 0);
            Color road =   (n / 3)&2?Color(107 / 4, 107 / 4, 107 / 4):Color(105 / 4, 105 / 4, 105 / 4);

            Line p = lines[(n - 1)%N];

            drawQuad(window, grass, 0, p.Y, windowWidth, 0, l.Y, windowWidth);
            drawQuad(window, rumble, p.X, p.Y, p.Z * 1.2f, l.X, l.Y, l.Z * 1.2f);
            drawQuad(window, road, p.X, p.Y, p.Z, l.X, l.Y, l.Z);
        }

        for(int n=startPos+300; n>startPos; n--) {lines[n%N].drawSprite(window);}

        window.display();
    }

    return 0;
}