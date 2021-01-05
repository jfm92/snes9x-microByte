#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include "snes9x.h"
#include "memmap.h"
#include "apu.h"
#include "gfx.h"
#include "controls.h"
#include "logger.h"
#include "display.h"
#include "conffile.h"

extern "C" {
	#include "esp_system.h"
	#include "esp_heap_caps.h"
	#include "freertos/FreeRTOS.h"
	#include "freertos/task.h"
	#include "sd_storage.h"
	#include "display_HAL.h"
	#include "freertos/queue.h"
}
QueueHandle_t vidQueue;
#define MAP_BUTTON(id, name) S9xMapButton((id), S9xGetCommandT((name)), false)

uint32_t timer = 0;
uint32_t loops = 0;



void S9xMessage (int type, int number, const char *message)
{
	printf("%s\n", message);
}

void S9xExtraUsage (void)
{
}

void S9xParseArg (char **argv, int &i, int argc)
{

}

void S9xParsePortConfig (ConfigFile &conf, int pass)
{

}

const char * S9xGetDirectory (enum s9x_getdirtype dirtype)
{
	return "";
}

const char * S9xGetFilename (const char *ex, enum s9x_getdirtype dirtype)
{
	static char	s[PATH_MAX + 1];
	char		drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	snprintf(s, PATH_MAX + 1, "%s%s%s%s", S9xGetDirectory(dirtype), SLASH_STR, fname, ex);

	return (s);
}

const char * S9xGetFilenameInc (const char *ex, enum s9x_getdirtype dirtype)
{
	static char	s[PATH_MAX + 1];
	char		drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	unsigned int	i = 0;
	const char		*d;
	struct stat		buf;

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	d = S9xGetDirectory(dirtype);

	do
		snprintf(s, PATH_MAX + 1, "%s%s%s.%03d%s", d, SLASH_STR, fname, i++, ex);
	while (stat(s, &buf) == 0 && i < 1000);

	return (s);
}

const char * S9xBasename (const char *f)
{
	const char	*p;

	if ((p = strrchr(f, '/')) != NULL || (p = strrchr(f, '\\')) != NULL)
		return (p + 1);

	return (f);
}

const char * S9xChooseFilename (bool8 read_only)
{
	return NULL;
}

const char * S9xChooseMovieFilename (bool8 read_only)
{
	return NULL;
}

bool8 S9xOpenSnapshotFile (const char *filename, bool8 read_only, STREAM *file)
{
	return (FALSE);
}

void S9xCloseSnapshotFile (STREAM file)
{

}

bool8 S9xContinueUpdate (int width, int height)
{
	return (TRUE);
}

void S9xToggleSoundChannel (int c)
{

}

void S9xAutoSaveSRAM (void)
{

}

static uint8_t skip = 0;
void S9xSyncSpeed (void)
{
	IPPU.RenderThisFrame = !IPPU.RenderThisFrame;
	//IPPU.RenderThisFrame = skip == 0;
	//if (skip++ == 10) skip = 0;
	//IPPU.RenderThisFrame = true;

	loops++;

	uint32_t time = (esp_timer_get_time() / 1000);

	if ((time - timer) >= 1000) {
		printf("Sync time avg %.2fms\n", (float)(time - timer) / loops);
		timer = time;
		loops = 0;
	}
}

bool8 S9xMapInput (const char *n, s9xcommand_t *cmd)
{
	return false;
}

bool S9xPollButton (uint32 id, bool *pressed)
{
	return false;
}

bool S9xPollAxis (uint32 id, int16 *value)
{
	return false;
}

bool S9xPollPointer (uint32 id, int16 *x, int16 *y)
{
	return false;
}

s9xcommand_t S9xGetPortCommandT (const char *n)
{
	s9xcommand_t	cmd = {};
	return cmd;
}

void S9xSetPalette (void)
{

}

void S9xTextMode (void)
{

}

void S9xGraphicsMode (void)
{

}

char * S9xGetPortCommandName (s9xcommand_t cmd)
{
	return strdup("None");
}

void S9xHandlePortCommand (s9xcommand_t cmd, int16 data1, int16 data2)
{

}

void S9xSetupDefaultKeymap (void)
{

}

void S9xInitInputDevices (void)
{

}

void S9xSamplesAvailable(void *data)
{

}

bool8 S9xOpenSoundDevice (void)
{
	return FALSE;
}

void S9xExit (void)
{
	exit(0);
}

void S9xInitDisplay (int argc, char **argv)
{
	// Setup SNES buffers
	GFX.Pitch = SNES_WIDTH ;
	GFX.Screen = (uint16 *) heap_caps_malloc(GFX.Pitch * SNES_HEIGHT_EXTENDED *2, MALLOC_CAP_8BIT );
	if(GFX.Screen == NULL) printf("Error\r\n");
	printf("GFX screen allocation: %i\r\n",GFX.Pitch * SNES_HEIGHT_EXTENDED *2);
	S9xGraphicsInit();
}

void S9xDeinitDisplay (void)
{

}

bool8 S9xInitUpdate (void)
{

	return (TRUE);
}

bool8 S9xDeinitUpdate (int width, int height)
{
	//spi_lcd_fb_update();
	//spi_lcd_fb_flush();
	//TODO: Add here call that the frame is ready to be send
	xQueueSend(vidQueue, &GFX.Screen, 0);
	return (TRUE);
}

void snes_ppu_task(void *arg)
{
	while(1) {
		if (IPPU.RefreshNow) {
			IPPU.RefreshNow = false;
			S9xUpdateScreen();
		}
	}
}

static void videoTask(void *arg){

    uint16_t *param;

    //Send empty frame
    display_HAL_gb_frame(NULL);
    
    while(1){
        xQueuePeek(vidQueue, &param, portMAX_DELAY);
        display_HAL_gb_frame(param);
        xQueueReceive(vidQueue, &param, portMAX_DELAY);
    }

    vTaskDelete(NULL);
}


void snes_task(void *arg) // IRAM_ATTR
{
	printf("snes task started\r\n");
	display_HAL_init();
	display_HAL_change_endian();
	display_HAL_clear();
	vidQueue = xQueueCreate(7, sizeof(uint16_t *));
	xTaskCreatePinnedToCore(&videoTask, "videoTask", 1024*2, NULL, 1, NULL, 1);
	sd_init();

	memset(&Settings, 0, sizeof(Settings));
	Settings.SupportHiRes = FALSE;
	Settings.Transparency = TRUE;
	Settings.AutoDisplayMessages = TRUE;
	Settings.InitialInfoStringTimeout = 120;
	Settings.HDMATimingHack = 100;
	Settings.BlockInvalidVRAMAccessMaster = TRUE;
	Settings.StopEmulation = TRUE;
	Settings.SkipFrames = 1;
	Settings.TurboSkipFrames = 1;
	Settings.CartAName[0] = 0;
	Settings.CartBName[0] = 0;

	S9xLoadConfigFiles(NULL, 0);

	if (!Memory.Init() || !S9xInitAPU())
	{
		fprintf(stderr, "Snes9x: Memory allocation failure - not enough RAM/virtual memory available.\nExiting...\n");
		Memory.Deinit();
		S9xDeinitAPU();
		exit(1);
	}

	bool8 loaded = FALSE;

	//odroid_spi_bus_acquire();
	loaded = Memory.LoadROM((const char*)arg);
	//odroid_spi_bus_release();

	if (!loaded)
	{
		printf("Error opening the ROM file.\n");
		exit(1);
	} else {
		printf("ROM loaded!\n");
	}

	CPU.Flags = 0;
	Settings.StopEmulation = FALSE;
	Settings.Paused = FALSE;
	Settings.SoundSync = FALSE;

	S9xInitSound(0);
	S9xReportControllers();
	S9xSetController(0, CTL_JOYPAD, 0, 0, 0, 0);
	S9xSetController(1, CTL_NONE, 1, 0, 0, 0);
	S9xInitInputDevices();
	S9xInitDisplay(NULL, 0);

    /*MAP_BUTTON(ODROID_INPUT_A, "Joypad1 A");
    MAP_BUTTON(ODROID_INPUT_B, "Joypad1 B");
    MAP_BUTTON(ODROID_INPUT_START, "Joypad1 X");
    MAP_BUTTON(ODROID_INPUT_SELECT, "Joypad1 Y");
    MAP_BUTTON(ODROID_INPUT_MENU, "Joypad1 Select");
    MAP_BUTTON(ODROID_INPUT_VOLUME, "Joypad1 Start");
    //MAP_BUTTON(BTN_L, "Joypad1 L");
    //MAP_BUTTON(BTN_R, "Joypad1 R");
    MAP_BUTTON(ODROID_INPUT_LEFT, "Joypad1 Left");
    MAP_BUTTON(ODROID_INPUT_RIGHT, "Joypad1 Right");
    MAP_BUTTON(ODROID_INPUT_UP, "Joypad1 Up");
    MAP_BUTTON(ODROID_INPUT_DOWN, "Joypad1 Down");

	MEMORY_BYTES;*/

	while (1)
	{
		S9xMainLoop();
	}
}
