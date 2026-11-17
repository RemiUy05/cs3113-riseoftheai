/**
* Author: [Your name here]
* Assignment: Rise of the AI
* Date due: 11/16/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "WinScene.h"

WinScene::WinScene() : Scene{ { 0.0f, 0.0f }, "#000000" } {}

WinScene::WinScene(Vector2 origin, const char *bgHexCode)
    : Scene{ origin, bgHexCode }
{}

WinScene::~WinScene() { shutdown(); }

void WinScene::initialise()
{
    mGameState.nextSceneID = NO_SCENE;
}

// nothing moves here --> main handles enter
void WinScene::update(float deltaTime) {}

void WinScene::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    DrawText("You Win", static_cast<int>(mOrigin.x - MeasureText("You Win", 72) / 2.0f),
             static_cast<int>(mOrigin.y + (-90)), 72, WHITE);
    DrawText("Press enter to go back to the menu", static_cast<int>(mOrigin.x - MeasureText("Press enter to go back to the menu", 28) / 2.0f),
             static_cast<int>(mOrigin.y + (20)), 28, WHITE);
}

void WinScene::shutdown() {}
