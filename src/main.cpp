#include <Geode/Geode.hpp>
#include <Geode/modify/CCEGLView.hpp>
#include <Geode/modify/CCApplication.hpp>
#include <Geode/modify/CCDirector.hpp>

using namespace geode::prelude;

// https://discord.com/channels/911701438269386882/911702535373475870/1258528072886648923

/// @brief Gets the window handle of the game.
HWND getWindowHandle() {
    HWND WINDOW_HANDLE = nullptr;
    if (WINDOW_HANDLE != nullptr)
        return WINDOW_HANDLE;

    auto *director = cocos2d::CCDirector::sharedDirector();
    auto *window = director->getOpenGLView()->getWindow();
    WINDOW_HANDLE = WindowFromDC(*reinterpret_cast<HDC *>(reinterpret_cast<uintptr_t>(window) + 0x278));
    return WINDOW_HANDLE;
}

HWND oldHandle;
WNDPROC oldProc;
Ref<FLAlertLayer> alertPopup;

LRESULT CALLBACK CustomWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CLOSE)
	{
		if (Mod::get()->getSettingValue<bool>("no-popup"))
			return 0;

		if (alertPopup)
			return 0;

		if (Mod::get()->getSettingValue<bool>("os-popup") || Loader::get()->isForwardCompatMode())
		{
			if (MessageBoxW(getWindowHandle(), L"Are you sure you want to quit?", L"Quit Game", MB_YESNO | MB_ICONWARNING) == IDYES)
			{
				utils::game::exit();
			}

			return 0;
		}

		if (Loader::get()->getGameVersion() >= "2.206")
		{
			alertPopup = createQuickPopup("Quit Game", "Are you sure you want to <cr>quit</c>?", "Cancel", "Yes", [](FLAlertLayer* alert, bool right)
			{
				alertPopup = nullptr;

				if (right)
				{
					utils::game::exit();
				}
			});
		}

		return 0;
	}

    return CallWindowProc(oldProc, hwnd, msg, wParam, lParam);
}

void refreshHWNDhook()
{
	auto hwnd = getWindowHandle();

	log::info("Current window HWND: {}", reinterpret_cast<void*>(hwnd));

	oldProc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(hwnd, GWLP_WNDPROC));
	SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(CustomWndProc));
}

$on_mod(Loaded)
{
	refreshHWNDhook();
}

class $modify (CCEGLView)
{
	void toggleFullScreen(bool fullscreen, bool borderless, bool fix)
	{
		CCEGLView::toggleFullScreen(fullscreen, borderless, fix);

		Loader::get()->queueInMainThread([]{
			refreshHWNDhook();
		});
	}
};
