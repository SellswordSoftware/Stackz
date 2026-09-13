#include "game.h"
#include "stackz.h"
#include "menu.h"

const struct playdate_graphics* gfx = NULL;
const struct playdate_sys* sys = NULL;
const struct playdate_display* display = NULL;
const struct playdate_file* file = NULL;
const struct playdate_sound* sound = NULL;

GameStruct Game;

static void initFont(void) {
	const char* fontPath14 = "sofachrome-rg-14";
	const char* fontPath20 = "sofachrome-rg-20";
	const char* fontPath40 = "sofachrome-rg-40";
	const char* err;
	Game.font14 = gfx->loadFont(fontPath14, &err);
	if (Game.font14 == NULL)
		sys->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontPath14, err);
	Game.font20 = gfx->loadFont(fontPath20, &err);
	if (Game.font20 == NULL)
		sys->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontPath20, err);
	Game.font40 = gfx->loadFont(fontPath40, &err);
	if (Game.font40 == NULL)
		sys->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fontPath40, err);
	gfx->setFont(Game.font14);
}

static void initCircularLinkedListOfStack(void) {
	struct Node* temp;
	for (int i = 0; i < STACKMAX; i++) {
		struct Node* node;
		node = (struct Node*)malloc(sizeof(struct Node));
		if (i == 0) {
			Game.StackzData.firstNode = node;
			temp = node;
		}
		else {
			temp->next = node;
			temp = node;
		}
		if (i == STACKMAX - 1) {
			sys->logToConsole("setting last box next?");
			Game.StackzData.lastNode = node;
			Game.StackzData.lastNode->next = Game.StackzData.firstNode;
		}
	}
}

void InitGame(PlaydateAPI* pd)
{
	pd->display->setRefreshRate(30);
	Game.gState = State_Menu;
	Game.gPd = pd;
	gfx = Game.gPd->graphics;
	sys = Game.gPd->system;
	display = Game.gPd->display;
	file = Game.gPd->file;
	sound = Game.gPd->sound;

	initFont();

	initCircularLinkedListOfStack();

	initStackzSceneData();
}

int Update(void* userdata)
{
	switch (Game.gState)
	{
	case State_Menu:
		updateMenu();
		break;
	case State_InGame:
		updateStackz();
		break;
	default:
		break;
	}

	//sys->drawFPS(0, 0);
	return 1;
}