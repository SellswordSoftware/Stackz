#ifndef _GAME_H_
#define _GAME_H_

#include <stdio.h>
#include "pd_api.h"
#include "mini3d.h"
#include "3dmath.h"
#include "shape.h"
#include "scene.h"

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 240

#define STACKMAX 20

extern const struct playdate_graphics *gfx;
extern const struct playdate_sys *sys;
extern const struct playdate_display *display;
extern const struct playdate_file *file;
extern const struct playdate_sound *sound;

typedef enum
{
    State_SplashScreen,
    State_Menu,
    State_InGame,
    State_Scoreboard,
    State_Tutorial
} GameScene;

struct Node {
    Scene3DNode *scene3DNode;
    struct Node *next;
};

struct BoxSize {
    float width;
    float depth;
};

typedef struct
{
    PlaydateAPI *gPd;
    GameScene gState;

    const char* fontpath;
    LCDFont* font;
    LCDFont* font14;
    LCDFont* font20;
    LCDFont* font40;

    struct
    {
        Scene3D *scene;
        Shape3D *activebox1;

        Scene3DNode *rootNode;
        Scene3DNode *stackNode;
        Scene3DNode *activeNode;

        Shape3D *stackBoxes[STACKMAX];
        Shape3D *activeBox;
        struct Node *firstNode;
        struct Node *lastNode;
        struct Node *currentNode;

        Matrix3D activeNodeMatrix;
        Matrix3D stackNodeMatrix;
        Matrix3D *crankMatrix;
        Matrix3D *updownMatrix;

        int stackBoxIndex;
        float ellapsed;
        float crankChange;
        float activeOscillator;
        int score;

        int gameover;

    } StackzData;

} GameStruct;

extern GameStruct Game;

void InitGame(PlaydateAPI *pd);
int Update(void *userdata);

#endif