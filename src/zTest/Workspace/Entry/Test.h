#include <iomanip>
#include <fstream>

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

		void Enable(bool enable)
		{
			if (enable == enabled)
				return;

			enabled = enable;

			if (enabled)
				time = std::chrono::high_resolution_clock::now();
			else
				elapsed += std::chrono::high_resolution_clock::now() - time;
		}

		void Reset(const Duration& elapsed = {})
		{
			this->elapsed = elapsed;
		}

		Duration Elapsed() const
		{
			return elapsed;
		}
	};

	struct File
	{
		long32 handle;
		std::string name;
		std::unique_ptr<char[]> data;
		long32 size;
	};

	class Cache
	{
	private:
		std::list<File> files;
		std::unordered_map<long32, File*> byHandle;
		std::unordered_map<std::string, File*> byName;

		Hook<long32(*)(text, long32)> Ivk_vdf_fopen;
		Hook<long32(*)(long32, HBuffer, long32)> Ivk_vdf_fread;

		Cache() :
			Ivk_vdf_fopen{ &vdf_fopen, &Cache::Hook_vdf_fopen, HookMode::Patch },
			Ivk_vdf_fread{ &vdf_fread, &Cache::Hook_vdf_fread, HookMode::Patch }
		{

		}

		static long32 Hook_vdf_fopen(text fullname, long32 flags)
		{
			long32 handle = GetInstance().Ivk_vdf_fopen(fullname, flags);
			auto it = GetInstance().byName.find(fullname);

			if (it != GetInstance().byName.end())
			{
				File& file = *it->second;
				GetInstance().byHandle.erase(file.handle);
				GetInstance().byHandle.insert(std::make_pair(handle, &file));
				file.handle = handle;
				//cmd << "Handle inserted: " << endl;
				//cmd << fullname << endl;
			}
			else
			{
				//cmd << "Fail: " << endl;
				//cmd << fullname << endl;
			}

			return handle;
		}

		static long32 Hook_vdf_fread(long32 handle, HBuffer buffer, long32 size)
		{
			auto it = GetInstance().byHandle.find(handle);

			if (it != GetInstance().byHandle.end())
			{
				File& file = *it->second;
				long32 pos = vdf_ftell(handle);
				size = CoerceInRange(size, 0L, 0L, file.size - pos);
				std::copy_n(file.data.get() + pos, size, buffer);
				vdf_fseek(handle, pos + size);
				//cmd << size << " bytes read from the cache" << endl;
				return size;
			}

			return GetInstance().Ivk_vdf_fread(handle, buffer, size);
		}

	public:
		static Cache& GetInstance()
		{
			static Cache cache;
			return cache;
		}

		void CacheIn(text fullname)
		{
			files.emplace_back();
			File& file = files.back();

			file.handle = {};
			file.name = std::string("\\") + fullname;
			
			long32 handle = Ivk_vdf_fopen(fullname, VDF_DEFAULT);
			file.size = vdf_ffilesize(handle);
			file.data = std::make_unique<char[]>(file.size);
			Ivk_vdf_fread(handle, file.data.get(), file.size);
			vdf_fclose(handle);

			byName.insert(std::make_pair(file.name, &file));

			//cmd << endl;
			//cmd << "Cached:" << endl;
			//cmd << fullname << endl;
			//LOG(file.size);
		}
	};

	ActiveValue<bool> activateHook;

	void CacheTextures()
	{
		char** list;
		long32 size = vdf_filelist_virtual(list);
		
		for (long32 i = 0; i < size; i++)
		{
			if ((A list[i]).EndWith(".TEX"))
				Cache::GetInstance().CacheIn(list[i]);
		}
		
		activateHook = true;
	}

	Sub cacheTextures(ZSUB(GameEvent::Init), &CacheTextures);

	std::chrono::steady_clock::duration freadTime;
	std::optional<decltype(std::chrono::high_resolution_clock::now())> timePoint;
	std::unique_ptr<std::ofstream> out;
	uint testNr;
	uint frameNr;

	extern Hook<long32(*)(long32, HBuffer, long32), ActiveValue<bool>> Ivk_vdf_fread;

	long32 vdf_fread_hook(long32 handle, HBuffer buffer, long32 size)
	{
		using namespace std::chrono;
		
		//Message::Info("Hook");

		auto start = high_resolution_clock::now();
		auto result = Ivk_vdf_fread(handle, buffer, size);
		auto time = high_resolution_clock::now() - start;
		freadTime += time;
		return result;
	}

	Hook<long32(*)(long32, HBuffer, long32), ActiveValue<bool>> Ivk_vdf_fread{ ZTEST(&vdf_fread), &vdf_fread_hook, HookMode::Patch, activateHook };

	void ResetTimer()
	{
		timePoint.reset();
		frameNr = 0;
		testNr += 1;
		out.reset(new std::ofstream((A"time" + testNr + ".txt").GetVector()));
	}

	Sub resetTimer(ZSUB(GameEvent::LoadEnd), &ResetTimer);

	void TestPerformance()
	{
		frameNr += 1;

		int y = 2000;
		LOGS((int)frameNr);

		if (frameNr == 100)
			player->trafoObjToWorld.SetTranslation({});

		if (frameNr == 200)
			return gameMan->Read_Savegame(1);

		using namespace std::chrono;

		auto now = high_resolution_clock::now();

		if (timePoint.has_value())
		{
			auto frameTime = now - timePoint.value();
			double frameSeconds = duration_cast<nanoseconds>(frameTime).count() / 1'000'000'000.0;
			double freadSeconds = duration_cast<nanoseconds>(freadTime).count() / 1'000'000'000.0;

			(*out) << std::fixed << std::setprecision(3) << std::left <<
				std::setw(20) << freadSeconds <<
				std::setw(20) << frameSeconds <<
				freadSeconds / frameSeconds * 100.0f << "%" << std::endl;
		}
		else
			(*out) << "_first_frame_" << std::endl;

		timePoint = now;
		freadTime = {};
	}

	Sub testPerformance(ZSUB(GameEvent::Loop), &TestPerformance);
}
