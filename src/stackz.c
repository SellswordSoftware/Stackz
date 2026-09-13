#include "stackz.h"

#include "lcdpatterns.h"
#include "mini3d.h"
#include "3dmath.h"
#include "shape.h"
#include "scene.h"
#include "3dhelper.h"
#include "float.h"

#define BORDER
#define TOP_BOX_COLOR_BIAS 0.f
#define PERFECT_RING_DURATION_FRAMES 12
#define PERFECT_RING_GROWTH 1.75f
#define PERFECT_RING_Y -0.255f
#define PERFECT_RING_WHITE_SCALE 0.92f
#define GRAVITY_DEAD_ZONE 0.08f
#define GRAVITY_RESPONSE 0.12f
#define PI 3.14159265358979323846f
#define SCORE_BITMAP_WIDTH 128
#define SCORE_BITMAP_HEIGHT 24
#define SCORE_TOP_DISTANCE 92.f
#define GAME_OVER_BITMAP_PADDING 4
#define GAME_OVER_SCREEN_MARGIN 8.f
#define SCORE_PER_SPEED_LEVEL 15
#define OSCILLATION_SPEED_PER_LEVEL 0.1f
#define OSCILLATION_START_PHASE 2.f

static Scene3D *scene;
static Scene3DNode *rootNode;
static Scene3DNode *rotationNode;
static Scene3DNode *activeNode;
static Scene3DNode *activeNodeSubnode;
static Scene3DNode *stackParentNode;
static Scene3DNode *perfectRingBlackNode;
static Scene3DNode *perfectRingWhiteNode;

static Shape3D *activeBox;

static Matrix3D *stackNodeMatrix;
static RenderStyle globalStyle;

static float leftrightrotation = 45.f;
static float updownrotation = 0.f;

static float targetBoxX = 0.f;
static float targetBoxZ = 0.f;
static float targetBoxXScale = 1.f;
static float targetBoxZScale = 1.f;
static int isFirstLoop = 0;
static int direction = 0;
static int animStarted = 0;
static float bounce = 0.f;
static int perfectRingFramesRemaining = 0;
static float perfectRingX = 0.f;
static float perfectRingZ = 0.f;
static float perfectRingXScale = 1.f;
static float perfectRingZScale = 1.f;
static float gravityUpX = 0.f;
static float gravityUpY = -1.f;
static float gravityUpAngle = -PI / 2.f;
static LCDBitmap *scoreBitmap;
static LCDBitmap *gameOverBitmap;
static int gameOverBitmapWidth;
static int renderedScore;
static int scoreBitmapNeedsUpdate = 1;

static float zoom = 5.f;

float getColorFromIndex(int index) {
    float adjustedIndex = (index + 10) * 0.4f;
    return (sinf((float)adjustedIndex) * 0.5f) - 0.3f;
}

Vector3D v = { 0.f,0.f,0.f };
float addv = 0;
static void addBoxToStack(float x, float z, float scalex, float scalez) {
    struct Node *previousTop = Game.StackzData.currentNode;

    Scene3DNode_setColorBias(previousTop->scene3DNode, previousTop->restingColorBias);
    Game.StackzData.currentNode = Game.StackzData.currentNode->next;
    node_resetTranform(Game.StackzData.currentNode->scene3DNode);
    Scene3DNode_setVisible(Game.StackzData.currentNode->scene3DNode, 1);
    Game.StackzData.currentNode->restingColorBias = getColorFromIndex(Game.StackzData.stackBoxIndex);
    Scene3DNode_setColorBias(Game.StackzData.currentNode->scene3DNode, TOP_BOX_COLOR_BIAS);
    addv += 0.5f;
    matrix_scaleByAndAddTranslation(Game.StackzData.currentNode->scene3DNode, scalex, 1.f, scalez, x, addv, z);
    Game.StackzData.stackNodeMatrix = matrix_addTranslation(0.f, -0.5f, 0.f);
    Scene3DNode_addTransform(stackParentNode, stackNodeMatrix);
    Game.StackzData.stackBoxIndex++;
}

static void updateActiveBlockSize(float x, float z, float scalex, float scalez) {
    node_resetTranform(activeNodeSubnode);
    matrix_scaleByAndAddTranslation(activeNodeSubnode, scalex, 1.f, scalez, x, 0.f, z);
}

static void initPerfectRing(void) {
    Point3D topLeft = { -1.f, 0.f, 1.f };
    Point3D topRight = { 1.f, 0.f, 1.f };
    Point3D bottomRight = { 1.f, 0.f, -1.f };
    Point3D bottomLeft = { -1.f, 0.f, -1.f };
    Shape3D *ring = shape_new();

    Shape3D_addFace(ring, &topLeft, &topRight, &bottomRight, &bottomLeft, 0.f);
    perfectRingBlackNode = Scene3DNode_newChild(rotationNode);
    Scene3DNode_addShape(perfectRingBlackNode, ring);
    Scene3DNode_setRenderStyle(perfectRingBlackNode, kRenderWireframe | kRenderWireframeBack);
    Scene3DNode_setVisible(perfectRingBlackNode, 0);

    perfectRingWhiteNode = Scene3DNode_newChild(rotationNode);
    Scene3DNode_addShape(perfectRingWhiteNode, ring);
    Scene3DNode_setRenderStyle(perfectRingWhiteNode, kRenderWireframe | kRenderWireframeBack | kRenderWireframeWhite);
    Scene3DNode_setVisible(perfectRingWhiteNode, 0);
}

static void emitPerfectRing(void) {
    perfectRingX = targetBoxX;
    perfectRingZ = -targetBoxZ;
    perfectRingXScale = targetBoxXScale;
    perfectRingZScale = targetBoxZScale;
    perfectRingFramesRemaining = PERFECT_RING_DURATION_FRAMES;
    Scene3DNode_setVisible(perfectRingBlackNode, 1);
    Scene3DNode_setVisible(perfectRingWhiteNode, 1);
}

static void updatePerfectRing(void) {
    if (perfectRingFramesRemaining == 0)
        return;

    float progress = 1.f - (float)perfectRingFramesRemaining / PERFECT_RING_DURATION_FRAMES;
    float easedProgress = 1.f - (1.f - progress) * (1.f - progress);
    float size = 1.f + (PERFECT_RING_GROWTH - 1.f) * easedProgress;

    node_resetTranform(perfectRingBlackNode);
    matrix_scaleByAndAddTranslation(perfectRingBlackNode,
        perfectRingXScale * size,
        1.f,
        perfectRingZScale * size,
        perfectRingX,
        PERFECT_RING_Y,
        perfectRingZ);

    node_resetTranform(perfectRingWhiteNode);
    matrix_scaleByAndAddTranslation(perfectRingWhiteNode,
        perfectRingXScale * size * PERFECT_RING_WHITE_SCALE,
        1.f,
        perfectRingZScale * size * PERFECT_RING_WHITE_SCALE,
        perfectRingX,
        PERFECT_RING_Y,
        perfectRingZ);

    if (--perfectRingFramesRemaining == 0) {
        Scene3DNode_setVisible(perfectRingBlackNode, 0);
        Scene3DNode_setVisible(perfectRingWhiteNode, 0);
    }
}

static void initSceneAndCamera(void) {
    scene = Game.StackzData.scene = scene_new();
    scene_setCameraOrigin(scene, 0.f, zoom*1.f, zoom*1.2f);
    scene_setLight(scene, -0.7f, 0.8f, 0.3f);
    sys->setPeripheralsEnabled(kAccelerometer);
}

static void initNodes(void) {
    rootNode = Game.StackzData.rootNode = Scene3D_getRootNode(scene);
    rotationNode = Game.StackzData.activeNode = Scene3DNode_newChild(rootNode);
    activeNode = Game.StackzData.activeNode = Scene3DNode_newChild(rotationNode);
    activeNodeSubnode = Scene3DNode_newChild(activeNode);
    stackParentNode = Scene3DNode_newChild(rotationNode);
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

static void initUiBitmaps(void) {
    scoreBitmap = gfx->newBitmap(SCORE_BITMAP_WIDTH, SCORE_BITMAP_HEIGHT, kColorClear);

    const char gameOverText[] = "Game Over";
    int textWidth = gfx->getTextWidth(Game.font20, gameOverText, sizeof(gameOverText) - 1, kASCIIEncoding, 0);
    int textHeight = gfx->getFontHeight(Game.font20);
    gameOverBitmapWidth = textWidth + GAME_OVER_BITMAP_PADDING * 2;
    int gameOverBitmapHeight = textHeight + GAME_OVER_BITMAP_PADDING * 2;

    gameOverBitmap = gfx->newBitmap(gameOverBitmapWidth, gameOverBitmapHeight, kColorClear);
    if (scoreBitmap == NULL || gameOverBitmap == NULL)
        sys->error("%s:%i Couldn't create UI bitmaps", __FILE__, __LINE__);

    gfx->pushContext(gameOverBitmap);
    gfx->clear(kColorClear);
    gfx->setFont(Game.font20);
    gfx->drawText(gameOverText, sizeof(gameOverText) - 1, kASCIIEncoding,
        GAME_OVER_BITMAP_PADDING, GAME_OVER_BITMAP_PADDING);
    gfx->popContext();
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
        ptr->scene3DNode = Scene3DNode_newChild(stackParentNode);
        Shape3D *shape = shape_new_cuboid(1.f,0.25f,1.f,0.0f);
        Scene3DNode_addShape(ptr->scene3DNode, shape);
        Scene3DNode_setVisible(ptr->scene3DNode, 0);
        ptr->restingColorBias = getColorFromIndex(0);

        ptr = ptr->next;
    } while (ptr != Game.StackzData.lastNode->next);
    Game.StackzData.currentNode = Game.StackzData.firstNode;

    Scene3DNode_setVisible(ptr->scene3DNode, 1);
    Scene3DNode_setColorBias(ptr->scene3DNode, TOP_BOX_COLOR_BIAS);
    Game.StackzData.stackNodeMatrix = matrix_addTranslation(0,-0.5,0);
    stackNodeMatrix = &Game.StackzData.stackNodeMatrix;
    Scene3DNode_addTransform(stackParentNode, stackNodeMatrix);
}

static void resetStack(void) {
    struct Node* ptr = Game.StackzData.firstNode;
    do {
        Scene3DNode_setVisible(ptr->scene3DNode, 0);
        ptr = ptr->next;
    } while (ptr != Game.StackzData.lastNode->next);

    // Restore the same parent offset and base node used for the first round.
    // addBoxToStack() intentionally accumulates this offset while a round runs.
    Game.StackzData.stackNodeMatrix = matrix_addTranslation(0.f, -0.5f, 0.f);
    Scene3DNode_setTransform(stackParentNode, &Game.StackzData.stackNodeMatrix);
    Game.StackzData.currentNode = Game.StackzData.firstNode;
    Game.StackzData.currentNode->restingColorBias = getColorFromIndex(0);
    Scene3DNode_setVisible(Game.StackzData.currentNode->scene3DNode, 1);
    Scene3DNode_setColorBias(Game.StackzData.currentNode->scene3DNode, TOP_BOX_COLOR_BIAS);

}

static void resetGame(void) {
    Game.StackzData.score = 0;
    Game.StackzData.stackBoxIndex = 0;
    sys->resetElapsedTime();
    direction = 0;
    targetBoxX = 0.f;
    targetBoxZ = 0.f;
    targetBoxXScale = 1.f;
    targetBoxZScale = 1.f;
    addv = 0.f;
    Game.StackzData.perfectCount = 0;
    animStarted = 0;
    bounce = 0.f;
    perfectRingFramesRemaining = 0;
    Scene3DNode_setVisible(perfectRingBlackNode, 0);
    Scene3DNode_setVisible(perfectRingWhiteNode, 0);
    node_resetTranform(activeNodeSubnode);
    resetStack();
}

// Scene Init
void initStackzSceneData(void) {
    initDataValues();
    initUiBitmaps();
    initSceneAndCamera();
    initNodes();
    initActiveBox();
    initPerfectRing();
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

static void buttonB(void);

static void buttonA(void) {
    buttonB();
}

#define PERFECT_TOLERANCE 0.15f
#define GROW_AMOUNT 0.15f

// After enough perfects, slightly widen the smaller axis so future
// imperfect placements hurt less. Scales are cumulative multipliers on
// the base full width (1.0 = full size), so multiply up by the ratio.
static void growSmallerAxis(void) {
    if (targetBoxXScale <= targetBoxZScale) {
        float old = targetBoxXScale;
        targetBoxXScale = old * (1.f + GROW_AMOUNT);
        if (targetBoxXScale > 1.f)
            targetBoxXScale = 1.f;
    } else {
        float old = targetBoxZScale;
        targetBoxZScale = old * (1.f + GROW_AMOUNT);
        if (targetBoxZScale > 1.f)
            targetBoxZScale = 1.f;
    }
}

static void buttonB(void) {
    float *targetCenter = direction == 0 ? &targetBoxX : &targetBoxZ;
    float *targetScale = direction == 0 ? &targetBoxXScale : &targetBoxZScale;
    float difference = fabsf(Game.StackzData.activeOscillator - *targetCenter);
    int isPerfect = difference <= PERFECT_TOLERANCE;

    // Touching edges have no overlap, so do not create a zero-size block.
    if (difference >= *targetScale * 2.f) {
        Game.StackzData.gameover = 1;
        return;
    }

    sys->logToConsole("hit");
    Game.StackzData.score++;

    if (isPerfect) {
        sys->logToConsole("perfect");
        Game.StackzData.perfectCount++;
        if (Game.StackzData.perfectCount >= 3) {
            sys->logToConsole("grow");
            Game.StackzData.perfectCount = 0;
            growSmallerAxis();
        }
    } else {
        Game.StackzData.perfectCount = 0;
        float scale = 1.f - difference / (*targetScale * 2.f);
        *targetScale *= scale;
        *targetCenter = (Game.StackzData.activeOscillator + *targetCenter) / 2.f;
    }

    addBoxToStack(targetBoxX, -targetBoxZ, targetBoxXScale, targetBoxZScale);
    updateActiveBlockSize(targetBoxX, -targetBoxZ, targetBoxXScale, targetBoxZScale);
    if (isPerfect)
        emitPerfectRing();

    direction = !direction;

    sys->resetElapsedTime();
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
    int speedLevel = Game.StackzData.score / SCORE_PER_SPEED_LEVEL;
    float oscillationSpeed = 1.f + speedLevel * OSCILLATION_SPEED_PER_LEVEL;
    Game.StackzData.ellapsed = OSCILLATION_START_PHASE + sys->getElapsedTime() * oscillationSpeed;
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

static char score[12];

static int formatScore(int value) {
    char digits[10];
    unsigned int magnitude = value < 0 ? 0u - (unsigned int)value : (unsigned int)value;
    int digitCount = 0;
    int length = 0;

    do {
        digits[digitCount++] = '0' + magnitude % 10;
        magnitude /= 10;
    } while (magnitude > 0);

    if (value < 0)
        score[length++] = '-';

    while (digitCount > 0)
        score[length++] = digits[--digitCount];

    score[length] = '\0';
    return length;
}

static int uiOrientation(void) {
    int orientation = (int)floorf((gravityUpAngle + PI / 2.f) / (PI / 2.f) + 0.5f);
    orientation %= 4;
    if (orientation < 0)
        orientation += 4;
    return orientation;
}

static void displayScore(void) {
    if (scoreBitmapNeedsUpdate || renderedScore != Game.StackzData.score) {
        int scoreLength = formatScore(Game.StackzData.score);
        int scoreWidth = gfx->getTextWidth(Game.font14, score, scoreLength, kASCIIEncoding, 0);
        int scoreHeight = gfx->getFontHeight(Game.font14);

        gfx->pushContext(scoreBitmap);
        gfx->clear(kColorClear);
        gfx->setFont(Game.font14);
        gfx->drawText(score, scoreLength, kASCIIEncoding,
            (SCORE_BITMAP_WIDTH - scoreWidth) / 2,
            (SCORE_BITMAP_HEIGHT - scoreHeight) / 2);
        gfx->popContext();

        renderedScore = Game.StackzData.score;
        scoreBitmapNeedsUpdate = 0;
    }

    static const int topX[] = { 0, 1, 0, -1 };
    static const int topY[] = { -1, 0, 1, 0 };
    int orientation = uiOrientation();
    float rotationDegrees = orientation * 90.f;
    int scoreX = (int)(SCREEN_WIDTH / 2.f + topX[orientation] * SCORE_TOP_DISTANCE);
    int scoreY = (int)(SCREEN_HEIGHT / 2.f + topY[orientation] * SCORE_TOP_DISTANCE);
    gfx->drawRotatedBitmap(scoreBitmap, scoreX, scoreY, rotationDegrees, 0.5f, 0.5f, 1.f, 1.f);
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

float durationofGainm = 2.f;
static void displayGameOver(void) {
    if (animStarted == 0) {
        sys->resetElapsedTime();
        animStarted = 1;
    }

    float progress = sys->getElapsedTime() / durationofGainm;
    float scale = 1.f;
    if (progress < 1.f) {
        bounce = outBounce(progress);
        scale = 0.2f + 0.8f * bounce;
    }

    int orientation = uiOrientation();
    float availableTextLength = orientation % 2 == 0 ? SCREEN_WIDTH : SCREEN_HEIGHT;
    float fitScale = (availableTextLength - GAME_OVER_SCREEN_MARGIN * 2.f) / gameOverBitmapWidth;
    if (fitScale > 1.f)
        fitScale = 1.f;
    scale *= fitScale;

    float rotationDegrees = orientation * 90.f;
    gfx->drawRotatedBitmap(gameOverBitmap, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2,
        rotationDegrees, 0.5f, 0.5f, scale, scale);
}


static float accelx;
static float accely;
static float accelz;
static void handleRotation(void) {
	sys->getAccelerometer(&accelx, &accely, &accelz);

    float magnitude = sqrtf(accelx * accelx + accely * accely);
    float desiredUpAngle = -PI / 2.f;

    // When the device is nearly face-up, gravity has no stable screen direction.
    if (magnitude >= GRAVITY_DEAD_ZONE) {
#if defined(TARGET_PLAYDATE)
        desiredUpAngle = atan2f(accely, accelx);
#else
        desiredUpAngle = atan2f(-accely, -accelx);
#endif
    }

    float angleDifference = desiredUpAngle - gravityUpAngle;
    if (angleDifference > PI)
        angleDifference -= 2.f * PI;
    else if (angleDifference < -PI)
        angleDifference += 2.f * PI;

    gravityUpAngle += angleDifference * GRAVITY_RESPONSE;
    gravityUpX = cosf(gravityUpAngle);
    gravityUpY = sinf(gravityUpAngle);
}

static void zoomCameraWithCrank(void) {
    Game.StackzData.crankChange = sys->getCrankChange() / 50.f;

    zoom -= Game.StackzData.crankChange;
    if (zoom > 10.f)
        zoom = 10.f;
    if (zoom < 2.f)
        zoom = 2.f;
    if (zoom*1.2f - (updownrotation) < 0.1f) {
        scene_setCameraView(scene, 0.f, zoom*1.f + updownrotation, 0.1f, gravityUpX, gravityUpY, 0.f);
    } else {
        scene_setCameraView(scene, 0.f, zoom*1.f + updownrotation, zoom*1.2f - (updownrotation), gravityUpX, gravityUpY, 0.f);
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
        updatePerfectRing();

        setupOscillator();
        OscillateActiveNode();

        zoomCameraWithCrank();

        displayScore();
        draw();
    } else {
        drawBackground();
        handleRotation();
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
