#include "MenuScene.h"

MenuScene::MenuScene() : Scene{ { 0.0f, 0.0f }, "#000000" } {}

MenuScene::MenuScene(Vector2 origin, const char *bgHexCode)
    : Scene{ origin, bgHexCode }
{}

MenuScene::~MenuScene() { shutdown(); }

void MenuScene::initialise()
{
    mGameState.nextSceneID = NO_SCENE;
}

// nothing moves here --> main handles enter
void MenuScene::update(float deltaTime) {}

void MenuScene::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    DrawText("My Little Pony: The Changeling Attack", static_cast<int>(mOrigin.x - MeasureText("My Little Pony: The Changeling Attack", 48) / 2.0f),
             static_cast<int>(mOrigin.y + (-110)), 48, WHITE);
    DrawText("Press enter to start", static_cast<int>(mOrigin.x - MeasureText("Press enter to start", 32) / 2.0f),
             static_cast<int>(mOrigin.y + (10)), 32, WHITE);
    DrawText("A / D to move, W to jump", static_cast<int>(mOrigin.x - MeasureText("A / D to move, W to jump", 22) / 2.0f),
             static_cast<int>(mOrigin.y + (80)), 22, LIGHTGRAY);
}

void MenuScene::shutdown() {}
