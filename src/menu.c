
#include "game.h"
#include "menu.h"

static int toUpdate = 1;

enum option {
    start,
    settings
};

static enum option selected = start;

static void buttonUp(void) {
    sys->logToConsole("Up pressed");
    if (selected == start)
        selected = settings;
    else
        selected = start;
    toUpdate = 1;
}

static void buttonDown(void) {
    sys->logToConsole("Down pressed");
    if (selected == start)
        selected = settings;
    else
        selected = start;
    toUpdate = 1;
}

static void buttonA(void) {
    if (selected == start) {
        Game.gState = State_InGame;
    }
}

static PDButtons pushed;
static void handleButtonPush(void) {
    sys->getButtonState(NULL, &pushed, NULL);

    if (pushed & kButtonUp)
        buttonUp();
    if (pushed & kButtonDown)
        buttonDown();
    if (pushed & kButtonA)
        buttonA();
}

static void displayTitle(void) {
    //ww = sys->formatString(&score, "%d", Game.StackzData.score);
    //= sprintf("%d", Game.StackzData.score);
    //scorewidth = gfx->getTextWidth(Game.font40, , strlen(score), kASCIIEncoding, 0);
    gfx->setFont(Game.font40);
    gfx->drawText("STACKZ", strlen("STACKZ"), kASCIIEncoding, SCREEN_WIDTH / 2 - 185, SCREEN_HEIGHT / 2 - 50);

    //sys->realloc(score, 0);
    //gfx->drawText("0", strlen("0"), kASCIIEncoding, SCREEN_WIDTH/2, 15);
}

static void displayOptions(void) {
    gfx->setFont(Game.font14);
    gfx->drawText("Start", strlen("Start"), kASCIIEncoding, SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 + 20);
    gfx->drawText("Settings", strlen("Settings"), kASCIIEncoding, SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 + 50);
}

static void displaySelector(void) {
    gfx->setFont(Game.font20);
    if (selected == start) {
        gfx->setDrawMode(kDrawModeInverted);
        gfx->drawText(">", strlen(">"), kASCIIEncoding, SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 + 48);
        gfx->setDrawMode(kDrawModeCopy);
        gfx->drawText(">", strlen(">"), kASCIIEncoding, SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 + 18);
    }
    else {
        gfx->setDrawMode(kDrawModeInverted);
        gfx->drawText(">", strlen(">"), kASCIIEncoding, SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 + 18);
        gfx->setDrawMode(kDrawModeCopy);
        gfx->drawText(">", strlen(">"), kASCIIEncoding, SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 + 48);
    }
    //gfx->drawText(">", strlen(">"), kASCIIEncoding, SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 + 18);
    //gfx->drawText("Settings", strlen("Settings"), kASCIIEncoding, SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 + 50);
}

static void updateIfNeeded(void) {
    if(toUpdate > 0) {
        displayTitle();
        displayOptions();
        displaySelector();
        toUpdate = 0;
    }
    handleButtonPush();
}

void updateMenu(void) {
    updateIfNeeded();
}