namespace NAMESPACE
{
	class SlotBackuper
	{
	private:
		int minSlot;
		int maxSlot;
		int nextSlot;
		bool recalcSlots;

		uint64_t number;

		ActiveValue<bool> isMonitoring;
		Sub<ActiveValue<bool>> monitor;

		ActiveValue<bool> hasPendingSlots;
		Sub<ActiveValue<bool>> backuper;
		std::vector<int> pendingSlots;

		bool ClearDirectory(const fs::path& path, std::error_code& code)
		{
			for (const fs::directory_entry& file : fs::directory_iterator(path, code))
			{
				fs::remove_all(file.path(), code);

				if (code)
					return false;
			}

			return !code;
		}

		void RecalcSlots()
		{
			const int maxSlotNumber = ogame->savegameManager->infoList.GetNum() - 1;
			minSlot = std::clamp(*Options::BackupToSlotMin, 1, maxSlotNumber);
			maxSlot = std::clamp(*Options::BackupToSlotMax, 1, maxSlotNumber);

			number = 1;
			nextSlot = minSlot;

			for (int slot = minSlot; slot <= maxSlot; slot++)
				if (oCSavegameInfo* info = ogame->savegameManager->GetSavegame(slot))
				{
					const uint64_t current = GetNumber(info->m_Name.ToChar());

					if (current >= number)
					{
						number = current + 1;
						nextSlot = (slot == maxSlot) ? minSlot : (slot + 1);
					}
				}

			isMonitoring = Options::BackupToSlotMin <= Options::BackupToSlotMax && minSlot <= maxSlot;
		}

		void Backup(oCSavegameInfo* info)
		{
			if (maxSlot < minSlot)
				return;

			oCSavegameInfo* const targetInfo = COA(ogame, savegameManager, GetSavegame(nextSlot));

			if (!info || !targetInfo)
			{
				LogError("Can not copy from slot");
				return;
			}

			nextSlot = (nextSlot == maxSlot) ? minSlot : (nextSlot + 1);

			std::ostringstream builder;
			builder << std::setw(4) << std::setfill('0') << number++ << ".";
			builder << (info->m_SlotNr ? info->m_Name.ToChar() : "quicksave");
			const std::string& saveName = builder.str();

			builder = {};
			builder << zoptions->GetDirString(DIR_ROOT) << zoptions->GetDirString(DIR_SAVEGAMES) << info->m_Dir;
			const fs::path sourcePath{ builder.str() };

			builder = {};
			builder << zoptions->GetDirString(DIR_ROOT) << zoptions->GetDirString(DIR_SAVEGAMES) << targetInfo->m_Dir;
			const fs::path targetPath{ builder.str() };

			std::error_code code;

			if (!DurableOperation([&](auto& code) { fs::create_directories(targetPath, code); return !code; }, code))
			{
				LogError("Can not create directory: " + targetPath.string());
				LogError(code.message());
				return;
			}

			if (!DurableOperation([&](auto& code) { return !fs::equivalent(sourcePath, targetPath, code) && !code; }, code))
			{
				LogError(code ? code.message() : "Can not copy to same directory");
				return;
			}

			targetInfo->CleanResources();

			if (!DurableOperation([&](auto& code) { return ClearDirectory(targetPath, code); }, code))
			{
				LogError("Can not clear directory: " + targetPath.string());
				LogError(code.message());
				return;
			}

			if (!CopyDirContent_Durable(sourcePath, targetPath, code))
			{
				LogError("Can not copy to directory: " + targetPath.string());
				LogError(code.message());
				return;
			}

			const zSTRING saveInfo = Z targetPath.string().c_str() + "SAVEINFO.SAV";

			ZOwner<zCArchiver> arc{ zarcFactory->CreateArchiverRead(saveInfo, 0) };

			if (!arc)
			{
				LogError("Can not read save info: " + std::string(saveInfo));
				return;
			}

			arc->ReadObject(targetInfo);
			targetInfo->ReloadResources();
			arc->Close();

			arc.reset(zarcFactory->CreateArchiverWrite(saveInfo, zARC_MODE_ASCII, true, 0));

			if (!arc)
			{
				LogError("Can not write save info: " + std::string(saveInfo));
				return;
			}

			targetInfo->m_Name = saveName.c_str();
			arc->WriteObject(targetInfo);
			arc->Close();
		}

		void OnSaveEnd()
		{
			if (recalcSlots)
			{
				RecalcSlots();
				recalcSlots = false;

				if (!isMonitoring)
					return;
			}

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
		SlotBackuper() :
			number{ 1 },
			minSlot{ 0 },
			maxSlot{ -1 },
			recalcSlots{ true }
		{
			auto setRecalcSlots = [this]() { recalcSlots = true; };
			Options::BackupToSlotMin.onChange += setRecalcSlots;
			Options::BackupToSlotMax.onChange += setRecalcSlots;

			isMonitoring = true;
			monitor = { GameEvent::SaveEnd, isMonitoring, [this]() { OnSaveEnd(); } };

			backuper = { GameEvent::PreLoop | GameEvent::MenuLoop | GameEvent::Exit, hasPendingSlots, [this]() { HandlePendingSlots(); } };
		}
	};

	std::optional<SlotBackuper> slotBackuper;

	Sub initSlotBackuper(ZSUB(GameEvent::Execute), []
		{
			slotBackuper.emplace();
		});
}
