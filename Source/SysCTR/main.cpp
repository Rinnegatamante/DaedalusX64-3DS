#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

#include <3ds.h>
#include <GL/picaGL.h>

#include "BuildOptions.h"
#include "Config/ConfigOptions.h"
#include "Core/Cheats.h"
#include "Core/CPU.h"
#include "Core/CPU.h"
#include "Core/Memory.h"
#include "Core/PIF.h"
#include "Core/RomSettings.h"
#include "Core/Save.h"
#include "Debug/DBGConsole.h"
#include "Debug/DebugLog.h"
#include "Graphics/GraphicsContext.h"
#include "HLEGraphics/TextureCache.h"
#include "Input/InputManager.h"
#include "Interface/RomDB.h"
#include "System/Paths.h"
#include "System/System.h"
#include "Test/BatchTest.h"

#include "UI/UserInterface.h"
#include "UI/RomSelector.h"

#include "Utility/IO.h"
#include "Utility/Preferences.h"
#include "Utility/Profiler.h"
#include "Utility/Thread.h"
#include "Utility/Translate.h"
#include "Utility/Timer.h"
#include "Utility/ROMFile.h"
#include "Utility/MemoryCTR.h"

static aptHookCookie _hookCookie;

bool isN3DS;

EAudioPluginMode enable_audio = APM_ENABLED_SYNC;

#ifdef DAEDALUS_LOG
void log2file(const char *format, ...) {
	__gnuc_va_list arg;
	int done;
	va_start(arg, format);
	char msg[512];
	done = vsprintf(msg, format, arg);
	va_end(arg);
	sprintf(msg, "%s\n", msg);
	FILE *log = fopen("sdmc:/DaedalusX64.log", "a+");
	if (log != NULL) {
		fwrite(msg, 1, strlen(msg), log);
		fclose(log);
	}
}
#endif

static void _AptEventHook(APT_HookType type, void* param)
{
	switch (type)
	{
		case APTHOOK_ONEXIT: CPU_Halt("Exiting"); break;
		default: break;
	}
}

static void Initialize()
{
	_InitializeSvcHack();
	aptHook(&_hookCookie, _AptEventHook, NULL);
	romfsInit();

	APT_CheckNew3DS(&isN3DS);
	osSetSpeedupEnable(true);

	gfxInit(GSP_RGB565_OES, GSP_RGB565_OES, true);
	gfxSet3D(true);

	pglInit();

	strcpy(gDaedalusExePath, DAEDALUS_CTR_PATH(""));
	strcpy(g_DaedalusConfig.mSaveDir, DAEDALUS_CTR_PATH("SaveGames/"));

	IO::Directory::EnsureExists( DAEDALUS_CTR_PATH("SaveStates/") );
	UI::Initialize();

	System_Init();
}

void HandleEndOfFrame()
{
	aptMainLoop();
}

extern u32 __ctru_heap_size;

int main(int argc, char* argv[])
{
	char fullpath[512];

	Initialize();
	
	std::string rom = UI::DrawRomSelector();

	sprintf(fullpath, "%s%s", DAEDALUS_CTR_PATH("Roms/"), rom.c_str());

	System_Open(fullpath);
	CPU_Run();
	System_Close();
	System_Finalize();

	return 0;
}
