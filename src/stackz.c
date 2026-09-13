#include "stackz.h"

#include "lcdpatterns.h"
#include "mini3d.h"
#include "3dmath.h"
#include "shape.h"
#include "scene.h"
#include "3dhelper.h"
#include "float.h"

#define BORDER

static Scene3D *scene;
static Scene3DNode *rootNode;
static Scene3DNode *rotationNode;
static Scene3DNode *activeNode;
static Scene3DNode *activeNodeSubnode;
static Scene3DNode *stackNode;

static Shape3D *activeBox;

static Matrix3D *stackNodeMatrix;
static RenderStyle globalStyle;

static float leftrightrotation = 45.f;
static float updownrotation = 0.f;

static float activeBoxWidth = 1.f;
static float activeBoxDepth = 1.f;
static float targetBoxWidth = 1.f;
static float targetBoxDepth = 1.f;
static float targetBoxX = 0.f;
static float targetBoxZ = 0.f;
static float targetBoxXScale = 1.f;
static float targetBoxZScale = 1.f;
static int isFirstLoop = 0;
static int direction = 0;

static float zoom = 5.f;

float getColorFromIndex(int index) {
    float adjustedIndex = (index + 10) * 0.4f;
    return (sinf((float)adjustedIndex) * 0.5f) - 0.3f;
}

Vector3D v = { 0.f,0.f,0.f };
float addv = 0;
static void addBoxToStack(float x, float z, float scalex, float scalez) {
    Game.StackzData.currentNode = Game.StackzData.currentNode->next;
    node_resetTranform(Game.StackzData.currentNode->scene3DNode);
    Game.StackzData.currentNode->scene3DNode->isVisible = 1;
    Game.StackzData.currentNode->scene3DNode->colorBias = getColorFromIndex(Game.StackzData.stackBoxIndex);
    addv += 0.5f;
    matrix_scaleByAndAddTranslation(Game.StackzData.currentNode->scene3DNode, scalex, 1.f, scalez, x, addv, z);
    Game.StackzData.stackNodeMatrix = matrix_addTranslation(0.f, -0.5f, 0.f);
    Scene3DNode_addTransform(stackNode, stackNodeMatrix);
    Game.StackzData.stackBoxIndex++;
}

static void updateActiveBlockSize(float x, float z, float scalex, float scalez) {
    node_resetTranform(activeNodeSubnode);
    matrix_scaleByAndAddTranslation(activeNodeSubnode, scalex, 1.f, scalez, x, 0.f, z);
}

static void initSceneAndCamera(void) {
    scene = Game.StackzData.scene = scene_new();
    scene_setCameraOrigin(scene, 0.f, zoom*1.f, zoom*1.2f);
	scene_setLight(scene, 0.2f, 0.8f, 0.4f);
    sys->setPeripheralsEnabled(kAccelerometer);
}

static void initNodes(void) {
    rootNode = Game.StackzData.rootNode = Scene3D_getRootNode(scene);
    rotationNode = Game.StackzData.activeNode = Scene3DNode_newChild(rootNode);
    activeNode = Game.StackzData.activeNode = Scene3DNode_newChild(rotationNode);
    activeNodeSubnode = Scene3DNode_newChild(activeNode);
    stackNode = Game.StackzData.stackNode = Scene3DNode_newChild(rotationNode);
    matrix_updateRotation(Game.StackzData.crankMatrix, leftrightrotation, 0.f, 1.f, 0.f);
    Scene3DNode_addTransform(rotationNode, Game.StackzData.crankMatrix);
#ifdef BORDER
    RenderStyle style = Scene3DNode_getRenderStyle(rootNode);
    style |= kRenderWireframe;
    globalStyle = style;
    Scene3DNode_setRenderStyle(rootNode,style);
#endif
}

static void initDataValues(void) {
    mini3d_setRealloc(sys->realloc);
    Game.StackzData.ellapsed=0.f;
    Game.StackzData.activeOscillator=0.f;
    Game.StackzData.score=0;
    Game.StackzData.crankMatrix = matrix_new();
    Game.StackzData.updownMatrix = matrix_new();
}

static void initActiveBox(void) {
    activeBox = Game.StackzData.activeBox = shape_new_cuboid(1.f,0.25f,1.f,-0.1f);
	Scene3DNode_addShape(activeNodeSubnode, activeBox);
}

static void initStack(void) {
    if(Game.StackzData.lastNode == NULL) {
        sys->logToConsole("List is empty");
        return;
    }
    if(Game.StackzData.lastNode->next != Game.StackzData.firstNode) {
        sys->logToConsole("List is not circular empty");
        return;
    }
    struct Node* ptr = Game.StackzData.firstNode;
    do {
        //create shape
        ptr->scene3DNode = Game.StackzData.stackNode = Scene3DNode_newChild(stackNode);
        Shape3D *shape = shape_new_cuboid(1.f,0.25f,1.f,0.0f);
        Scene3DNode_addShape(ptr->scene3DNode, shape);
        Scene3DNode_setVisible(ptr->scene3DNode, 0);

        ptr = ptr->next;
    } while (ptr != Game.StackzData.lastNode->next);
    Game.StackzData.currentNode = Game.StackzData.firstNode;

    Scene3DNode_setVisible(ptr->scene3DNode, 1);
    Game.StackzData.stackNodeMatrix = matrix_addTranslation(0,-0.5,0);
    stackNodeMatrix = &Game.StackzData.stackNodeMatrix;
    Scene3DNode_addTransform(stackNode, stackNodeMatrix);
}

static void resetStack(void) {
    struct Node* ptr = Game.StackzData.firstNode;
    do {
        Scene3DNode_setVisible(ptr->scene3DNode, 0);
        ptr = ptr->next;
    } while (ptr != Game.StackzData.lastNode->next);
    addBoxToStack(0.f, 0.f, 1.f, 1.f);
    
}

static void resetGame(void) {
    Game.StackzData.score = 0;
    Game.StackzData.stackBoxIndex = 0;
    sys->resetElapsedTime();
    direction = 0;
    activeBoxDepth = 1.f;
    activeBoxWidth = 1.f;
    targetBoxDepth = 1.f;
    targetBoxWidth = 1.f;
    targetBoxX = 0.f;
    targetBoxZ = 0.f;
    targetBoxXScale = 1.f;
    targetBoxZScale = 1.f;
    node_resetTranform(activeNodeSubnode);
    resetStack();
}

// Scene Init
void initStackzSceneData(void) {
    initDataValues();
    initSceneAndCamera();
    initNodes();
    initActiveBox();
    initStack();
}

LCDBitmap *bg;
int bgcreated = 0;

static void drawBackground(void)
{
    if(bgcreated == 0) {
        gfx->clear((long unsigned int)LCD_PATTERN_DITHER_GREY60);
        gfx->fillEllipse(-45, -65, 490, 370, 0.0, 0.0, (long unsigned int)LCD_PATTERN_DITHER_GREY50_ALT1);
        gfx->fillEllipse(-40, -60, 480, 360, 0.0, 0.0, (long unsigned int)LCD_PATTERN_DITHER_GREY45);
        gfx->fillEllipse(-35, -55, 470, 350, 0.0, 0.0, (long unsigned int)LCD_PATTERN_DITHER_GREY40);
        gfx->fillEllipse(-30, -50, 460, 340, 0.0, 0.0, (long unsigned int)LCD_PATTERN_DITHER_GREY35);
        gfx->fillEllipse(-25, -45, 450, 330, 0.0, 0.0, (long unsigned int)LCD_PATTERN_DITHER_GREY30);
        gfx->fillEllipse(-20, -40, 440, 320, 0.0, 0.0, (long unsigned int)LCD_PATTERN_DITHER_GREY25);
        gfx->fillEllipse(-15, -35, 430, 310, 0.0, 0.0, (long unsigned int)LCD_PATTERN_DITHER_GREY20);
        gfx->fillEllipse(-10, -30, 420, 300, 0.0, 0.0, (long unsigned int)LCD_PATTERN_DITHER_GREY15);
        gfx->fillEllipse(0, -20, 400, 280, 0.0, 0.0, (long unsigned int)LCD_PATTERN_DITHER_GREY10);
        gfx->fillEllipse(10, -10, 380, 260, 0.0, 0.0, (long unsigned int)LCD_PATTERN_DITHER_GREY05);
        gfx->fillEllipse(20, 0, 360, 240, 0.0, 0.0, kColorWhite);
        bg = gfx->copyFrameBufferBitmap();
        bgcreated=1;
        sys->logToConsole("creating background");
    } else {
        gfx->drawBitmap(bg, 0, 0, kBitmapUnflipped);
    }
}

static void firstLoop(void) {
    if(isFirstLoop == 0)
    {
        isFirstLoop = 1;
		sys->resetElapsedTime();
	}
}

static void buttonUp(void) {
    updownrotation += 0.5f;
    if (updownrotation > 5.f)
        updownrotation = 5.f;
    sys->logToConsole("Up pressed");
}

static void buttonDown(void) {
    updownrotation -= 0.5f;
    if (updownrotation < -5.f)
        updownrotation = -5.f;
    sys->logToConsole("Down pressed");
}

static void buttonLeft(void) {
    leftrightrotation = 10.f;
    matrix_updateRotation(Game.StackzData.crankMatrix, leftrightrotation, 0.f, 1.f, 0.f);
    Scene3DNode_addTransform(rootNode, Game.StackzData.crankMatrix);
}

static void buttonRight(void) {
    leftrightrotation = -10.f;
    matrix_updateRotation(Game.StackzData.crankMatrix, leftrightrotation, 0.f, 1.f, 0.f);
    Scene3DNode_addTransform(rootNode, Game.StackzData.crankMatrix);
}

static void buttonA(void) {
    addBoxToStack(0.f,0.f,1.f,1.f);
}

static void buttonB(void) {
    if (direction == 0) {
        if (Game.StackzData.activeOscillator > (targetBoxDepth * 2.f) + targetBoxX || Game.StackzData.activeOscillator < -(targetBoxDepth*2.f)+targetBoxX) {
            Game.StackzData.gameover = 1;
            return;
        }
        sys->logToConsole("hit");
        Game.StackzData.score++;

        float difference = Game.StackzData.activeOscillator - targetBoxX;
        if (difference < 0.f)
            difference = targetBoxX - Game.StackzData.activeOscillator;

        float pdiff = (difference / (targetBoxDepth * 2.f));
        float scale = 1.f - pdiff;
        targetBoxXScale *= scale;
        targetBoxX = (Game.StackzData.activeOscillator / 2.f) + (targetBoxX / 2.f);

        addBoxToStack(targetBoxX, -targetBoxZ, targetBoxXScale, targetBoxZScale);

        targetBoxDepth -= (difference/2.f);
        
        updateActiveBlockSize(targetBoxX, -targetBoxZ, targetBoxXScale, targetBoxZScale);

    }
    else {
        if (Game.StackzData.activeOscillator > (targetBoxWidth * 2.f) + targetBoxZ || Game.StackzData.activeOscillator < -(targetBoxWidth * 2.f) + targetBoxZ) {
            resetGame();
            return;
        }
        sys->logToConsole("hit");
        Game.StackzData.score++;

        float difference = Game.StackzData.activeOscillator - targetBoxZ;
        if (difference < 0.f)
            difference = targetBoxZ - Game.StackzData.activeOscillator;
        float pdiff = (difference / (targetBoxDepth * 2.f));
        float scale = 1.f - pdiff;
        targetBoxZScale *= scale;
        targetBoxZ = (Game.StackzData.activeOscillator / 2.f) + (targetBoxZ / 2.f);

        addBoxToStack(targetBoxX, -targetBoxZ, targetBoxXScale, targetBoxZScale);

        targetBoxWidth -= (difference / 2.f);

        updateActiveBlockSize(targetBoxX, -targetBoxZ, targetBoxXScale, targetBoxZScale);
    }

    if (direction == 0)
        direction = 1;
    else
        direction = 0;

    sys->resetElapsedTime();
    
    return;

    if(Game.StackzData.activeOscillator > 2.f || Game.StackzData.activeOscillator < -2.f){
        sys->logToConsole("missed\r");
        resetGame();
    } else {
        if (Game.StackzData.activeOscillator < 0.1f && Game.StackzData.activeOscillator > -0.1f)
            sys->logToConsole("perfect\r");

        sys->logToConsole("hit\r");
        
        sys->resetElapsedTime();
        Game.StackzData.score++;

        float difference = Game.StackzData.activeOscillator;
        //sys->logToConsole("difference: %f\r", (double)Game.StackzData.activeOscillator);

        
        if (direction == 0) {
            float scale = (activeBoxDepth - (fabsf(difference)/2.f)) / activeBoxDepth;
            addBoxToStack((Game.StackzData.activeOscillator / 2), 0.f, scale, 1.f);
        }
        else {
            float scale = (activeBoxWidth - (fabsf(difference) / 2.f)) / activeBoxWidth;
            addBoxToStack(0.f, (-Game.StackzData.activeOscillator / 2), 1.f, scale);
        }

        
    }

}

static PDButtons pushed;
static PDButtons current;
static void handleButtonPush(void) {
	sys->getButtonState(&current, &pushed, NULL);
	
	if ( pushed & kButtonUp || current & kButtonUp )
		buttonUp();
    if ( pushed & kButtonDown || current & kButtonDown )
		buttonDown();
    if ( pushed & kButtonLeft || current & kButtonLeft )
		buttonLeft();
    if ( pushed & kButtonRight || current & kButtonRight )
		buttonRight();
    if ( pushed & kButtonA )
		buttonA();
    if ( pushed & kButtonB )
		buttonB();
}

static void setupOscillator(void) {
    Game.StackzData.ellapsed = (sys->getElapsedTime() +2.f) * 1.f;
    Game.StackzData.activeOscillator = sinf(Game.StackzData.ellapsed)*3.f;
}

static void OscillateActiveNode(void) {
    if (direction == 0) {
        Game.StackzData.activeNodeMatrix = matrix_addTranslation(Game.StackzData.activeOscillator,0.f,0.f);
        Scene3DNode_setTransform(activeNode, &Game.StackzData.activeNodeMatrix);
    } else {
        Game.StackzData.activeNodeMatrix = matrix_addTranslation(0.f, 0.f, -Game.StackzData.activeOscillator);
        Scene3DNode_setTransform(activeNode, &Game.StackzData.activeNodeMatrix);
    }
}

static void draw(void) {
    Scene3D_draw(Game.StackzData.scene, gfx->getFrame(), LCD_ROWSIZE);
	gfx->markUpdatedRows(0, LCD_ROWS-1);
}

static char* score;
static int ww = 0;
int scorewidth = 0;
static void displayScore(void) {
    gfx->setFont(Game.font14);
    ww = sys->formatString(&score,"%d",Game.StackzData.score);
    scorewidth = gfx->getTextWidth(Game.font, score, strlen(score),kASCIIEncoding,0);
    gfx->drawText(score, strlen(score), kASCIIEncoding, SCREEN_WIDTH/2-(scorewidth/2), 15);
    sys->realloc(score, 0);
}

static float outBounce(float x) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;
    float tx = 0.f;
    if (x < 1.f / d1) {
        return n1 * x * x;
    } else if (x < 2.f / d1) {
        tx = x - 1.5f / d1;
        return n1 * tx * tx + 0.75f;
    } else if (x < 2.5f / d1) {
        tx = x - 2.25f / d1;
        return n1 * tx * tx + 0.9375f;
    } else {
        tx = x - 2.625f / d1;
        return n1 * tx * tx + 0.984375f;
    }
}

int gameovertextwidth = 0;
float bounce = 0.f;
float startofGanim = 0.f;
int animStarted = 0;
float durationofGainm = 2.f;
float ganimstarty = 0.f;
float gainmendy = SCREEN_HEIGHT / 2 + 20;
static void displayGameOver(void) {
    if (animStarted == 0) {
        sys->resetElapsedTime();
        animStarted = 1;
    }
    gfx->setFont(Game.font20);
    gameovertextwidth = gfx->getTextWidth(Game.font, "Game Over", strlen("Game Over"),kASCIIEncoding,0);
    float progress = sys->getElapsedTime() / durationofGainm;
    if (progress < 1.f) {
        bounce = outBounce(progress);
        float animdif = gainmendy - ganimstarty;
        float curros = animdif * bounce; 
        //sys->logToConsole("progress: %f, bounce: %f, curros: %f", progress, bounce, curros);

        gfx->drawText("Game Over", strlen("Game Over"), kASCIIEncoding, SCREEN_WIDTH/2-(gameovertextwidth/2), curros - 20 );
    } else {
        gfx->drawText("Game Over", strlen("Game Over"), kASCIIEncoding, SCREEN_WIDTH/2-(gameovertextwidth/2), SCREEN_HEIGHT / 2);
    }
}


static float accelx;
static float accely;
static float accelz;
static float smoothing = 0.9f;
static float shiftx = 0.0f;
void handleRotation(void) {
	sys->getAccelerometer(&accelx, &accely, &accelz);
	//pd->system->logToConsole("Accel data: x(%f) y(%f) z(%f)\r", accelx, accely, accelz);
	shiftx = smoothing * shiftx + (1-smoothing) * accelx;
	//pd->system->logToConsole("shiftx: %f\r", shiftx);
	//activebox1->points->x = res;

	//activeNode->transform.dz = res;

	//angle = sys->getCrankChange();

	//matrix_updateRotation(Game.StackzData.updownMatrix, shiftx*90,0.f,0.f,1.f);
	//Scene3DNode_setTransform(rootNode, Game.StackzData.updownMatrix);
}

static void zoomCameraWithCrank(void) {
    Game.StackzData.crankChange = sys->getCrankChange() / 50.f;

    sys->logToConsole("shiftx: %f", (double)shiftx);

    float rotty = shiftx * -5.f;
    zoom -= Game.StackzData.crankChange;
    if (zoom > 10.f)
        zoom = 10.f;
    if (zoom < 2.f)
        zoom = 2.f;
    if (zoom*1.2f - (updownrotation) < 0.1f) {
        scene_setCameraUp(scene, 0.f, zoom*1.f + updownrotation, 0.1f, rotty,-1.f,0.f);
    } else {
        scene_setCameraUp(scene, 0.f, zoom*1.f + updownrotation, zoom*1.2f - (updownrotation), rotty,-1.f,0.f);
    }
}

int isFlipped=1;
void flipCamera(void) {
	if(isFlipped == 0){
		isFlipped = 1;
		sys->logToConsole("flipped");
		scene_setCameraUp(scene, 0.f, 5.f, 6.f, 5.f,0.f,0.f);
	} else {
		isFlipped = 0;
		scene_setCameraUp(scene, 0.f, 5.f, 6.f, 0.f,-1.f,0.f);
	}
}


void updateStackz() {
    firstLoop();
    if(Game.StackzData.gameover == 0){
        drawBackground();
        handleRotation();
        handleButtonPush();
        
        setupOscillator();
        OscillateActiveNode();

        zoomCameraWithCrank();

        displayScore();
        draw();
    } else {
        drawBackground();
        displayScore();
        draw();
        displayGameOver();

        sys->getButtonState(NULL, &pushed, NULL);
        
        if ( pushed ) {
            Game.StackzData.gameover = 0;
            resetGame();
        }
    }
    
}