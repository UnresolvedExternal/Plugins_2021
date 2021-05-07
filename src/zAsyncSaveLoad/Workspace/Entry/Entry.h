#include <future>

namespace NAMESPACE
{
	std::atomic<bool> isAsync = false;
	std::atomic<oCBinkPlayer*> binkPlayer = nullptr;
	std::atomic<int> playResult = 0;

	void __cdecl Hook__sysEvent();
	Hook<void(__cdecl*)()> Ivk__sysEvent(ZENFOR(0x004F6AC0, 0x00509530, 0x005026F0, 0x005053E0), &Hook__sysEvent, HookMode::Patch);
	void __cdecl Hook__sysEvent()
	{
		if (isAsync)
			return;

		Ivk__sysEvent();
	}

	int __fastcall Hook_oCBinkPlayer_PlayHandleEvents(oCBinkPlayer*, void*);
	Hook<int(__thiscall*)(oCBinkPlayer*)> Ivk_oCBinkPlayer_PlayHandleEvents(ZENFOR(0x004223D0, 0x00424960, 0x00422970, 0x00422CA0), &Hook_oCBinkPlayer_PlayHandleEvents, HookMode::Patch);
	int __fastcall Hook_oCBinkPlayer_PlayHandleEvents(oCBinkPlayer* _this, void* vtable)
	{
		if (!isAsync)
		{
			Ivk__sysEvent();
			return Ivk_oCBinkPlayer_PlayHandleEvents(_this);
		}

		binkPlayer = _this;
		while (binkPlayer);
		return playResult;
	}

	template <class TTask>
	auto Execute(TTask task)
	{
		auto future = task.get_future();

		if (isAsync || zrenderer->Vid_GetScreenMode() != zTRnd_ScreenMode::zRND_SCRMODE_WINDOWED)
		{
			task();
			return future.get();
		}

		isAsync = true;
		std::thread thread(std::move(task));

		while (future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
		{
			MSG msg;

			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if (binkPlayer != nullptr)
			{
				playResult = Ivk_oCBinkPlayer_PlayHandleEvents(binkPlayer);
				binkPlayer = nullptr;
			}
		}

		thread.join();
		isAsync = false;
		return future.get();
	}

	void __fastcall Hook_oCGame_WriteSavegame(oCGame*, void*, int, int);
	Hook<void(__thiscall*)(oCGame*, int, int)> Ivk_oCGame_WriteSavegame(ZENFOR(0x0063AD80, 0x00661680, 0x006685D0, 0x006C5250), &Hook_oCGame_WriteSavegame, HookMode::Patch);
	void __fastcall Hook_oCGame_WriteSavegame(oCGame* _this, void* vtable, int a0, int a1)
	{
		return Execute(std::packaged_task<void()>([&]() { return Ivk_oCGame_WriteSavegame(_this, a0, a1); }));
	}

	void __fastcall Hook_oCGame_LoadSavegame(oCGame*, void*, int, int);
	Hook<void(__thiscall*)(oCGame*, int, int)> Ivk_oCGame_LoadSavegame(ZENFOR(0x0063C2A0, 0x00662D60, 0x00669BA0, 0x006C67D0), &Hook_oCGame_LoadSavegame, HookMode::Patch);
	void __fastcall Hook_oCGame_LoadSavegame(oCGame* _this, void* vtable, int a0, int a1)
	{
		return Execute(std::packaged_task<void()>([&]() { return Ivk_oCGame_LoadSavegame(_this, a0, a1); }));
	}

	void __fastcall Hook_oCGame_LoadGame(oCGame*, void*, int, zSTRING const&);
	Hook<void(__thiscall*)(oCGame*, int, zSTRING const&)> Ivk_oCGame_LoadGame(ZENFOR(0x0063C070, 0x00662B20, 0x00669970, 0x006C65A0), &Hook_oCGame_LoadGame, HookMode::Patch);
	void __fastcall Hook_oCGame_LoadGame(oCGame* _this, void* vtable, int a0, zSTRING const& a1)
	{
		return Execute(std::packaged_task<void()>([&]() { return Ivk_oCGame_LoadGame(_this, a0, a1); }));
	}
}
