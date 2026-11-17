#include "LoseScene.h"

LoseScene::LoseScene() : Scene{ { 0.0f, 0.0f }, "#000000" } {}

LoseScene::LoseScene(Vector2 origin, const char *bgHexCode)
    : Scene{ origin, bgHexCode }
{}

LoseScene::~LoseScene() { shutdown(); }

void LoseScene::initialise()
{
    mGameState.nextSceneID = NO_SCENE;
}

// nothing moves here --> main handles enter
void LoseScene::update(float deltaTime) {}

void LoseScene::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    DrawText("You Lose", static_cast<int>(mOrigin.x - MeasureText("You Lose", 72) / 2.0f),
             static_cast<int>(mOrigin.y + (-90)), 72, WHITE);
    DrawText("Press enter to go back to the menu", static_cast<int>(mOrigin.x - MeasureText("Press enter to go back to the menu", 28) / 2.0f),
             static_cast<int>(mOrigin.y + (20)), 28, WHITE);
}

void LoseScene::shutdown() {}
