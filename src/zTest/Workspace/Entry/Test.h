#include <iomanip>
#include <fstream>
#include <queue>
#include <array>
#include <thread>
#include <mutex>
#include <future>

namespace NAMESPACE
{
	typedef decltype(std::chrono::high_resolution_clock::now()) Time;
	typedef decltype(std::chrono::high_resolution_clock::now() - std::chrono::high_resolution_clock::now()) Duration;

	class Stopwatch
	{
	private:
		bool enabled;
		Duration elapsed;
		Time time;

	public:
		Stopwatch() :
			enabled{ false },
			elapsed{}
		{

		}

		void Enable(bool enable = true)
		{
			if (enable == enabled)
				return;

			enabled = enable;

			if (enabled)
				time = std::chrono::high_resolution_clock::now();
			else
				elapsed += std::chrono::high_resolution_clock::now() - time;
		}

		bool GetEnabled() const
		{
			return enabled;
		}

		void Reset(const Duration& elapsed = {})
		{
			time = std::chrono::high_resolution_clock::now();
			this->elapsed = elapsed;
		}

		Duration Restart()
		{
			Duration elapsed = Elapsed();
			Reset();
			Enable();
			return elapsed;
		}

		Duration Elapsed()
		{
			bool wasEnabled = enabled;
			Enable(false);

			if (wasEnabled)
				Enable();

			return elapsed;
		}
	};

	struct MethodInfo
	{
		int id;
		std::string name;
		Stopwatch stopwatch;
	};

	std::list<MethodInfo> methods;
	std::unordered_map<int, MethodInfo*> methodsMap;
	Stopwatch sessionStopwatch;
	Stopwatch frameStopwatch;
	Stopwatch methodsStopwatch;
	uint64_t frameCounter;
	std::ofstream out;

	Sub initFile(ZSUB(GameEvent::Execute), []()
		{
			out.open("freezes.txt");
		});

	const std::string frameNrText = "Frame Number";
	const std::string frameTimeText = "Total Frame Time";
	const std::string averageFrameTimeText = "Average Frame Time";
	const std::string unknownText = "Unknown";

	Sub reset(ZSUB(GameEvent::LoadEnd), []()
		{
			frameCounter = 0;
		});

	int GetPercents(const Duration& value, const Duration& total)
	{
		double part = value.count() / static_cast<double>(total.count());
		return static_cast<int>(part * 100.0 + 0.5);
	}

	std::string MakeString(const Duration& value, const Duration& total)
	{
		using namespace std::chrono;

		std::ostringstream out;
		out << duration_cast<milliseconds>(value).count() << "(" << GetPercents(value, total) << "%" << ")";

		return out.str();
	}

	class MethodsResetter
	{
	public:
		~MethodsResetter()
		{
			for (MethodInfo& method : methods)
				method.stopwatch.Reset();
		}
	};

	Sub testPerformance(ZSUB(GameEvent::Loop), []()
		{
			using namespace std::chrono;

			MethodsResetter resetter;
			frameCounter += 1;

			if (frameCounter == 1)
			{
				sessionStopwatch.Restart();
				frameStopwatch.Restart();
				methodsStopwatch.Reset();
				return;
			}

			Duration sessionTime = sessionStopwatch.Elapsed();
			Duration averageFrameTime = sessionTime / (frameCounter - 1);
			Duration frameTime = frameStopwatch.Restart();

			Duration methodsTime = methodsStopwatch.Elapsed();
			methodsStopwatch.Reset();

			if (sessionTime < 1s)
				return;

			if (frameTime < 2.0 * averageFrameTime)
				return;

			ogame->GetTextView()->Printwin(A frameCounter);

			out << std::endl;
			out << std::right << std::setw(frameNrText.length() + 5) << frameNrText;
			out << std::right << std::setw(frameTimeText.length() + 5) << frameTimeText;
			out << std::right << std::setw(averageFrameTimeText.length() + 5) << averageFrameTimeText;

			for (const MethodInfo& method : methods)
				out << std::right << std::setw(method.name.length() + 5) << method.name;

			out << std::right << std::setw(unknownText.length() + 5) << unknownText;
			out << std::endl;

			out << std::right << std::setw(frameNrText.length() + 5) << frameCounter;
			out << std::right << std::setw(frameTimeText.length() + 5) << MakeString(frameTime, averageFrameTime);
			out << std::right << std::setw(averageFrameTimeText.length() + 5) << MakeString(averageFrameTime, frameTime);


			for (MethodInfo& method : methods)
			{
				const Duration duration = method.stopwatch.Elapsed();
				out << std::right << std::setw(method.name.length() + 5) << MakeString(duration, frameTime);
			}

			out << std::right << std::setw(unknownText.length() + 5) << MakeString(frameTime - methodsTime, frameTime);
			out << std::endl;
		});

	class MethodScope
	{
	private:
		static std::vector<MethodInfo*> stack;

	public:
		MethodScope(int id, const char* name)
		{
			auto it = methodsMap.find(id);
			MethodInfo* method = nullptr;

			if (it == methodsMap.end())
			{
				method = &methods.emplace_back();
				method->id = id;
				method->name = name;

				static const std::string prefix = ZENDEF("Gothic_I_Classic::Hook_", "Gothic_I_Addon::Hook_", "Gothic_II_Classic::Hook_", "Gothic_II_Addon::Hook_");

				if (method->name.rfind(prefix, 0) == 0)
					method->name.erase(0, prefix.length());

				methodsMap[id] = method;
			}
			else
				method = it->second;


			if (stack.empty())
				methodsStopwatch.Enable();
			else
				stack.back()->stopwatch.Enable(false);

			method->stopwatch.Enable(true);
			stack += method;
		}

		~MethodScope()
		{
			stack.back()->stopwatch.Enable(false);
			stack.pop_back();

			if (stack.empty())
			{
				methodsStopwatch.Enable(false);
				return;
			}

			stack.back()->stopwatch.Enable(true);
		}
	};

	std::vector<MethodInfo*> MethodScope::stack;

	constexpr int npcsPerFrame = 3;
	bool inCheckInsertNpcs;

	struct InsertNpcEntry
	{
		ZOwner<oCNpc> npc;
		zVEC3 spawnPos;

		InsertNpcEntry(oCNpc* npc, const zVEC3& spawnPos) :
			npc{ npc },
			spawnPos{ spawnPos }
		{
			npc->AddRef();
		}
	};

	std::queue<InsertNpcEntry> insertQueue;

	void __fastcall Hook_oCSpawnManager_CheckInsertNpcs_2(oCSpawnManager*, void*);
	Hook<void(__thiscall*)(oCSpawnManager*)> Ivk_oCSpawnManager_CheckInsertNpcs_2(ZENFOR(0x006CF9A0, 0x007063D0, 0x00718650, 0x00777BE0), &Hook_oCSpawnManager_CheckInsertNpcs_2, HookMode::Patch);
	void __fastcall Hook_oCSpawnManager_CheckInsertNpcs_2(oCSpawnManager* _this, void* vtable)
	{
		auto scope = AssignTemp(inCheckInsertNpcs, true);
		Ivk_oCSpawnManager_CheckInsertNpcs_2(_this);
	}

	void __fastcall Hook_oCNpc_Enable(oCNpc*, void*, zVEC3&);
	Hook<void(__thiscall*)(oCNpc*, zVEC3&)> Ivk_oCNpc_Enable(ZENFOR(0x006A2000, 0x006D4820, 0x006E72C0, 0x00745D40), &Hook_oCNpc_Enable, HookMode::Patch);
	void __fastcall Hook_oCNpc_Enable(oCNpc* _this, void* vtable, zVEC3& a0)
	{
		if (inCheckInsertNpcs)
		{
			insertQueue.emplace(_this, a0);
			return;
		}

		Ivk_oCNpc_Enable(_this, a0);
	}

	Sub clearInsertQueue(ZSUB(GameEvent::LoadBegin, GameEvent::Exit), []()
		{
			while (!insertQueue.empty())
				insertQueue.pop();
		});

	void InsertNpcs(int limit = -1)
	{
		for (int i = 0; i != limit && !insertQueue.empty(); i++)
		{
			oCNpc* const npc = insertQueue.front().npc.get();
			const zVEC3& spawnPos = insertQueue.front().spawnPos;

			if (!ogame->GetSpawnManager()->InsertNpc(npc, spawnPos))
				ogame->GetSpawnManager()->SpawnNpc(npc, spawnPos, 0.0f);

			insertQueue.pop();
		}
	}

	Sub purgeInsertQueue(ZSUB(GameEvent::SaveBegin), []()
		{
			InsertNpcs();
		});

#define COLUMN MethodScope methodScope{ __LINE__, __FUNCTION__ }

	void __fastcall Hook_zCWorld_AdvanceClock(zCWorld*, void*, float);
	Hook<void(__thiscall*)(zCWorld*, float)> Ivk_zCWorld_AdvanceClock(ZENFOR(0x005F7A80, 0x00618C30, 0x0061E950, 0x006260E0), &Hook_zCWorld_AdvanceClock, HookMode::Patch);
	void __fastcall Hook_zCWorld_AdvanceClock(zCWorld* _this, void* vtable, float a0)
	{
		if (_this == COA(ogame, GetGameWorld()))
			InsertNpcs(npcsPerFrame);

		Ivk_zCWorld_AdvanceClock(_this, a0);
	}

	void __fastcall Hook_zCVob_DoFrameActivity(zCVob*, void*);
	Hook<void(__thiscall*)(zCVob*)> Ivk_zCVob_DoFrameActivity(ZENFOR(0x005D75A0, 0x005F6BE0, 0x005FBE30, 0x00602C60), &Hook_zCVob_DoFrameActivity, HookMode::Patch);
	void __fastcall Hook_zCVob_DoFrameActivity(zCVob* _this, void* vtable)
	{
		COLUMN;
		Ivk_zCVob_DoFrameActivity(_this);
	}

	long32 Hook_vdf_fread(long32, HBuffer, long32);
	Hook<long32(*)(long32, HBuffer, long32)> Ivk_vdf_fread(&vdf_fread, &Hook_vdf_fread, ZTEST(HookMode::Patch));
	long32 Hook_vdf_fread(long32 handle, HBuffer buffer, long32 size)
	{
		COLUMN;
		return Ivk_vdf_fread(handle, buffer, size);
	}

	void __fastcall Hook_zCBspTree_Render(zCBspTree*, void*);
	Hook<void(__thiscall*)(zCBspTree*)> Ivk_zCBspTree_Render(ZENFOR(0x0051D840, 0x005335A0, 0x0052D130, 0x00530080), &Hook_zCBspTree_Render, HookMode::Patch);
	void __fastcall Hook_zCBspTree_Render(zCBspTree* _this, void* vtable)
	{
		COLUMN;
		Ivk_zCBspTree_Render(_this);
	}

	void __fastcall Hook_zCBspTree_RenderVobList(zCBspTree*, void*);
	Hook<void(__thiscall*)(zCBspTree*)> Ivk_zCBspTree_RenderVobList(ZENFOR(0x0051A7E0, 0x005304A0, 0x0052A150, 0x0052D0A0), &Hook_zCBspTree_RenderVobList, HookMode::Patch);
	void __fastcall Hook_zCBspTree_RenderVobList(zCBspTree* _this, void* vtable)
	{
		COLUMN;
		Ivk_zCBspTree_RenderVobList(_this);
	}

	// WARNING: supported versions are G2, G2A
	void __fastcall Hook_zCRenderManager_BuildShader(zCRenderManager*, zCMaterial*, zCTexture*, unsigned long, int, int, zCShader*&);
	Hook<void(__fastcall*)(zCRenderManager*, zCMaterial*, zCTexture*, unsigned long, int, int, zCShader*&)> Ivk_zCRenderManager_BuildShader(ZENFOR(0x00000000, 0x00000000, 0x005D1110, 0x005D7ED0), &Hook_zCRenderManager_BuildShader, HookMode::Patch);
	void __fastcall Hook_zCRenderManager_BuildShader(zCRenderManager* _this, zCMaterial* a0, zCTexture* a1, unsigned long a2, int a3, int a4, zCShader*& a5)
	{
		COLUMN;
		Ivk_zCRenderManager_BuildShader(_this, a0, a1, a2, a3, a4, a5);
	}

	void __fastcall Hook_zCRenderManager_PackVB(zCRenderManager*, zCArray<zCPolygon*> const&, zCShader*);
	Hook<void(__fastcall*)(zCRenderManager*, zCArray<zCPolygon*> const&, zCShader*)> Ivk_zCRenderManager_PackVB(ZENFOR(0x005B2D60, 0x005CFFD0, 0x005D2940, 0x005D9700), &Hook_zCRenderManager_PackVB, HookMode::Patch);
	void __fastcall Hook_zCRenderManager_PackVB(zCRenderManager* _this, zCArray<zCPolygon*> const& a0, zCShader* a1)
	{
		COLUMN;
		Ivk_zCRenderManager_PackVB(_this, a0, a1);
	}

	void __fastcall Hook_zCRenderManager_DrawVertexBuffer(zCRenderManager*, zCVertexBuffer*, int, int, unsigned short*, unsigned long, zCShader*);
	Hook<void(__fastcall*)(zCRenderManager*, zCVertexBuffer*, int, int, unsigned short*, unsigned long, zCShader*)> Ivk_zCRenderManager_DrawVertexBuffer(ZENFOR(0x005B2AA0, 0x005CFD10, 0x005D25D0, 0x005D9390), &Hook_zCRenderManager_DrawVertexBuffer, HookMode::Patch);
	void __fastcall Hook_zCRenderManager_DrawVertexBuffer(zCRenderManager* _this, zCVertexBuffer* a0, int a1, int a2, unsigned short* a3, unsigned long a4, zCShader* a5)
	{
		COLUMN;
		Ivk_zCRenderManager_DrawVertexBuffer(_this, a0, a1, a2, a3, a4, a5);
	}

	int __fastcall Hook_zCTexture_CacheInNamed(zCTexture*, void*, zSTRING const*);
	Hook<int(__thiscall*)(zCTexture*, zSTRING const*)> Ivk_zCTexture_CacheInNamed(ZENFOR(0x005CBA70, 0x005EA4D0, 0x005EFC30, 0x005F69E0), &Hook_zCTexture_CacheInNamed, HookMode::Patch);
	int __fastcall Hook_zCTexture_CacheInNamed(zCTexture* _this, void* vtable, zSTRING const* a0)
	{
		COLUMN;
		int result = Ivk_zCTexture_CacheInNamed(_this, a0);
		return result;
	}

	int __fastcall Hook_zCTextureFileFormatInternal_LoadTexture(zCTextureFileFormatInternal*, void*, zSTRING const&, zCTextureExchange*);
	Hook<int(__thiscall*)(zCTextureFileFormatInternal*, zSTRING const&, zCTextureExchange*)> Ivk_zCTextureFileFormatInternal_LoadTexture(ZENFOR(0x005C7D90, 0x005E6560, 0x005EBB10, 0x005F28C0), &Hook_zCTextureFileFormatInternal_LoadTexture, HookMode::Patch);
	int __fastcall Hook_zCTextureFileFormatInternal_LoadTexture(zCTextureFileFormatInternal* _this, void* vtable, zSTRING const& a0, zCTextureExchange* a1)
	{
		COLUMN;
		int result = Ivk_zCTextureFileFormatInternal_LoadTexture(_this, a0, a1);
		return result;
	}

	int __fastcall Hook_zCRnd_D3D_DrawVertexBuffer(zCRnd_D3D*, void*, zCVertexBuffer*, int, int, unsigned short*, unsigned long);
	Hook<int(__thiscall*)(zCRnd_D3D*, zCVertexBuffer*, int, int, unsigned short*, unsigned long)> Ivk_zCRnd_D3D_DrawVertexBuffer(ZENFOR(0x00719710, 0x007558C0, 0x00765090, 0x006519F0), &Hook_zCRnd_D3D_DrawVertexBuffer, HookMode::Patch);
	int __fastcall Hook_zCRnd_D3D_DrawVertexBuffer(zCRnd_D3D* _this, void* vtable, zCVertexBuffer* a0, int a1, int a2, unsigned short* a3, unsigned long a4)
	{
		COLUMN;
		int result = Ivk_zCRnd_D3D_DrawVertexBuffer(_this, a0, a1, a2, a3, a4);
		return result;
	}

	void __fastcall Hook_oCGame_RenderBlit(oCGame*, void*);
	Hook<void(__thiscall*)(oCGame*)> Ivk_oCGame_RenderBlit(ZENFOR(0x0063FE90, 0x00666EC0, 0x0066DB90, 0x006CA910), &Hook_oCGame_RenderBlit, HookMode::Patch);
	void __fastcall Hook_oCGame_RenderBlit(oCGame* _this, void* vtable)
	{
		COLUMN;
		Ivk_oCGame_RenderBlit(_this);
	}

	void __fastcall Hook_zCWorld_Render(zCWorld*, void*, zCCamera&);
	Hook<void(__thiscall*)(zCWorld*, zCCamera&)> Ivk_zCWorld_Render(ZENFOR(0x005F3EC0, 0x00614D70, 0x00619FB0, 0x00621700), &Hook_zCWorld_Render, HookMode::Patch);
	void __fastcall Hook_zCWorld_Render(zCWorld* _this, void* vtable, zCCamera& a0)
	{
		COLUMN;
		Ivk_zCWorld_Render(_this, a0);
	}

	void __fastcall Hook_zCRnd_D3D_FlushPolys(zCRnd_D3D*, void*);
	Hook<void(__thiscall*)(zCRnd_D3D*)> Ivk_zCRnd_D3D_FlushPolys(ZENFOR(0x00716F50, 0x00752EC0, 0x00762770, 0x0064DD10), &Hook_zCRnd_D3D_FlushPolys, HookMode::Patch);
	void __fastcall Hook_zCRnd_D3D_FlushPolys(zCRnd_D3D* _this, void* vtable)
	{
		COLUMN;
		Ivk_zCRnd_D3D_FlushPolys(_this);
	}

	void __fastcall Hook_zCParticleFX_zCStaticPfxList_ProcessList(zCParticleFX::zCStaticPfxList*, void*);
	Hook<void(__thiscall*)(zCParticleFX::zCStaticPfxList*)> Ivk_zCParticleFX_zCStaticPfxList_ProcessList(ZENFOR(0x0058D650, 0x005A8C00, 0x005A7D70, 0x005AD3F0), &Hook_zCParticleFX_zCStaticPfxList_ProcessList, HookMode::Patch);
	void __fastcall Hook_zCParticleFX_zCStaticPfxList_ProcessList(zCParticleFX::zCStaticPfxList* _this, void* vtable)
	{
		COLUMN;
		Ivk_zCParticleFX_zCStaticPfxList_ProcessList(_this);
	}

	void __fastcall Hook_oCZoneMusic_ProcessZoneList(oCZoneMusic*, void*, zCArraySort<zCZone*> const&, zCArraySort<zCZone*> const&, zCWorld*);
	Hook<void(__thiscall*)(oCZoneMusic*, zCArraySort<zCZone*> const&, zCArraySort<zCZone*> const&, zCWorld*)> Ivk_oCZoneMusic_ProcessZoneList(ZENFOR(0x0070AE80, 0x00745D40, 0x00754FC0, 0x00640560), &Hook_oCZoneMusic_ProcessZoneList, HookMode::Patch);
	void __fastcall Hook_oCZoneMusic_ProcessZoneList(oCZoneMusic* _this, void* vtable, zCArraySort<zCZone*> const& a0, zCArraySort<zCZone*> const& a1, zCWorld* a2)
	{
		COLUMN;
		Ivk_oCZoneMusic_ProcessZoneList(_this, a0, a1, a2);
	}

	class AsyncManager
	{
	private:
		std::thread thread;
		std::queue<std::function<void()>> work;
		std::mutex mutex;
		std::condition_variable cond;
		bool exit;

		void DoWork()
		{
			while (true)
			{
				std::unique_lock lock{ mutex };

				if (work.empty())
					cond.notify_one();

				cond.wait(lock, [this]() { return exit || !work.empty(); });

				if (exit && work.empty())
					return;

				std::function item = work.front();
				work.pop();
				lock.unlock();
				item();
			}
		}

	public:
		AsyncManager() :
			exit{ false }
		{
			thread = std::thread{ &AsyncManager::DoWork, this };
		}

		void Execute(const std::function<void()>& item)
		{
			std::lock_guard lock{ mutex };
			work.push(item);
			cond.notify_all();
		}

		bool IsAsync()
		{
			return thread.get_id() == std::this_thread::get_id();
		}

		template <class TFunc, class... TArgs >
		auto ExecuteSync(TFunc func, TArgs&&... args)
		{
			if (IsAsync())
				return func(std::forward(args)...);

			if constexpr (std::is_same_v<void, decltype(func(std::forward(args)...))>)
			{
				std::function<void()> item = [&]()
				{
					func(std::forward(args)...);
				};

				Execute(item);
				Wait();
			}
			else
			{
				decltype(func(std::forward(args)...)) ret{};

				std::function<void()> item = [&]()
				{
					ret = func(std::forward(args)...);
				};

				Execute(item);
				Wait();
				return ret;
			}
		}

		void Wait()
		{
			std::unique_lock lock{ mutex };
			cond.wait(lock, [this]() { return work.empty(); });
		}

		~AsyncManager()
		{
			std::lock_guard lock{ mutex };
			exit = true;
			cond.notify_all();
			thread.join();
		}
	};

	AsyncManager* asyncMan;

	zCMusicSys_DirectMusic* __fastcall Hook_zCMusicSys_DirectMusic_zCMusicSys_DirectMusic(zCMusicSys_DirectMusic*, void*);
	Hook<zCMusicSys_DirectMusic* (__thiscall*)(zCMusicSys_DirectMusic*)> Ivk_zCMusicSys_DirectMusic_zCMusicSys_DirectMusic(ZENFOR(0x004DA270, 0x004EB490, 0x004E4AD0, 0x004E7450), &Hook_zCMusicSys_DirectMusic_zCMusicSys_DirectMusic, HookMode::Patch);
	zCMusicSys_DirectMusic* __fastcall Hook_zCMusicSys_DirectMusic_zCMusicSys_DirectMusic(zCMusicSys_DirectMusic* _this, void* vtable)
	{
		ASSERT(!asyncMan);
		asyncMan = new AsyncManager();
		return asyncMan->ExecuteSync([=]() { return Ivk_zCMusicSys_DirectMusic_zCMusicSys_DirectMusic(_this); });
	}

	void __fastcall Hook_zCMusicSys_DirectMusic_PlayThemeByScript(zCMusicSys_DirectMusic*, void*, zSTRING const&, int, int*);
	Hook<void(__thiscall*)(zCMusicSys_DirectMusic*, zSTRING const&, int, int*)> Ivk_zCMusicSys_DirectMusic_PlayThemeByScript(ZENFOR(0x004DB850, 0x004ECB40, 0x004E5FA0, 0x004E8AB0), &Hook_zCMusicSys_DirectMusic_PlayThemeByScript, HookMode::Patch);
	void __fastcall Hook_zCMusicSys_DirectMusic_PlayThemeByScript(zCMusicSys_DirectMusic* _this, void* vtable, zSTRING const& a0, int a1, int* a2)
	{
		if (a2)
			*a2 = true;

		return asyncMan->Execute([=]() { return Ivk_zCMusicSys_DirectMusic_PlayThemeByScript(_this, a0, a1, nullptr); });
	}

	void __fastcall Hook_zCMusicSys_DirectMusic_DoMusicUpdate(zCMusicSys_DirectMusic*, void*);
	Hook<void(__thiscall*)(zCMusicSys_DirectMusic*)> Ivk_zCMusicSys_DirectMusic_DoMusicUpdate(ZENFOR(0x004DCBE0, 0x004EE0E0, 0x004E7400, 0x004E9F10), &Hook_zCMusicSys_DirectMusic_DoMusicUpdate, HookMode::Patch);
	void __fastcall Hook_zCMusicSys_DirectMusic_DoMusicUpdate(zCMusicSys_DirectMusic* _this, void* vtable)
	{
		return asyncMan->ExecuteSync([=]() { return Ivk_zCMusicSys_DirectMusic_DoMusicUpdate(_this); });
	}

	zCMusicTheme* __fastcall Hook_zCMusicSys_DirectMusic_LoadThemeByScript(zCMusicSys_DirectMusic*, void*, zSTRING const&);
	Hook<zCMusicTheme* (__thiscall*)(zCMusicSys_DirectMusic*, zSTRING const&)> Ivk_zCMusicSys_DirectMusic_LoadThemeByScript(ZENFOR(0x004DBA20, 0x004ECD40, 0x004E6170, 0x004E8C80), &Hook_zCMusicSys_DirectMusic_LoadThemeByScript, HookMode::Patch);
	zCMusicTheme* __fastcall Hook_zCMusicSys_DirectMusic_LoadThemeByScript(zCMusicSys_DirectMusic* _this, void* vtable, zSTRING const& a0)
	{
		return asyncMan->ExecuteSync([=]() { return Ivk_zCMusicSys_DirectMusic_LoadThemeByScript(_this, a0); });
	}

	zCMusicTheme* __fastcall Hook_zCMusicSys_DirectMusic_GetActiveTheme(zCMusicSys_DirectMusic*, void*);
	Hook<zCMusicTheme* (__thiscall*)(zCMusicSys_DirectMusic*)> Ivk_zCMusicSys_DirectMusic_GetActiveTheme(ZENFOR(0x004DAED0, 0x004EC060, 0x004E5620, 0x004E8130), &Hook_zCMusicSys_DirectMusic_GetActiveTheme, HookMode::Patch);
	zCMusicTheme* __fastcall Hook_zCMusicSys_DirectMusic_GetActiveTheme(zCMusicSys_DirectMusic* _this, void* vtable)
	{
		return asyncMan->ExecuteSync([=]() { return Ivk_zCMusicSys_DirectMusic_GetActiveTheme(_this); });
	}

	void __fastcall Hook_zCMusicSys_DirectMusic_SetVolume(zCMusicSys_DirectMusic*, void*, float);
	Hook<void(__thiscall*)(zCMusicSys_DirectMusic*, float)> Ivk_zCMusicSys_DirectMusic_SetVolume(ZENFOR(0x004DCC80, 0x004EE180, 0x004E74C0, 0x004E9FD0), &Hook_zCMusicSys_DirectMusic_SetVolume, HookMode::Patch);
	void __fastcall Hook_zCMusicSys_DirectMusic_SetVolume(zCMusicSys_DirectMusic* _this, void* vtable, float a0)
	{
		return asyncMan->ExecuteSync([=]() { return Ivk_zCMusicSys_DirectMusic_SetVolume(_this, a0); });
	}
	
	void __fastcall Hook_zCMusicSys_DirectMusic_Destructor(zCMusicSys_DirectMusic*, void*);
	Hook<void(__thiscall*)(zCMusicSys_DirectMusic*)> Ivk_zCMusicSys_DirectMusic_Destructor(ZENFOR(0x004DB790, 0x004ECA80, 0x004E5EE0, 0x004E89F0), &Hook_zCMusicSys_DirectMusic_Destructor, HookMode::Patch);
	void __fastcall Hook_zCMusicSys_DirectMusic_Destructor(zCMusicSys_DirectMusic* _this, void* vtable)
	{
		asyncMan->ExecuteSync([=]() { return Ivk_zCMusicSys_DirectMusic_Destructor(_this); });
		delete asyncMan;
		asyncMan = nullptr;
	}

	void __fastcall Hook_zCMusicSys_DirectMusic_Stop(zCMusicSys_DirectMusic*, void*);
	Hook<void(__thiscall*)(zCMusicSys_DirectMusic*)> Ivk_zCMusicSys_DirectMusic_Stop(ZENFOR(0x004DCBF0, 0x004EE0F0, 0x004E7410, 0x004E9F20), &Hook_zCMusicSys_DirectMusic_Stop, HookMode::Patch);
	void __fastcall Hook_zCMusicSys_DirectMusic_Stop(zCMusicSys_DirectMusic* _this, void* vtable)
	{
		return asyncMan->ExecuteSync([=]() { return Ivk_zCMusicSys_DirectMusic_Stop(_this); });
	}

	int __fastcall Hook_zCMusicSys_DirectMusic_IsAvailable(zCMusicSys_DirectMusic*, void*, zSTRING const&);
	Hook<int(__thiscall*)(zCMusicSys_DirectMusic*, zSTRING const&)> Ivk_zCMusicSys_DirectMusic_IsAvailable(ZENFOR(0x004DCCC0, 0x004EE1C0, 0x004E7500, 0x004EA010), &Hook_zCMusicSys_DirectMusic_IsAvailable, HookMode::Patch);
	int __fastcall Hook_zCMusicSys_DirectMusic_IsAvailable(zCMusicSys_DirectMusic* _this, void* vtable, zSTRING const& a0)
	{
		return asyncMan->ExecuteSync([=]() { return Ivk_zCMusicSys_DirectMusic_IsAvailable(_this, a0); });
	}

	void __fastcall Hook_zCMusicSys_DirectMusic_PlayTheme(zCMusicSys_DirectMusic*, void*, zCMusicTheme*, float const&, zTMus_TransType const&, zTMus_TransSubType const&);
	Hook<void(__thiscall*)(zCMusicSys_DirectMusic*, zCMusicTheme*, float const&, zTMus_TransType const&, zTMus_TransSubType const&)> Ivk_zCMusicSys_DirectMusic_PlayTheme(ZENFOR(0x004DC4E0, 0x004ED980, 0x004E6D10, 0x004E9820), &Hook_zCMusicSys_DirectMusic_PlayTheme, HookMode::Patch);
	void __fastcall Hook_zCMusicSys_DirectMusic_PlayTheme(zCMusicSys_DirectMusic* _this, void* vtable, zCMusicTheme* a0, float const& a1, zTMus_TransType const& a2, zTMus_TransSubType const& a3)
	{
		return asyncMan->ExecuteSync([=]() { return Ivk_zCMusicSys_DirectMusic_PlayTheme(_this, a0, a1, a2, a3); });
	}

	int __fastcall Hook_zCMusicSys_DirectMusic_PlayJingleByScript(zCMusicSys_DirectMusic*, void*, zSTRING const&);
	Hook<int(__thiscall*)(zCMusicSys_DirectMusic*, zSTRING const&)> Ivk_zCMusicSys_DirectMusic_PlayJingleByScript(ZENFOR(0x004DC750, 0x004EDBF0, 0x004E6F80, 0x004E9A90), &Hook_zCMusicSys_DirectMusic_PlayJingleByScript, HookMode::Patch);
	int __fastcall Hook_zCMusicSys_DirectMusic_PlayJingleByScript(zCMusicSys_DirectMusic* _this, void* vtable, zSTRING const& a0)
	{
		asyncMan->Execute([=]() { return Ivk_zCMusicSys_DirectMusic_PlayJingleByScript(_this, a0); });
		return true;
	}

	zCMusicTheme* __fastcall Hook_zCMusicSys_DirectMusic_LoadTheme(zCMusicSys_DirectMusic*, void*, zSTRING const&);
	Hook<zCMusicTheme* (__thiscall*)(zCMusicSys_DirectMusic*, zSTRING const&)> Ivk_zCMusicSys_DirectMusic_LoadTheme(ZENFOR(0x004DBA10, 0x004ECD30, 0x004E6160, 0x004E8C70), &Hook_zCMusicSys_DirectMusic_LoadTheme, HookMode::Patch);
	zCMusicTheme* __fastcall Hook_zCMusicSys_DirectMusic_LoadTheme(zCMusicSys_DirectMusic* _this, void* vtable, zSTRING const& a0)
	{
		return asyncMan->ExecuteSync([=]() { return Ivk_zCMusicSys_DirectMusic_LoadTheme(_this, a0); });
	}

	float __fastcall Hook_zCMusicSys_DirectMusic_GetVolume(zCMusicSys_DirectMusic*, void*);
	Hook<float(__thiscall*)(zCMusicSys_DirectMusic*)> Ivk_zCMusicSys_DirectMusic_GetVolume(ZENFOR(0x004DAF00, 0x004EC090, 0x004E5650, 0x004E8160), &Hook_zCMusicSys_DirectMusic_GetVolume, HookMode::Patch);
	float __fastcall Hook_zCMusicSys_DirectMusic_GetVolume(zCMusicSys_DirectMusic* _this, void* vtable)
	{
		return asyncMan->ExecuteSync([=]() { return Ivk_zCMusicSys_DirectMusic_GetVolume(_this); });
	}

	int __fastcall Hook_zCMusicSys_DirectMusic_IsJingleActive(zCMusicSys_DirectMusic*, void*, int const&);
	Hook<int(__thiscall*)(zCMusicSys_DirectMusic*, int const&)> Ivk_zCMusicSys_DirectMusic_IsJingleActive(ZENFOR(0x004DAEF0, 0x004EC080, 0x004E5640, 0x004E8150), &Hook_zCMusicSys_DirectMusic_IsJingleActive, HookMode::Patch);
	int __fastcall Hook_zCMusicSys_DirectMusic_IsJingleActive(zCMusicSys_DirectMusic* _this, void* vtable, int const& a0)
	{
		return asyncMan->ExecuteSync([=]() { return Ivk_zCMusicSys_DirectMusic_IsJingleActive(_this, a0); });
	}

	zCMusicJingle* __fastcall Hook_zCMusicSys_DirectMusic_LoadJingle(zCMusicSys_DirectMusic*, void*, zSTRING const&);
	Hook<zCMusicJingle* (__thiscall*)(zCMusicSys_DirectMusic*, zSTRING const&)> Ivk_zCMusicSys_DirectMusic_LoadJingle(ZENFOR(0x004DCA50, 0x004EDF40, 0x004E7270, 0x004E9D80), &Hook_zCMusicSys_DirectMusic_LoadJingle, HookMode::Patch);
	zCMusicJingle* __fastcall Hook_zCMusicSys_DirectMusic_LoadJingle(zCMusicSys_DirectMusic* _this, void* vtable, zSTRING const& a0)
	{
		return asyncMan->ExecuteSync([=]() { return Ivk_zCMusicSys_DirectMusic_LoadJingle(_this, a0); });
	}

	zCMusicJingle* __fastcall Hook_zCMusicSys_DirectMusic_LoadJingleByScript(zCMusicSys_DirectMusic*, void*, zSTRING const&);
	Hook<zCMusicJingle* (__thiscall*)(zCMusicSys_DirectMusic*, zSTRING const&)> Ivk_zCMusicSys_DirectMusic_LoadJingleByScript(ZENFOR(0x004DC890, 0x004EDD50, 0x004E70C0, 0x004E9BD0), &Hook_zCMusicSys_DirectMusic_LoadJingleByScript, HookMode::Patch);
	zCMusicJingle* __fastcall Hook_zCMusicSys_DirectMusic_LoadJingleByScript(zCMusicSys_DirectMusic* _this, void* vtable, zSTRING const& a0)
	{
		return asyncMan->ExecuteSync([=]() { return Ivk_zCMusicSys_DirectMusic_LoadJingleByScript(_this, a0); });
	}

	void __fastcall Hook_zCMusicSys_DirectMusic_Mute(zCMusicSys_DirectMusic*, void*);
	Hook<void(__thiscall*)(zCMusicSys_DirectMusic*)> Ivk_zCMusicSys_DirectMusic_Mute(ZENFOR(0x004DCC20, 0x004EE120, 0x004E7430, 0x004E9F40), &Hook_zCMusicSys_DirectMusic_Mute, HookMode::Patch);
	void __fastcall Hook_zCMusicSys_DirectMusic_Mute(zCMusicSys_DirectMusic* _this, void* vtable)
	{
		return asyncMan->ExecuteSync([=]() { return Ivk_zCMusicSys_DirectMusic_Mute(_this); });
	}

	int __fastcall Hook_zCMusicSys_DirectMusic_PlayJingle(zCMusicSys_DirectMusic*, void*, zCMusicJingle const*, float const&, zTMus_TransSubType const&);
	Hook<int(__thiscall*)(zCMusicSys_DirectMusic*, zCMusicJingle const*, float const&, zTMus_TransSubType const&)> Ivk_zCMusicSys_DirectMusic_PlayJingle(ZENFOR(0x004DCA60, 0x004EDF50, 0x004E7280, 0x004E9D90), &Hook_zCMusicSys_DirectMusic_PlayJingle, HookMode::Patch);
	int __fastcall Hook_zCMusicSys_DirectMusic_PlayJingle(zCMusicSys_DirectMusic* _this, void* vtable, zCMusicJingle const* a0, float const& a1, zTMus_TransSubType const& a2)
	{
		return asyncMan->ExecuteSync([=]() { return Ivk_zCMusicSys_DirectMusic_PlayJingle(_this, a0, a1, a2); });
	}

	void __fastcall Hook_zCMusicSys_DirectMusic_StopJingle(zCMusicSys_DirectMusic*, void*, int const&);
	Hook<void(__thiscall*)(zCMusicSys_DirectMusic*, int const&)> Ivk_zCMusicSys_DirectMusic_StopJingle(ZENFOR(0x004DAEE0, 0x004EC070, 0x004E5630, 0x004E8140), &Hook_zCMusicSys_DirectMusic_StopJingle, HookMode::Patch);
	void __fastcall Hook_zCMusicSys_DirectMusic_StopJingle(zCMusicSys_DirectMusic* _this, void* vtable, int const& a0)
	{
		return asyncMan->ExecuteSync([=]() { return Ivk_zCMusicSys_DirectMusic_StopJingle(_this, a0); });
	}

	void __fastcall Hook_zCMusicSys_DirectMusic_StopJingleByScript(zCMusicSys_DirectMusic*, void*, zSTRING const&);
	Hook<void(__thiscall*)(zCMusicSys_DirectMusic*, zSTRING const&)> Ivk_zCMusicSys_DirectMusic_StopJingleByScript(ZENFOR(0x004DC880, 0x004EDD40, 0x004E70B0, 0x004E9BC0), &Hook_zCMusicSys_DirectMusic_StopJingleByScript, HookMode::Patch);
	void __fastcall Hook_zCMusicSys_DirectMusic_StopJingleByScript(zCMusicSys_DirectMusic* _this, void* vtable, zSTRING const& a0)
	{
		return asyncMan->ExecuteSync([=]() { return Ivk_zCMusicSys_DirectMusic_StopJingleByScript(_this, a0); });
	}

	class LightSearcher : public zCVobCallback
	{
	public:
		zCVob* light;

		LightSearcher() :
			light(nullptr)
		{

		}

		virtual void HandleVob(zCVob* vob, void* data) override
		{
			if (!vob || vob->type != zVOB_TYPE_LIGHT)
				return;

			if (!light || player->GetDistanceToVob2(*vob) < player->GetDistanceToVob2(*light))
				light = vob;
		}
	};

	Sub light(ZSUB(GameEvent::Loop), []()
		{
			int y = 4000;
			LOGS(COA(player, GetRightHand(), objectName));
			LOGS(AHEX32(COA(player, GetRightHand(), globalVobTreeNode)));
			LOGS(COA(player, GetRightHand(), globalVobTreeNode, GetParent(), GetData(), objectName));
			LOGS((bool)ogame->GetGameWorld()->voblist_items->IsInList((oCItem*)player->GetRightHand()));
			
			int maces = 0;

			for (oCItem* item : ogame->GetGameWorld()->voblist_items)
				if (item && item->objectName == "ITMW_1H_BAU_MACE")
					maces += 1;

			LOGS(maces);
		});
}