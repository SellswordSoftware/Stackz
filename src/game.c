#include "game.h"
#include "stackz.h"
#include "menu.h"
#include "song1.h"

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
	Game.font = Game.font14;
	gfx->setFont(Game.font14);
}

static void initCircularLinkedListOfStack(void) {
	struct Node* temp;
	for (int i = 0; i < STACKMAX; i++) {
		struct Node* node;
		node = (struct Node*)malloc(sizeof(struct Node));
		node->scene3DNode = NULL;
		node->restingColorBias = 0.f;
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

	PdnaAudioResult audio_result = pdna_audio_init(
		&Game.audio,
		sound,
		&song1
	);
	if (audio_result == PDNA_AUDIO_OK) {
		Game.audio_initialized = true;
		pdna_audio_start_music(&Game.audio);
	} else {
		sys->logToConsole("PDNA audio initialization failed: %d", audio_result);
	}

	initFont();

	initCircularLinkedListOfStack();

	initStackzSceneData();
}

void DeinitGame(void)
{
	if (Game.audio_initialized) {
		pdna_audio_deinit(&Game.audio);
		Game.audio_initialized = false;
	}
}

void PlayPdnaEffect(const PdnaEffectPreset *preset)
{
	if (Game.audio_initialized) {
		pdna_audio_play_effect(&Game.audio, preset);
	}
}

void PlayPdnaSongOnce(const PdnaSongPreset *song)
{
	if (!Game.audio_initialized) {
		return;
	}

	PdnaAudioResult result = pdna_audio_play_song_once(&Game.audio, song);
	if (result != PDNA_AUDIO_OK) {
		sys->logToConsole("PDNA one-shot song initialization failed: %d", result);
	}
}

void PlayPdnaSongLoop(const PdnaSongPreset *song)
{
	if (!Game.audio_initialized) {
		return;
	}

	PdnaAudioResult result = pdna_audio_play_song_loop(&Game.audio, song);
	if (result != PDNA_AUDIO_OK) {
		sys->logToConsole("PDNA looping song initialization failed: %d", result);
	}
}

int Update(void* userdata)
{
	(void)userdata;
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
