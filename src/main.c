#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"
#include "game.h"

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg)
{
	(void)arg;
	if ( event == kEventInit )
	{
		InitGame(playdate);
		playdate->system->setUpdateCallback(Update, NULL);
	}
	else if (event == kEventTerminate)
	{
		DeinitGame();
	}
	
	return 0;
}
