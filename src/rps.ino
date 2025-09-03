//  This sketch plays rock, paper, scissors with another AtomS3.
//
//  License:  MIT. See the LICENSE file in the project root for
//            more details.
//

#include <M5Unified.h>
#include <M5GFX.h>
#include <WiFi.h>
#include <WiFiServer.h>
#include <WiFiClient.h>

#include <set>
#include <vector>
#include <algorithm>

//  Set this define to a value of 1 to run the demo, 0
//  to skip the demo
#define RUN_DEMO              1
#if (RUN_DEMO)
#define DEMO_ONLY(...)        __VA_ARGS__
#else   //  RUN_DEMO
#define DEMO_ONLY(...)
#endif  //  RUN_DEMO

//  Number of players
constexpr int numPlayers      = 2;
constexpr int player1Index    = 0;
constexpr int player2Index    = 1;

//  Number of rounds and games/round
constexpr int numRounds       = 4;
constexpr int gamesPerRound   = 3;

//  Size of player icons
constexpr int playerSize      = 30;
constexpr int halfPlayerSize  = playerSize / 2;

//  Colors for player 1 and 2
constexpr int player1Color    = ORANGE;
constexpr int player2Color    = GREEN;

//  Border pixels
constexpr int border1Px       = 5;
constexpr int border2Px       = 10;

#if (RUN_DEMO)
constexpr int demoDelay       = 1000;
#endif  //  RUN_DEMO

//  Time between selections (ms)
constexpr int selectionTime   = 1500;

//  Time between connection checks
constexpr int connectionTime  = 1000;

//  Game result wait time
constexpr int resultDelay     = 5000;

//  Ratios used for scaling various graphics
constexpr float playerRatio     = 0.25F;
constexpr float headRatio       = 0.25F;
constexpr float atomRatio       = 0.40F;
constexpr float eyeballRatio    = 0.40F;
constexpr float pupilRatio      = 0.50F;
constexpr float rockRatio       = 0.33F;
constexpr float scissorRatio    = 0.10F;
constexpr float arcRatio        = 0.25F;

//  WiFi configuration
constexpr int     serverPort    = 9999;
const     String  ssidPrefix    = "RPS";
const     String  ssidPassword  = "topsecret";
const     int     wifiDelay     = 100;

#if (RUN_DEMO)
//  Demo sprites
M5Canvas demoTiltSprite(&M5.Display);
M5Canvas demoPushSprite(&M5.Display);
#endif  //  RUN_DEMO

//  Size of demo sprites
int atomSize;
int halfAtomSize;

//  Sprites for rock, paper and scissors
M5Canvas rockSprite(&M5.Display);
M5Canvas paperSprite(&M5.Display);
M5Canvas scissorSprite(&M5.Display);

//  Player 1 and 2 sprites
M5Canvas player1Sprite(&M5.Display);
M5Canvas player2Sprite(&M5.Display);

//  Win, lose or draw sprites
M5Canvas winSprite(&M5.Display);
M5Canvas loseSprite(&M5.Display);
M5Canvas drawSprite(&M5.Display);

//  Champion or loser sprites
M5Canvas championSprite(&M5.Display);
M5Canvas loserSprite(&M5.Display);

//  Screen dimensions
int screenWidth;
int screenHeight;
int halfScrWidth;
int halfScrHeight;

//  Player 1 and 2 scores
typedef struct scores
{
  //  Number of rounds won
  int playerRound;

  //  Score in this round
  int playerScore;
} scores_t;
scores_t  playerScores [numPlayers];

//  X and Y positions of player information
int playerX;
int playerY;

//  Program state
enum programState : uint8_t {programInit, DEMO_ONLY(initDemo, tiltScreenDemo, pushScreenDemo,) createConnection, createPlayers, randomShapes, selectShapes, playerUpdate};
programState currentState = programInit;

#if (RUN_DEMO)
//  Structure to hold vertices 
typedef struct vertex
{
  int x;
  int y;
} vertex_t;

//  Class to represent a shape with outer and inner vertices
//
//  Note: This class would usually be in a separate header and
//        source file. But, embedding the code makes installation
//        easier
//
class borderShape
{
  public:
    //  CTOR
    borderShape (int shapeX, int shapeY, int shapeSize)
    { 
      //  Central X and Y
      centerX   = shapeX; 
      centerY   = shapeY;

      //  Shape size
      size      = shapeSize;
    };


    //  DTOR    
    ~borderShape (void)
    {
      outerVertices.clear(); 
      innerVertices.clear();
      origOuterVertices.clear(); 
      origInnerVertices.clear();
    }


    //  Drawing and helper functions
    void AddVertex (int x, int y)
    {
      //  Create the outer vertex
      vertex_t v;
      v.x = x;
      v.y = y;

      //  Track outer vertex
      outerVertices.push_back(v);

      //  Adjust for inner vertex
      v.x += (x < centerX) ? 5 : -5;
      v.y += (y < centerY) ? 5 : -5;

      //  Track inner vertex
      innerVertices.push_back(v);
    }


    //  Save the original vertices so we can restore
    //  the shape later
    void SaveShape (void)
    {
      origOuterVertices = outerVertices;
      origInnerVertices = innerVertices;
    }


    //  Restore the shape back to its original settings
    void RestoreShape (void)
    {
      outerVertices = origOuterVertices;
      innerVertices = origInnerVertices;
    }
    

    //  Draw the current shape onto a sprite
    void DrawShape (M5Canvas *sprite)
    {
      //  Draw the outer vertices
      for (int i = 0; i < outerVertices.size() - 1; i ++)
      {
        //  Get 2 vertices
        vertex_t base = outerVertices.at(i);
        vertex_t next = outerVertices.at(i + 1);

        //  Draw a line
        sprite->drawLine(base.x, base.y, next.x, next.y, WHITE);
      }

      for (int i = 0; i < innerVertices.size() - 1; i ++)
      {
        //  Get 2 vertices
        vertex_t base = innerVertices.at(i);
        vertex_t next = innerVertices.at(i + 1);

        //  Draw a line
        sprite->drawLine(base.x, base.y, next.x, next.y, WHITE);
      }

      //  Get the first outer vertex
      if (!outerVertices.empty())
      {
        vertex base = outerVertices.at(0);
        sprite->floodFill(base.x + 2, base.y + 2, WHITE);
      }
    }


    //  Rotate the shape to a given angle
    void RotateShape (int angle)
    {
      //  Convert from degrees to radians which gives
      //  a cleaner rotation
      float adjAngle = float(angle) * (PI / 180);

      //  Loop through all outer vertices
      for (auto &v : outerVertices)
      {
        //  Get the current X and Y
        int tmpX = v.x;
        int tmpY = v.y;

        //  Rotate
        v.x = (tmpX - centerX) * cos(adjAngle) - (tmpY - centerY) * sin(adjAngle) + centerX,
        v.y = (tmpX - centerX) * sin(adjAngle) + (tmpY - centerY) * cos(adjAngle) + centerY;
      }

      //  Loop through all inner vertices
      for (auto &v : innerVertices)
      {
        //  Get the current X and Y
        int tmpX = v.x;
        int tmpY = v.y;

        //  Rotate
        v.x = (tmpX - centerX) * cos(adjAngle) - (tmpY - centerY) * sin(adjAngle) + centerX,
        v.y = (tmpX - centerX) * sin(adjAngle) + (tmpY - centerY) * cos(adjAngle) + centerY;
      }
    }


  private:
    //  Center of the shape
    int centerX;
    int centerY;

    //  Size of the shape
    int size;

    //  Track all vertices
    std::vector<vertex_t> outerVertices;
    std::vector<vertex_t> innerVertices;
    std::vector<vertex_t> origOuterVertices;
    std::vector<vertex_t> origInnerVertices;
};

//  AtomS3 screen shape for demo
borderShape *atomShape;
#endif  //  RUN_DEMO

//  Rock, paper, scissors configuration
int           selectionIndex;
unsigned long nextSelectionTime;
enum          RPSSelection : uint8_t {selectRock, selectPaper, selectScissors, maxSelection, invalidSelection};
std::vector<RPSSelection>  currentSelection;

//  Win, lose or draw
enum winState : uint8_t {win, lose, draw};

//  Our WiFi server object
WiFiServer* ourServer = nullptr;

//  Our WiFi client objects
WiFiClient* ourClient = nullptr;
WiFiClient  serverClient;

//  Connection check time
unsigned long nextConnectionTime = 0;


#if (RUN_DEMO)
//  Redraw the tilt demo sprite
void RedrawTiltSprite (const int direction, const char tiltChoice)
{
  //  Draw the shape
  atomShape->DrawShape(&demoTiltSprite);

  //  Output a tilt choice
  demoTiltSprite.setTextSize(2);
  demoTiltSprite.setTextColor(WHITE);
  demoTiltSprite.setCursor(halfScrWidth - border1Px, halfAtomSize);
  demoTiltSprite.printf("%c", tiltChoice);

  //  Determine the size of the tilt arc
  int tiltRadius  = (screenHeight - playerSize) * playerRatio;
  int arcY        = ((screenHeight - playerSize) / 2) + border1Px;
  demoTiltSprite.fillArc(screenWidth / 2, arcY, tiltRadius, tiltRadius + border1Px, 0, 180, WHITE);

  //  Draw the arrow at the right end of the arc
  if (direction == 0)
  {
    demoTiltSprite.fillTriangle(halfScrWidth + tiltRadius - border1Px, arcY, halfScrWidth + tiltRadius + (border1Px * 2), arcY,  halfScrWidth + tiltRadius, arcY - (border1Px * 2), WHITE);
  }
  else
  {
    demoTiltSprite.fillTriangle(halfScrWidth - tiltRadius - (border1Px * 2), arcY, halfScrWidth - tiltRadius + border1Px, arcY,  halfScrWidth - tiltRadius, arcY - (border1Px * 2), WHITE);
  }

  //  Display the updated tilt sprite
  demoTiltSprite.pushSprite(0, 0);
}

//  Run the demo
void RunDemo (void)
{
  if (currentState == initDemo)
  {
    //  Create the demo sprites
    demoTiltSprite.setColorDepth(1);
    demoPushSprite.setColorDepth(1);
    demoTiltSprite.createSprite(screenWidth, screenHeight - playerSize);
    demoPushSprite.createSprite(screenWidth, screenHeight - playerSize);

    //  Size of the demo M5AtomS3
    atomSize      = (screenHeight - playerSize) * atomRatio;
    halfAtomSize  = atomSize / 2;

    //  Create a representation of the M5AtomS3 screen
    atomShape = new borderShape(halfScrWidth, atomSize / 2, atomSize);

    //  Add sides to the M5AtomS3
    atomShape->AddVertex(halfScrWidth - halfAtomSize, border1Px);             //  top left
    atomShape->AddVertex(halfScrWidth + halfAtomSize, border1Px);             //  top right
    atomShape->AddVertex(halfScrWidth + halfAtomSize, atomSize + border1Px);  //  bottom right
    atomShape->AddVertex(halfScrWidth - halfAtomSize, atomSize + border1Px);  //  bottom left
    atomShape->AddVertex(halfScrWidth - halfAtomSize, border1Px);             //  back to top left

    //  Save the original dimensions
    atomShape->SaveShape();

    //  Draw the shape into the tilt demo
    atomShape->DrawShape(&demoTiltSprite);

    //  Draw the inital tilt sprite
    demoTiltSprite.pushSprite(0, 0);

    //  Next stage
    currentState = tiltScreenDemo;
  }
  else
  if (currentState == tiltScreenDemo)
  {
    //  Internal counters
    static int direction = 0;
    static int tiltCount = 0;

    //  Text and angles for each tilt
    char  tiltChoices []  = {'A', 'B', 'C'};
    int   tiltAngles  []  = {345, 15, 345};

    //  Clear the sprite
    demoTiltSprite.fillSprite(BLACK);

    if (tiltCount != 0)
    {
      //  Restore the original co-ordinates if we need to
      atomShape->RestoreShape();
    }

    //  Rotate the screen
    atomShape->RotateShape(tiltAngles [tiltCount]);

    //  Re-draw the tilt screen
    RedrawTiltSprite(direction, tiltChoices [tiltCount]);

    //  Short delay
    delay(demoDelay);

    //  Restore the original shape
    atomShape->RestoreShape();
    demoTiltSprite.fillSprite(BLACK);
    RedrawTiltSprite(direction, tiltChoices [tiltCount]);

    //  Next tilt sequence
    tiltCount ++;
    direction ^= 1;

    //  Maxed out?
    if (tiltCount >= sizeof(tiltAngles) / sizeof(tiltAngles [0]))
    {
      //  Move to the next stage
      currentState = pushScreenDemo;
    }
  }
  else
  {
    //  Has to be the push demo
    static int pushX = 0;

    //  Restore original shape
    atomShape->RestoreShape();

    //  Draw the shape into the push demo
    demoPushSprite.fillSprite(BLACK);
    atomShape->DrawShape(&demoPushSprite);

    //  Output a push choice
    demoPushSprite.setTextSize(2);
    demoPushSprite.setTextColor(WHITE);
    demoPushSprite.setCursor(halfScrWidth - border1Px, halfAtomSize);
    demoPushSprite.printf("D");

    //  Draw an arrow
    int pushY = halfAtomSize + border1Px;
    demoPushSprite.fillTriangle(pushX, pushY - border2Px, pushX, pushY + border2Px, pushX + border2Px, pushY, WHITE);

    //  Display the push sprite
    demoPushSprite.pushSprite(0, 0);

    //  Calculate there the next X location should be
    pushX += border2Px;

    //  Maxed out?
    if (pushX > halfScrWidth - border1Px)
    {
      //  Move to the next stage
      currentState = createConnection;

      //  We are done with the demo shapes and sprites
      delete atomShape;
      atomShape = nullptr;
      demoPushSprite.deleteSprite();
      demoTiltSprite.deleteSprite();
    }
  }

  //  Short delay between phases
  delay(demoDelay);
}
#endif  //  RUN_DEMO


//  Clear all player scores
void ClearAllScores (void)
{
  memset(&playerScores, 0, sizeof(playerScores));
}

//  Update player scores
void UpdateScores (void)
{
  M5.Display.setTextColor(player1Color, BLACK);
  M5.Display.setCursor((screenWidth / 2) - playerX - border1Px, screenHeight - playerSize);
  M5.Display.printf("%d:%d", playerScores [player1Index].playerRound, playerScores [player1Index].playerScore);

  M5.Display.setTextColor(player2Color, BLACK);
  M5.Display.setCursor(screenWidth - playerX - border1Px, screenHeight - playerSize);
  M5.Display.printf("%d:%d", playerScores [player2Index].playerRound, playerScores [player2Index].playerScore);
}


//  Update round information
void UpdateRounds (void)
{
  //  Is a player over the number of games to win a round?
  if (playerScores [player1Index].playerScore > gamesPerRound)
  {
    //  Player 1 wins the round
    playerScores [player1Index].playerRound ++;
  }
  else
  if (playerScores [player2Index].playerScore > gamesPerRound)
  {
    //  Player 2 wins the round
    playerScores [player2Index].playerRound ++;
  }
  else
  {
    //  Still working towards the win
    return;
  }

  //  Reset scores
  playerScores [player1Index].playerScore = 0;
  playerScores [player2Index].playerScore = 0;
}


//  See if there is a champion, which is the player
//  that has won the most rounds
void IsThereAChampion (void)
{
  //  Did player 1 win all rounds?
  if (playerScores [player1Index].playerRound >= numRounds)
  {
      //  Player 1 wins
      championSprite.pushSprite(0, 0);
  }
  else
  if (playerScores [player2Index].playerRound >= numRounds)
  {
      //  Player 2 wins
      loserSprite.pushSprite(0, 0);
  }
  else
  {
    //  Still playing
    return;    
  }

  //  Clear all scores
  ClearAllScores();

  //  Leave the result visible for a while
  delay(resultDelay);

  //  Update scores
  UpdateScores();
}

//  Create the player sprites
void CreatePlayerSprites (void)
{
  //  Create the player icons
  int playerHead    = playerSize * headRatio;
  int playerBody    = playerSize - playerHead;

  //  Common sizing calculations
  int halfHeadSize  = playerHead / 2;
  int halfBodySize  = playerBody / 2;

  //  Player 1
  player1Sprite.setColorDepth(8);
  player1Sprite.createSprite(playerSize, playerSize);
  player1Sprite.fillCircle(halfPlayerSize, halfHeadSize, halfHeadSize, player1Color);
  player1Sprite.fillTriangle(halfPlayerSize - halfBodySize, playerSize, halfPlayerSize + halfBodySize, playerSize, halfPlayerSize, playerHead, player1Color);

  //  Player 2
  player2Sprite.setColorDepth(8);
  player2Sprite.createSprite(playerSize, playerSize);
  player2Sprite.fillCircle(halfPlayerSize, halfHeadSize, halfHeadSize, player2Color);
  player2Sprite.fillTriangle(halfPlayerSize - halfBodySize, playerSize, halfPlayerSize +  halfBodySize, playerSize, halfPlayerSize, playerHead, player2Color);

  //  Position the player icons
  playerX = screenWidth / 4;
  playerY = screenHeight - playerSize;

  //  Clear screen and display the sprites
  M5.Display.fillScreen(BLACK);
  player1Sprite.pushSprite(0, playerY);
  player2Sprite.pushSprite(screenWidth / 2, playerY);

  //  Initialize and update scores
  ClearAllScores();
  UpdateScores();
}

//  Create sprites for win, lose, draw as well as champion
//  and loser
void CreateWLDSprites (void)
{
  int arcRadius   = (screenHeight - playerSize) * arcRatio;
  int arcY        = ((screenHeight - playerSize) / 2) + border1Px;

  int eyeballSize = (screenHeight - playerSize) * eyeballRatio;
  int pupilSize   = eyeballSize * pupilRatio;

  int eyeX        = screenWidth / 4;
  int eyeY        = eyeballSize / 2;

  //  Create the win sprite (smiley face)
  winSprite.setColorDepth(8);
  winSprite.createSprite(screenWidth, screenHeight - playerSize - border1Px);
  winSprite.fillCircle(eyeX, eyeY, eyeballSize / 2, GREEN);
  winSprite.fillCircle(eyeX, eyeY, pupilSize / 2, BLACK);
  winSprite.fillCircle(screenWidth - eyeX, eyeY, eyeballSize / 2, GREEN);
  winSprite.fillCircle(screenWidth - eyeX, eyeY, pupilSize / 2, BLACK);
  winSprite.fillArc(screenWidth / 2, arcY, arcRadius, arcRadius + border1Px, 0, 180, WHITE);

  //  Create the lose sprite (sad face)
  loseSprite.setColorDepth(8);
  loseSprite.createSprite(screenWidth, screenHeight - playerSize  - border1Px);
  loseSprite.fillCircle(eyeX, eyeY, eyeballSize / 2, BLUE);
  loseSprite.fillCircle(eyeX, eyeY, pupilSize / 2, BLACK);
  loseSprite.fillCircle(screenWidth - eyeX, eyeY, eyeballSize / 2, BLUE);
  loseSprite.fillCircle(screenWidth - eyeX, eyeY, pupilSize / 2, BLACK);
  loseSprite.fillArc(screenWidth / 2, arcY + arcRadius, arcRadius, arcRadius - border1Px, 180, 0, WHITE);

  //  Create the draw sprite (meh face)
  drawSprite.setColorDepth(8);
  drawSprite.createSprite(screenWidth, screenHeight - playerSize  - border1Px);
  drawSprite.fillCircle(eyeX, eyeY, eyeballSize / 2, ORANGE);
  drawSprite.fillCircle(eyeX, eyeY, pupilSize / 2, BLACK);
  drawSprite.fillCircle(screenWidth - eyeX, eyeY, eyeballSize / 2, ORANGE);
  drawSprite.fillCircle(screenWidth - eyeX, eyeY, pupilSize / 2, BLACK);
  drawSprite.fillRect(eyeX, arcY +arcRadius, screenWidth - (eyeX * 2), border1Px, WHITE);

  //  Create the champion sprite (crown)
  championSprite.setColorDepth(1);
  championSprite.createSprite(screenWidth, screenHeight - playerSize  - border1Px);
  championSprite.setPaletteColor(1, GOLD);
  
  //  Draw the crown
  int crownX      = screenWidth / 3;
  int crownY      = (screenHeight - playerSize  - border1Px) / 2;
  int halfCrownY  = crownY / 2;
  for (int i = 0; i < screenWidth; i += crownX)
  {
    championSprite.fillTriangle(i, crownY + halfCrownY, i + crownX, crownY + halfCrownY, i + (crownX / 2), crownY - halfCrownY, WHITE);
  }

  //  Create the loser sprite (big red X)
  loserSprite.setColorDepth(1);
  loserSprite.createSprite(screenWidth, screenHeight - playerSize  - border1Px);
  loserSprite.setPaletteColor(1, RED);
  loserSprite.setCursor(screenWidth / 3, border2Px);
  loserSprite.setTextSize(10);
  loserSprite.printf("X");
}


//  Create the sprites for each shape
void CreateRPSSprites (void)
{
  rockSprite.setColorDepth(8);
  paperSprite.setColorDepth(1);
  scissorSprite.setColorDepth(1);
  rockSprite.createSprite(screenWidth, screenHeight - playerSize - border1Px);
  paperSprite.createSprite(screenWidth, screenHeight - playerSize- border1Px);
  scissorSprite.createSprite(screenWidth, screenHeight - playerSize- border1Px);

  //  The paper sprite is easy
  paperSprite.fillRect(screenWidth / 4, 0, screenWidth / 2, screenHeight - playerSize - border1Px, WHITE);
  paperSprite.setTextSize(1);
  paperSprite.setCursor(screenWidth / 4, 0);
  paperSprite.setTextColor(BLACK);
  paperSprite.printf("It was a...");

  //  The rock is a sequence of cicles
  int rockSize    = (screenHeight - playerSize) * rockRatio;
  int rockRadius  = rockSize / 2;
  rockSprite.fillCircle(halfScrWidth, rockSize, rockRadius, BROWN);
  rockSprite.fillCircle(halfScrWidth - rockRadius, rockSize * 2, rockRadius, BROWN);
  rockSprite.fillCircle(halfScrWidth + rockRadius, rockSize * 2, rockRadius, BROWN);
  
  //  The scissors are some triangles and circles
  int scissorWidth  = screenWidth * scissorRatio;
  int scissorRadius = scissorWidth / 2;
  int scissorLength = (screenHeight - playerSize - scissorWidth) / 2;

  //  Upward pointing part
  scissorSprite.fillTriangle(halfScrWidth - scissorWidth, scissorLength, halfScrWidth, scissorLength, halfScrWidth - (scissorWidth / 2), 0, WHITE);
  scissorSprite.fillTriangle(halfScrWidth, scissorLength, halfScrWidth + scissorWidth, scissorLength, halfScrWidth + (scissorWidth / 2), 0, WHITE);

  //  Downward pointing part
  scissorSprite.fillTriangle(halfScrWidth - scissorWidth, scissorLength, halfScrWidth, scissorLength, halfScrWidth - (scissorWidth / 2), scissorLength * 2, WHITE);
  scissorSprite.fillTriangle(halfScrWidth, scissorLength, halfScrWidth + scissorWidth, scissorLength, halfScrWidth + (scissorWidth / 2), scissorLength * 2, WHITE);

  //  Draw the handles
  scissorSprite.fillCircle(halfScrWidth - scissorRadius, scissorLength * 2, scissorRadius, WHITE);
  scissorSprite.fillCircle(halfScrWidth + scissorRadius, scissorLength * 2, scissorRadius, WHITE);
}


//  Red circle indicates we are not connected to the network
void WeAreNotConnected (void)
{
  //  Circle Y position and radius
  int circleYR = (screenHeight - playerSize - border1Px) / 2;

  //  Clear screen and display
  M5.Display.fillRect(0, 0, screenWidth, screenHeight - playerSize, BLACK);
  M5.Display.fillCircle(halfScrWidth, circleYR, circleYR, RED);
}


//  Orange circle indicates we are waiting for something
void WeAreWaiting (void)
{
  //  Circle Y position and radius
  int circleYR = (screenHeight - playerSize - border1Px) / 2;

  //  Clear screen and display
  M5.Display.fillRect(0, 0, screenWidth, screenHeight - playerSize, BLACK);
  M5.Display.fillCircle(halfScrWidth, circleYR, circleYR, ORANGE);
}


//  Green circle indicates we are connected to the network
void WeAreConnected (void)
{
  //  Circle Y position and radius
  int circleYR = (screenHeight - playerSize - border1Px) / 2;

  //  Clear screen and display
  M5.Display.fillRect(0, 0, screenWidth, screenHeight - playerSize, BLACK);
  M5.Display.fillCircle(halfScrWidth, circleYR, circleYR, GREEN);
}


//  Scan available networks to find players
String ScanNetworks (String ownSSID)
{
  std::set <String> networks;

  //  Insert our own SSID in to the set in case
  //  we don't come up in the scan
  networks.insert(ownSSID);

  //  We need 2 RPS players
  while (networks.size() != 2)
  {
    int numNetworks = WiFi.scanNetworks();
    if (numNetworks == 0)
    {
      //  This can't be right
      M5.Display.fillScreen(BLUE);
      while (true)
      {
        delay(wifiDelay);
      }
    }

    for (int i = 0; i < numNetworks; i ++)
    {
      if (WiFi.SSID(i).indexOf(ssidPrefix) != -1)
      {
        //  Add the network
        networks.insert(WiFi.SSID(i));

        if (networks.size() >= 2)
        {
          //  We found enough networks
          break;
        }
      }
    }

    delay(wifiDelay);
  }

  //  The networks we found will be sorted, so pick 
  //  the first one
  return *networks.begin();
}


//  Create a network access point
void CreateConnection (void)
{
  //  WiFi IP addresses
  IPAddress APIP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

  //  Let the user know we are not connected yet
  WeAreNotConnected();

  //  Based on an M5 demo, we will create a WiFi access point based on
  //  a unique ID from this M5AtomS3
  uint64_t  chipID  = ESP.getEfuseMac();
  uint32_t  uniqID  = chipID >> 32;
  String    ssidStr = ssidPrefix + String(uniqID, HEX);

  //  Hopefully seed a different random number sequence than the other devide
  srand(uniqID);

  //  Create an AP on this device and allow scanning
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.softAPConfig(APIP, gateway, subnet);
  WiFi.softAP(ssidStr.c_str(), ssidPassword.c_str());

  //  Now scan for other servers
  String connectTo = ScanNetworks(ssidStr);

  //  Let the user know were are waiting
  WeAreWaiting();

  //  Am I the server
  if (connectTo.equals(ssidStr))
  {
    //  Yes - set up the access point
    ourServer = new WiFiServer(serverPort);
    ourServer->begin();

    while (true)
    {
      //  See if we have a client yet
      serverClient = ourServer->accept();
      if (serverClient)
      {
        ourClient = &serverClient;
        break;
      }

      //  Wait a bit
      delay(wifiDelay);
    }
  }
  else
  {
    //  The client has to be reset to station mode
    WiFi.mode(WIFI_MODE_STA);

    //  Start the connection
    WiFi.begin(connectTo.c_str(), ssidPassword.c_str());

    //  Wait for connection to complete
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(wifiDelay);
    }

    //  Connect to the server
    ourClient = new WiFiClient();
    while (!ourClient->connect(APIP, serverPort))
    {
      delay(wifiDelay);
    }
  }

  //  Let the user know we are connected
  WeAreConnected();
}

//  Check network connection
//
//  This is only called after we know we connected to a network
//
bool CheckConnected (void)
{
  if (nextConnectionTime > millis())
  {
    //  Not time to check the connection status
    return true;
  }

  //  Set the next connection check time
  nextConnectionTime = millis() + connectionTime;

  if (ourClient != nullptr
  &&  ourClient->connected())
  {
    //  Everything is connected
    return true;
  }

  //  Something is wrong. Drop any current client, and go back 
  //  to the create connections stage
  if (ourClient != nullptr)
  {
    //  Stop the client
    ourClient->stop();
    delete(ourClient);
    ourClient = nullptr;
  }
  
  if (ourServer != nullptr)
  {
    //  Remove any server
    delete ourServer;
    ourServer = nullptr;
  }

  //  Reset WiFi
  WiFi.disconnect();

  //  Clear the screen
  M5.Display.fillScreen(BLACK);

  //  Restart network connections
  currentState = createConnection;
  return false;
}


//  Determine if I won or not
winState DidIWin (RPSSelection mySelection, RPSSelection otherSelection)
{
  //  We could have a draw
  if (mySelection == otherSelection)
  {
    return draw;
  }

  //  Feeling lucky
  winState retVal = win;

  switch (mySelection)
  {
    case selectRock:
      if (otherSelection == selectPaper)
      {
        //  I lose
        retVal = lose;
      }
      break;

    case selectPaper:
      if (otherSelection == selectScissors)
      {
        //  I lose
        retVal = lose;
      }
      break;

    case selectScissors:
      if (otherSelection == selectRock)
      {
        //  I lose
        retVal = lose;
      }
      break;
  }

  return retVal;
}


//  Display the current selected shape
void DisplayCurrentSelection (void)
{
  switch (currentSelection.at(selectionIndex))
  {
    case selectRock:
      rockSprite.pushSprite(0, 0);
      break;

    case selectPaper:
      paperSprite.pushSprite(0, 0);
      break;

    case selectScissors:
      scissorSprite.pushSprite(0, 0);
      break;
  }
}


//  Initialization
void setup (void)
{
  M5.begin();

  //  Configure the display
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextSize(2);

  //  Get screen dimensions
  screenWidth   = M5.Display.width();
  screenHeight  = M5.Display.height();

  //  Half screen dimensions
  halfScrWidth  = screenWidth / 2;
  halfScrHeight = screenHeight / 2;

  //  Move onto running the demo
#if (RUN_DEMO)
  currentState  = initDemo;
#else   //  RUN_DEMO 
  currentState  = createConnections;
#endif  //  RUN_DEMO
}


//  Main processing loop
void loop (void)
{
  M5.update();

  //  Check current state of the program
  switch (currentState)
  {
#if (RUN_DEMO)    
    //  Run the demo
    case initDemo:
    case tiltScreenDemo:
    case pushScreenDemo:
      RunDemo();
      break;
#endif  //  RUN_DEMO

    //  Create connection
    case createConnection:
      CreateConnection();

      //  We can move to the next stage
      currentState = createPlayers;
      break;

    //  Create player information
    case createPlayers:
      //  Clear the main display and output player information
      CreatePlayerSprites();

      //  Create the win, lose and draw sprites
      CreateWLDSprites();

      //  Create the rock, paper and scissors sprites
      CreateRPSSprites();

      //  Move to the selection stage
      currentState = randomShapes;
      break;

    //  Create a random sequence of shapes
    //
    //  This prevents players from assuming the order is always rock, paper and
    //  scissors. They could count the number of tilts of the other player, if 
    //  we kept this order, and win more than they should
    //
    case randomShapes:
    {
      if (!CheckConnected())
      {
        //  Not connecting - restart networking
        return;
      }

      //  Clear out previous selections
      currentSelection.clear();

      //  Make a random list of selections
      int numSelected = 0;
      while (true)
      {
        //  Get a random number in range 0..max-1
        RPSSelection selected = RPSSelection(rand() % maxSelection);

        //  See if this is in the list already
        if (std::find(currentSelection.begin(), currentSelection.end(), selected) == currentSelection.end())
        {
          //  Add the selection to the list
          currentSelection.push_back(selected);

          //  Did we hit the limit?
          if (currentSelection.size() == maxSelection)
          {
            break;
          }
        }
      }

      //  Next stage - select a shape
      selectionIndex    = 0;
      nextSelectionTime = 0;
      currentState      = selectShapes;

      //  Display the current shape
      DisplayCurrentSelection();

      break;
    }
  
    case selectShapes:
    {
      if (!CheckConnected())
      {
        //  Not connecting - restart networking
        return;
      }

      if (M5.BtnA.wasPressed())
      {
        //  Move to score update
        currentState = playerUpdate;
        WeAreWaiting();
        return;
      }      

      //  Is it time for the next selection tilt?
      if (nextSelectionTime > millis())
      {
        //  No - wait a little more
        //
        //  This prevents unwanted selections
        //
        return;
      }
      
      //  See if the user tilted the screen
      auto IMUUpdate = M5.Imu.update();
      if (IMUUpdate)
      {
        //  Get the IMU data
        auto IMUData = M5.Imu.getImuData();

        //  We are looking for a significant change to the Gryo Z axis
        if (IMUData.gyro.z > 100.0)
        {
          //  Move to the next shape
          selectionIndex ++;
          if (selectionIndex >= maxSelection)
          {
            //  Back to the first shape if we went
            //  too far
            selectionIndex = 0;
          }

          //  Display the shape
          DisplayCurrentSelection();

          //  Next time we allow a selection
          nextSelectionTime = millis() + selectionTime;
        }
      }

      break;
    }

    case playerUpdate:
    {
      //  Check the connection status
      if (!CheckConnected())
      {
        //  Not connecting - restart networking
        return;
      }

      //  Am I the server side?
      winState matchResult;
      if (ourServer != nullptr)
      {
        //  I am the server - send my selection
        //
        //  Note: The local device is alway player 1
        //
        ourClient->write((uint8_t) currentSelection [selectionIndex]);

        //  Wait for our response
        while (!ourClient->available())
        {
          //  Short delay
          delay(wifiDelay);
        }

        //  Read the response
        RPSSelection theirSelection = RPSSelection(ourClient->read());

        //  Did I win?
        matchResult = DidIWin(currentSelection [selectionIndex], theirSelection);
      }
      else
      {
        //  Client side - wait for input from the server
        while (!ourClient->available())
        {
          //  Short delay
          delay(wifiDelay);
        }

        //  Read the input
        RPSSelection theirSelection = RPSSelection(ourClient->read());

        ourClient->write((uint8_t) currentSelection [selectionIndex]);

        //  Did I win?
        matchResult = DidIWin(currentSelection [selectionIndex], theirSelection);
      }

      switch (matchResult)
      {
        case win:
          winSprite.pushSprite(0, 0);
          playerScores [player1Index].playerScore ++;
          break;

        case lose:
          loseSprite.pushSprite(0, 0);
          playerScores [player2Index].playerScore ++;
          break;

        case draw:
          drawSprite.pushSprite(0, 0);
          break;
      }

      //  Update scores
      UpdateScores();

      //  Take in the result
      delay(resultDelay);

      //  Update rounds and see if we have a champion
      UpdateRounds();
      IsThereAChampion();

      //  Move back to the random selection stage
      currentState = randomShapes;

      break;
    }
  }
}