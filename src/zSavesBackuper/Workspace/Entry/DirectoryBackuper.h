namespace fs = std::filesystem;

namespace NAMESPACE
{
	class DirectoryBackuper
	{
	private:
		struct Subdir
		{
			fs::path path;
			uint64_t number;

			Subdir(const fs::path& path, const uint64_t& number) :
				path{ path },
				number{ number }
			{

			}

			bool operator<(const Subdir& y) const
			{
				return number > y.number;
			}
		};

		std::optional<fs::path> directory;
		size_t limit;
		
		uint64_t next;
		std::vector<Subdir> subdirs;
	
		ActiveValue<bool> isMonitoring;
		Sub<ActiveValue<bool>> monitor;

		std::vector<int> pendingSlots;
		ActiveValue<bool> hasPendingSlots;
		Sub<ActiveValue<bool>> backuper;

		void AddSubdirSorted(const fs::path& path, uint64_t number)
		{
			Subdir subdir{ path, number };
			auto it = std::upper_bound(subdirs.begin(), subdirs.end(), subdir);
			subdirs.insert(it, subdir);
		}

		bool InitDirectory(std::error_code& code)
		{
			if (DurableOperation([this](auto& code) { return fs::create_directories(*directory, code), !code; }, code))
				return true;
			
			if
			(
				DurableOperation([&](auto& code) { return fs::remove(*directory, code); }, code) &&
				DurableOperation([&](auto& code) { return fs::create_directories(*directory, code); }, code)
			)
			{
				return true;
			}

			return false;
		}

		bool CollectSubdirs(std::error_code& code)
		{
			code.clear();
			subdirs.clear();
			next = 1;

			for (const auto& file : fs::directory_iterator(*directory, code))
			{
				if (code)
					return false;

				if (!file.is_directory())
					continue;

				const std::string& name = file.path().filename().string();
				
				const uint64_t number = GetNumber(name);

				if (!number)
				{
					LogWarning("Directory ignored: " + name);
					continue;
				}

				subdirs += { file.path(), number };
				next = std::max(next, number + 1);
			}

			std::sort(subdirs.begin(), subdirs.end());
			return !code;
		}

		bool RemoveBeyondLimitSubdirs(bool plusOne, std::error_code& code)
		{
			while (subdirs.size() + static_cast<int>(plusOne) > limit)
				if (!DurableOperation([this](auto& code) { return fs::remove_all(subdirs.back().path); }, code))
					return false;
				else
					subdirs.pop_back();

			return true;
		}

		static void NormalizePath(std::string& path)
		{
			for (char& ch : path)
				if (ch == '\\')
					ch = '/';

			if (!path.empty() && *path.rbegin() == '/')
				path.erase(path.begin() + path.size() - 1);
		}

		static fs::path CreatePath(std::string option, std::string root, std::string saves)
		{
			NormalizePath(option);
			NormalizePath(root);
			NormalizePath(saves);

			if (option.empty())
				return root + "/";

			if (fs::path{ option }.is_absolute())
				return option;

			std::ostringstream out;
			out << root;

			if (option[0] != '/')
				out << "/";

			out << option;

			std::transform(saves.begin(), saves.end(), saves.begin(), &tolower);
			out << saves;

			return out.str();
		}

		void OnPathChanged()
		{
			try
			{
				directory = CreatePath(Options::BackupToDirPath, zoptions->GetDirString(DIR_ROOT).ToChar(), zoptions->GetDirString(DIR_SAVEGAMES).ToChar());
				isMonitoring = limit;
			}
			catch (...)
			{
				LogError("DirectoryBackuper: Invalid path");
				directory = {};
				isMonitoring = false;
			}
		}

		void OnLimitChanged()
		{
			if (Options::BackupToDirLimit < 0)
				limit = std::numeric_limits<size_t>::max();
			else
				limit = Options::BackupToDirLimit;

			isMonitoring = directory && limit;
		}

		void OnMonitoringChanged()
		{
			if (!isMonitoring)
				return;

			std::error_code code;

			if (!InitDirectory(code))
			{
				LogError("Unable to initialize directory: " + directory->string());
				LogError(code.message());
				isMonitoring = false;
				return;
			}

			if (!DurableOperation(std::bind(&DirectoryBackuper::CollectSubdirs, this, std::placeholders::_1), code))
			{
				LogError("Unable to traverse subfolders: " + directory->string());
				LogError(code.message());
				isMonitoring = false;
				return;
			}

			if (!RemoveBeyondLimitSubdirs(false, code))
			{
				LogError("Unable to remove subdirs: " + code.message());
				isMonitoring = false;
				return;
			}
		}

		void Backup(oCSavegameInfo* info)
		{
			std::error_code code;

			if (!RemoveBeyondLimitSubdirs(true, code))
			{
				LogError("Unable to remove excessive subdirs: " + code.message());
				return;
			}

			const std::string& name = GenerateSavegameName(next++, info);
			const fs::path subdirPath = *directory / name;

			if (!DurableOperation([&subdirPath](std::error_code& code) { return fs::create_directory(subdirPath, code); }, code))
			{
				LogError("Unable to create directory: " + subdirPath.string());
				LogError(code.message());
				return;
			}

			AddSubdirSorted(subdirPath, next - 1);

			if (!CopyDirContent_Durable(GetPath(info), subdirPath, code))
			{
				LogError("Unable to copy dir content: " + code.message());
				return;
			}
		}

		void OnSaveEnd()
		{
			if (SaveLoadGameInfo.slotID < 0 || SaveLoadGameInfo.changeLevel)
				return;

			if (SaveLoadGameInfo.slotID >= Options::DoNotBackupSlotMin && SaveLoadGameInfo.slotID <= Options::DoNotBackupSlotMax)
				return;

			auto it = std::find(pendingSlots.begin(), pendingSlots.end(), SaveLoadGameInfo.slotID);

			if (it != pendingSlots.end())
				pendingSlots.erase(it);

			pendingSlots += SaveLoadGameInfo.slotID;
			hasPendingSlots = true;
		}

		void HandlePendingSlots()
		{
			for (int slot : pendingSlots)
				if (oCSavegameInfo* info = COA(ogame, savegameManager, GetSavegame(slot)))
					Backup(info);
				else
					LogError((A"No info found for slot: " + slot).GetVector());

			pendingSlots.clear();
			hasPendingSlots = false;
		}

	public:
		DirectoryBackuper() :
			limit{ 0 }
		{
			isMonitoring.onChange += [this]() { OnMonitoringChanged(); };
			backuper = { GameEvent::PreLoop | GameEvent::MenuLoop | GameEvent::Exit, hasPendingSlots, [this]() { HandlePendingSlots(); } };
			monitor = { GameEvent::SaveEnd, isMonitoring, [this]() { OnSaveEnd(); } };
			Options::BackupToDirPath.onChange += [this]() { OnPathChanged(); };
			Options::BackupToDirLimit.onChange += [this]() { OnLimitChanged(); };
		}
	};

	std::optional<DirectoryBackuper> directoryBackuper;

	Sub initDirectoryBackuper(ZSUB(GameEvent::Execute), []
		{
			directoryBackuper.emplace();
		});
}
