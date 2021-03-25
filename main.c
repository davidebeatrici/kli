#include "Config.h"
#include "Dialog.h"
#include "Layout.h"

#include <string.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

int main()
{
	// Prevent LoadKeyboardLayout() from loading the layout DLL from the application directory.
	// https://github.com/MicrosoftDocs/sdk-api/pull/711
	if (!SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32)) {
		showSysError("main()", "SetDefaultDllDirectories() failed!", GetLastError());
		return -1;
	}

	Config config = {0};
	if (!loadConfig(&config, "kli.ini")) {
		return 1;
	}

	int ret = 0;

	Layouts *layouts = getLayouts();
	if (!layouts) {
		ret = 2;
		goto FINAL;
	}

	for (DWORD i = 0; i < layouts->num; ++i) {
		const Layout *layout = &layouts->list[i];
		if (!layout->product_code || strcmp(layout->product_code, config.product_code) != 0) {
			continue;
		}

		if (!showPrompt("main()", "This layout already appears to be installed.\n\nDo you want to uninstall it?")) {
			goto FINAL;
		}

		if (!uninstallLayout(layout)) {
			ret = 4;
			goto FINAL;
		}

		showInfo("main()", "Layout successfully uninstalled!");
		goto FINAL;
	}

	if (!installLayout(&config, layouts)) {
		ret = 3;
		goto FINAL;
	}

	showInfo("main()", "Layout successfully installed!");
FINAL:
	freeLayouts(layouts);
	freeConfig(&config);
	return ret;
}
