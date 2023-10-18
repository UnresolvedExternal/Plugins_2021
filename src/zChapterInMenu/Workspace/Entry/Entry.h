namespace NAMESPACE
{
	void __fastcall Hook_oCMenu_Log_SetDayTime(oCMenu_Log*, void*, zSTRING const&, zSTRING const&);
	Hook<void(__thiscall*)(oCMenu_Log*, zSTRING const&, zSTRING const&)> Ivk_oCMenu_Log_SetDayTime(ZENFOR(0x00474D80, 0x0047E560, 0x0047AA20, 0x0047BEF0), &Hook_oCMenu_Log_SetDayTime, HookMode::Patch);
	void __fastcall Hook_oCMenu_Log_SetDayTime(oCMenu_Log* menu, void* vtable, zSTRING const& day, zSTRING const& time)
	{
		Ivk_oCMenu_Log_SetDayTime(menu, day, time);

		if (Symbol chapter{ parser, "Kapitel" })
			if (chapter.GetType() == Symbol::Type::VarInt)
				if (ZOwner<zCMenuItem> item{ zCMenuItem::GetByName("MENU_ITEM_LOG_CHAPTER") })
					item->SetText(chapter.GetValue<int>(0), 0, false);
	}

	class ChapterSaveData : public SaveData
	{
	public:
		int chapter;

		ChapterSaveData(const string& name) :
			SaveData{ name },
			chapter{ Invalid }
		{

		}

		virtual void Clear() override
		{
			chapter = Invalid;
		}

		virtual void Archive(zCArchiver& arc) override
		{
			arc.WriteInt("Version", 1);
			arc.WriteInt("Chapter", chapter);
		}

		virtual void Unarchive(zCArchiver& arc) override
		{
			const int version = arc.ReadInt("Version");

			if (version >= 1)
				chapter = arc.ReadInt("Chapter");
		}
	};

	ChapterSaveData& GetChapterSaveData()
	{
		static ChapterSaveData& data = SaveData::Get<ChapterSaveData>("SaveInfoMenu");
		return data;
	}

	std::unordered_map<oCSavegameInfo*, int> chapters;

	void __fastcall Hook_oCSavegameInfo_Archive(oCSavegameInfo*, void*, zCArchiver&);
	Hook<void(__thiscall*)(oCSavegameInfo*, zCArchiver&)> Ivk_oCSavegameInfo_Archive(ZENFOR(0x00434850, 0x00438920, 0x00437470, 0x00437970), &Hook_oCSavegameInfo_Archive, HookMode::Patch);
	void __fastcall Hook_oCSavegameInfo_Archive(oCSavegameInfo* info, void* vtable, zCArchiver& arc)
	{
		Ivk_oCSavegameInfo_Archive(info, arc);

		if (SaveLoadGameInfo.slotID != -3)
			if (Symbol chapter{ parser, "Kapitel" })
				if (chapter.GetType() == Symbol::Type::VarInt)
				{
					GetChapterSaveData().chapter = chapter.GetValue<int>(0);
					chapters[info] = GetChapterSaveData().chapter;
					GetChapterSaveData().Save(GameEvent::SaveEnd);
					GetChapterSaveData().Clear();
				}
	}

	void __fastcall Hook_oCSavegameInfo_Unarchive(oCSavegameInfo*, void*, zCArchiver&);
	Hook<void(__thiscall*)(oCSavegameInfo*, zCArchiver&)> Ivk_oCSavegameInfo_Unarchive(ZENFOR(0x00434C40, 0x00438DC0, 0x004379C0, 0x00437EC0), &Hook_oCSavegameInfo_Unarchive, HookMode::Patch);
	void __fastcall Hook_oCSavegameInfo_Unarchive(oCSavegameInfo* info, void* vtable, zCArchiver& arc)
	{
		Ivk_oCSavegameInfo_Unarchive(info, arc);

		if (!arc.GetFile())
		{
			chapters.erase(info);
			return;
		}

		const zSTRING path = arc.GetFile()->GetDirectoryPath() + "SAVEINFOMENU.SAV";

		if (ZOwner<zCArchiver> archiver{ zarcFactory->CreateArchiverRead(path, 0) })
		{
			GetChapterSaveData().Clear();
			GetChapterSaveData().Unarchive(*archiver);
			archiver->Close();
			chapters[info] = GetChapterSaveData().chapter;
			GetChapterSaveData().Clear();
		}
		else
			chapters.erase(info);
	}

	void __fastcall Hook_oCMenuSavegame_HandleSlotChange(oCMenuSavegame*, void*, int);
	Hook<void(__thiscall*)(oCMenuSavegame*, int)> Ivk_oCMenuSavegame_HandleSlotChange(ZENFOR(0x0042C420, 0x0042F500, 0x0042E620, 0x0042E940), &Hook_oCMenuSavegame_HandleSlotChange, HookMode::Patch);
	void __fastcall Hook_oCMenuSavegame_HandleSlotChange(oCMenuSavegame* menu, void* vtable, int oldSlot)
	{
		Ivk_oCMenuSavegame_HandleSlotChange(menu, oldSlot);

		if (ZOwner<zCMenuItem> item{ zCMenuItem::GetByName("MENU_ITEM_LOADSAVE_CHAPTER") })
		{
			auto it = chapters.find(menu->savegameManager->GetSavegame(menu->m_selSlot));

			if (it == chapters.end())
				item->SetText("", 0, true);
			else
				item->SetText(it->second, 0, true);
		}

		if (!menu->m_item_WorldName)
			return;

		if (Symbol worldName{ parserMenu, Z"WORLDNAME_" + menu->m_item_WorldName->GetText(0) })
			if (worldName.GetType() == Symbol::Type::VarString)
				menu->m_item_WorldName->SetText(worldName.GetValue<string>(0), 0, true);
	}
}
