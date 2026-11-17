#include "LevelC.h"

constexpr Vector2 C_PLAYER_START   = {   24.0f, 505.0f }; // pink mark --> start
constexpr Vector2 C_ENDPOINT_START = {  150.0f,  44.0f }; // green mark --> star

constexpr int C_NUMBER_OF_ENEMIES = 5;

constexpr EnemySpawn C_ENEMY_SPAWNS[C_NUMBER_OF_ENEMIES] = {
    {   635.0f,  157.0f, JUMPER  ,     500.0f }, // long platform left end --> hops up/down
    {   900.0f,  154.0f, WANDERER,       0.0f }, // long platform --> paces whole length
    {   700.0f,  514.0f, FOLLOWER, 100000.0f }, // bottom-left --> follows x between pillars
    {  1700.0f,  514.0f, FOLLOWER, 100000.0f }, // bottom-right --> follows x past middle pillar
    {  1915.0f,   30.0f, LERPER  ,     215.0f }, // top-right --> flyer, wakes inside red arc
};

LevelC::LevelC() : Scene{ { 0.0f, 0.0f }, "#000000" } {}

LevelC::LevelC(Vector2 origin, const char *bgHexCode)
    : Scene{ origin, bgHexCode }
{}

LevelC::~LevelC() { shutdown(); }

void LevelC::initialise()
{
    mGameState.nextSceneID = NO_SCENE;

    mGameState.background = LoadTexture("assets/game/canterlotbg.png");

    // map
    mGameState.map = new Map(
        LEVEL_C_WIDTH, LEVEL_C_HEIGHT,   // map grid cols & rows
        mLevelData,                  // grid data
        "assets/game/ground.png",    // texture filepath
        TILE_DIMENSION,              // tile size
        1, 1,                        // texture cols & rows
        mOrigin                      // in-game origin
    );

    // player
    std::map<Direction, std::vector<int>> ponyAnimationAtlas = {
        { DOWN,  { 51, 52, 53 } },
        { LEFT,  { 63, 64, 65 } },
        { RIGHT, { 75, 76, 77 } },
        { UP,    { 87, 88, 89 } },
    };

    mGameState.player = new Entity(
        { mGameState.map->getLeftBoundary() + C_PLAYER_START.x,
          mGameState.map->getTopBoundary()  + C_PLAYER_START.y }, // position
        { 60.0f, 60.0f },                       // scale
        "assets/game/ponysprites.png",          // texture file address
        ATLAS,                                  // single image or atlas?
        { 8, 12 },                              // atlas dimensions --> rows, cols
        ponyAnimationAtlas,                     // actual atlas
        PLAYER                                  // entity type
    );

    // small enough for low platforms --> jump power = just enough
    // to climb the steps
    mGameState.player->setJumpingPower(560.0f);
    mGameState.player->setFrameSpeed(8);
    mGameState.player->setColliderDimensions({ 28.0f, 48.0f });
    mGameState.player->setAcceleration({ 0.0f, ACCELERATION_OF_GRAVITY });

    // enemies
    std::map<Direction, std::vector<int>> changelingAnimationAtlas = {
        { LEFT,  {  7,  8,  9, 10, 11, 12, 13 } },
        { RIGHT, { 14, 15, 16, 17, 18, 19, 20 } },
    };

    // rainbow dash (flyer)
    std::map<Direction, std::vector<int>> rainbowDashAnimationAtlas = {
        { LEFT,  { 60, 61, 62 } },
        { RIGHT, { 72, 73, 74 } },
    };

    // pinkie pie (jumper)
    std::map<Direction, std::vector<int>> pinkieAnimationAtlas = {
        { LEFT,  { 18, 19, 20 } },
        { RIGHT, { 30, 31, 32 } },
    };

    mGameState.enemyCount = C_NUMBER_OF_ENEMIES;

    for (int i = 0; i < C_NUMBER_OF_ENEMIES; i++)
    {
        // sprite sheet --> depends on ai type
        const char *texture = "assets/game/changelings.png";
        Vector2     scale   = { 64.0f, 64.0f };
        Vector2     atlasDimensions = { 4, 7 };
        std::map<Direction, std::vector<int>> *atlas = &changelingAnimationAtlas;

        if (C_ENEMY_SPAWNS[i].type == LERPER)
        {
            texture         = "assets/game/ponysprites.png";
            scale           = { 60.0f, 60.0f };
            atlasDimensions = { 8, 12 };
            atlas           = &rainbowDashAnimationAtlas;
        }
        else if (C_ENEMY_SPAWNS[i].type == JUMPER)
        {
            texture         = "assets/game/ponysprites.png";
            scale           = { 60.0f, 60.0f };
            atlasDimensions = { 8, 12 };
            atlas           = &pinkieAnimationAtlas;
        }

        mGameState.enemies[i] = new Entity(
            { mGameState.map->getLeftBoundary() + C_ENEMY_SPAWNS[i].x,
              mGameState.map->getTopBoundary()  + C_ENEMY_SPAWNS[i].y }, // position
            scale,
            texture,
            ATLAS,
            atlasDimensions,
            *atlas,
            NPC
        );

        Entity *enemy = mGameState.enemies[i];

        enemy->setAIType(C_ENEMY_SPAWNS[i].type);
        enemy->setAIState(IDLE);
        enemy->setDirection(LEFT);
        enemy->setFrameSpeed(10);

        switch (C_ENEMY_SPAWNS[i].type)
        {
        case LERPER:
            // flyer --> no gravity, wakes up when player in range
            enemy->setColliderDimensions({ 34.0f, 34.0f });
            enemy->setFrameSpeed(8);
            enemy->setDetectionRange(C_ENEMY_SPAWNS[i].range);
            break;

        case WANDERER:
            // wanderer --> paces back & forth, turns at ledges
            enemy->setAcceleration({ 0.0f, ACCELERATION_OF_GRAVITY });
            enemy->setColliderDimensions({ 30.0f, 50.0f });
            enemy->setSpeed(Entity::DEFAULT_SPEED * 0.50f);
            break;

        case JUMPER:
            // jumper --> hops in place, last col of spawn table = jump power
            // (0 --> default 450, ~105px high)
            enemy->setAcceleration({ 0.0f, ACCELERATION_OF_GRAVITY });
            enemy->setColliderDimensions({ 28.0f, 46.0f });
            enemy->setJumpingPower(C_ENEMY_SPAWNS[i].range > 0.0f ? C_ENEMY_SPAWNS[i].range : 450.0f);
            enemy->setFrameSpeed(8);
            break;

        case FOLLOWER:
            // follower --> walks toward player once in range
            // (huge range = from anywhere)
            // pillars & ledges --> stays on its own platform
            enemy->setAcceleration({ 0.0f, ACCELERATION_OF_GRAVITY });
            enemy->setColliderDimensions({ 30.0f, 50.0f });
            enemy->setSpeed(Entity::DEFAULT_SPEED * 0.70f);
            enemy->setDetectionRange(C_ENEMY_SPAWNS[i].range);
            break;

        default:
            break;
        }
    }

    // endpoint
    mGameState.endpoint = new Entity(
        { mGameState.map->getLeftBoundary() + C_ENDPOINT_START.x,
          mGameState.map->getTopBoundary()  + C_ENDPOINT_START.y },
        { 59.0f, 76.0f },
        "assets/game/endpoint.png",
        BLOCK
    );
}

void LevelC::update(float deltaTime)
{
    mGameState.player->update(
        deltaTime,      // delta time / fixed timestep
        nullptr,        // player
        mGameState.map, // map
        nullptr,        // collidable entities
        0               // col. entity count
    );

    // player can't leave level --> clamp x to left/right edges,
    // y to top edge
    Vector2 playerPosition = mGameState.player->getPosition();
    float   playerHalfWidth  = mGameState.player->getColliderDimensions().x / 2.0f;
    float   playerHalfHeight = mGameState.player->getColliderDimensions().y / 2.0f;

    playerPosition.x = fminf(
        fmaxf(playerPosition.x, mGameState.map->getLeftBoundary() + playerHalfWidth),
        mGameState.map->getRightBoundary() - playerHalfWidth
    );
    playerPosition.y = fmaxf(
        playerPosition.y, mGameState.map->getTopBoundary() + playerHalfHeight
    );

    mGameState.player->setPosition(playerPosition);

    for (int i = 0; i < mGameState.enemyCount; i++)
    {
        mGameState.enemies[i]->update(
            deltaTime,          // delta time / fixed timestep
            mGameState.player,  // player --> ai target
            mGameState.map,     // map
            nullptr,            // collidable entities
            0                   // col. entity count
        );
    }

    // star touched --> next scene
    if (mGameState.player->isColliding(mGameState.endpoint))
        mGameState.nextSceneID = WIN;

    // fell off floor --> restart level (no life lost)
    if (mGameState.player->getPosition().y > END_GAME_THRESHOLD)
        mGameState.nextSceneID = LEVEL_C;
}

void LevelC::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    // background --> scaled to level height, tiled across level width
    // (every other copy mirrored --> seams line up)
    float levelWidth  = mGameState.map->getRightBoundary()  - mGameState.map->getLeftBoundary();
    float levelHeight = mGameState.map->getBottomBoundary() - mGameState.map->getTopBoundary();
    float copyWidth   = mGameState.background.width * (levelHeight / mGameState.background.height);

    int copies = static_cast<int>(ceilf(levelWidth / copyWidth));

    for (int i = 0; i < copies; i++)
    {
        float flip = (i % 2 == 0) ? 1.0f : -1.0f; // negative width --> mirrored

        DrawTexturePro(
            mGameState.background,
            { 0.0f, 0.0f, flip * mGameState.background.width,
              static_cast<float>(mGameState.background.height) },
            { mGameState.map->getLeftBoundary() + i * copyWidth,
              mGameState.map->getTopBoundary(), copyWidth, levelHeight },
            { 0.0f, 0.0f },
            0.0f,
            WHITE
        );
    }

    mGameState.map->render();

    mGameState.endpoint->render();

    for (int i = 0; i < mGameState.enemyCount; i++)
        mGameState.enemies[i]->render();

    mGameState.player->render();
}

// safe to call twice --> every ptr nulled after delete
void LevelC::shutdown()
{
    delete mGameState.player;
    mGameState.player = nullptr;

    for (int i = 0; i < mGameState.enemyCount; i++)
    {
        delete mGameState.enemies[i];
        mGameState.enemies[i] = nullptr;
    }
    mGameState.enemyCount = 0;

    delete mGameState.endpoint;
    mGameState.endpoint = nullptr;

    delete mGameState.map;
    mGameState.map = nullptr;

    UnloadTexture(mGameState.background);
    mGameState.background = {};
}
