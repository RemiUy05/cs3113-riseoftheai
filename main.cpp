/**

* Author: Remi Uy

* Assignment: Rise of the AI

* Date due: 11/16/2026

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/

#include "CS3113/LevelA.h"
#include "CS3113/LevelB.h"
#include "CS3113/LevelC.h"
#include "CS3113/MenuScene.h"
#include "CS3113/LoseScene.h"
#include "CS3113/WinScene.h"

// Global Constants
constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120;

constexpr Vector2 ORIGIN = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

// lives --> whole game (not per level)
constexpr int STARTING_LIVES = 3;

// Global Variables
AppStatus gAppStatus       = RUNNING;
float     gPreviousTicks   = 0.0f,
          gTimeAccumulator = 0.0f;

int gLives = STARTING_LIVES;

// sounds loaded once here --> survive level restarts
Sound gJumpSound = {};
Sound gHitSound  = {};
Sound gStarSound = {};
Music gBGM       = {};

Camera2D gCamera = {};

Scene *gCurrentScene = nullptr;
std::map<SceneID, Scene*> gLevels = {};

// Function Declarations
void switchToScene(Scene *scene);
void initialise();
void processInput();
void update();
void render();
void shutdown();

void switchToScene(Scene *scene)
{
    if (gCurrentScene) gCurrentScene->shutdown(); // free the old scene's resources
    gCurrentScene = scene;
    gCurrentScene->initialise();

    // camera starts on player (clamped to level)
    // no player (menu/lose/win) --> window centre
    gCamera.target = { ORIGIN.x, ORIGIN.y };

    if (gCurrentScene->getState().player != nullptr)
    {
        gCamera.target.x = fminf(
            fmaxf(gCurrentScene->getState().player->getPosition().x,
                  gCurrentScene->getState().map->getLeftBoundary() + ORIGIN.x),
            gCurrentScene->getState().map->getRightBoundary() - ORIGIN.x
        );
    }

    // level load takes a sec --> dont count it as game time
    gPreviousTicks   = static_cast<float>(GetTime());
    gTimeAccumulator = 0.0f;
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Rise of the AI");
    InitAudioDevice();

    gJumpSound = LoadSound("assets/game/balloon_pop.wav");
    gHitSound  = LoadSound("assets/game/changeling_hit.wav");
    gStarSound = LoadSound("assets/game/star_piece.wav");

    gBGM = LoadMusicStream("assets/game/discord.wav");
    SetMusicVolume(gBGM, 0.33f);
    PlayMusicStream(gBGM); // music loops by default

    gLevels[LEVEL_A] = new LevelA(ORIGIN, "#C0897E");
    gLevels[LEVEL_B] = new LevelB(ORIGIN, "#011627");
    gLevels[LEVEL_C] = new LevelC(ORIGIN, "#2E294E");
    gLevels[MENU]    = new MenuScene(ORIGIN, "#1B1B3A");
    gLevels[LOSE]    = new LoseScene(ORIGIN, "#5C1A1B");
    gLevels[WIN]     = new WinScene(ORIGIN, "#7B2CBF");

    gCamera.offset   = ORIGIN;
    gCamera.rotation = 0.0f;
    gCamera.zoom     = 1.0f;

    // game starts on the menu
    switchToScene(gLevels[MENU]);

    SetTargetFPS(FPS);
}

void processInput()
{
    // only levels have a player --> menu/lose/win skip this
    Entity *player = gCurrentScene->getState().player;

    if (player != nullptr)
    {
        player->resetMovement();

        if      (IsKeyDown(KEY_A)) player->moveLeft();
        else if (IsKeyDown(KEY_D)) player->moveRight();

        if (IsKeyPressed(KEY_W) && player->isCollidingBottom())
        {
            player->jump();
            PlaySound(gJumpSound);
        }

        // to avoid faster diagonal speed
        if (GetLength(player->getMovement()) > 1.0f)
            player->normaliseMovement();
    }

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;

    // switching scenes deletes old player --> do it last

    // enter --> menu starts game, lose/win goes back to menu
    // lives reset either way
    if (IsKeyPressed(KEY_ENTER))
    {
        if (gCurrentScene == gLevels[MENU])
        {
            PlaySound(gStarSound);
            gLives = STARTING_LIVES;
            switchToScene(gLevels[LEVEL_A]);
            return;
        }
        else if (gCurrentScene == gLevels[LOSE] || gCurrentScene == gLevels[WIN])
        {
            PlaySound(gStarSound);
            gLives = STARTING_LIVES;
            switchToScene(gLevels[MENU]);
            return;
        }
    }

    // debug --> 1/2/3 jumps to that level
    // (same level --> restarts it)
    if      (IsKeyPressed(KEY_ONE))   switchToScene(gLevels[LEVEL_A]);
    else if (IsKeyPressed(KEY_TWO))   switchToScene(gLevels[LEVEL_B]);
    else if (IsKeyPressed(KEY_THREE)) switchToScene(gLevels[LEVEL_C]);
}

void update()
{
    // Delta time
    float ticks     = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    // Fixed timestep
    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        UpdateMusicStream(gBGM);

        // Look at how beautiful this is:
        gCurrentScene->update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;

        // check collisions from player's side
        // (menu/lose/win --> no enemies)
        GameState state = gCurrentScene->getState();

        for (int i = 0; i < state.enemyCount; i++)
        {
            if (state.player->isColliding(state.enemies[i]))
            {
                PlaySound(gHitSound);
                gLives--;

                // 0 lives --> lose scene, else restart level
                if (gLives <= 0) switchToScene(gLevels[LOSE]);
                else             switchToScene(gCurrentScene);

                break;
            }
        }

        // scene changed --> grab state again
        state = gCurrentScene->getState();

        // horizontal-only tracking
        // clamped to level --> no empty space past the edges
        Vector2 currentPlayerPosition = { ORIGIN.x, ORIGIN.y };

        if (state.player != nullptr)
        {
            currentPlayerPosition.x = fminf(
                fmaxf(state.player->getPosition().x,
                      state.map->getLeftBoundary() + ORIGIN.x),
                state.map->getRightBoundary() - ORIGIN.x
            );
        }

        panCamera(&gCamera, &currentPlayerPosition);
    }

    gTimeAccumulator = deltaTime;
}

void render()
{
    BeginDrawing();

    BeginMode2D(gCamera);

    gCurrentScene->render();

    EndMode2D();

    // hud --> lives & level #, levels only
    if (gCurrentScene->getState().map != nullptr)
    {
        // which level? (LEVEL_A = 0 --> +1)
        int levelNumber = 0;
        for (std::pair<const SceneID, Scene *> &entry : gLevels)
            if (entry.second == gCurrentScene) levelNumber = entry.first + 1;

        DrawText(TextFormat("Lives: %d", gLives), 20, 20, 30, BLACK);
        DrawText(TextFormat("Level %d   (press 1, 2 or 3 to switch)", levelNumber),
                 20, 56, 20, BLACK);
    }

    EndDrawing();
}

void shutdown()
{
    for (std::pair<const SceneID, Scene *> &entry : gLevels)
        delete entry.second;
    gLevels.clear();

    UnloadSound(gJumpSound);
    UnloadSound(gHitSound);
    UnloadSound(gStarSound);
    UnloadMusicStream(gBGM);

    CloseAudioDevice();
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();

        if (gCurrentScene->getState().nextSceneID != NO_SCENE)
        {
            SceneID id = gCurrentScene->getState().nextSceneID;

            // asking for a different scene --> star touched
            // (fell off floor --> same level again, no sound)
            if (gLevels[id] != gCurrentScene) PlaySound(gStarSound);

            switchToScene(gLevels[id]);
        }

        render();
    }

    shutdown();

    return 0;
}
