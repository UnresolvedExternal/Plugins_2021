namespace NAMESPACE
{
	void __fastcall Hook_oCInfoManager_Unarchive(oCInfoManager*, void*, zCArchiver&);
	Hook<void(__thiscall*)(oCInfoManager*, zCArchiver&)> Ivk_oCInfoManager_Unarchive(ZENFOR(0x00665330, 0x00692610, 0x006A57B0, 0x00703240), &Hook_oCInfoManager_Unarchive, HookMode::Patch);
	void __fastcall Hook_oCInfoManager_Unarchive(oCInfoManager* _this, void* vtable, zCArchiver& arc)
	{
		Ivk_oCInfoManager_Unarchive(_this, arc);

		if (!arc.InSaveGame())
			return;

		zCPar_Symbol* c_info = parser->GetSymbol("C_INFO");

		if (!c_info)
			return;

		for (int i = 0; i < parser->symtab.GetNumInList(); i++)
		{
			Symbol symbol{ parser, i };

			if (symbol.GetType() != Symbol::Type::Instance)
				continue;

			if (symbol.GetSymbol()->parent != c_info && COA(symbol.GetSymbol()->parent, parent) != c_info)
				continue;

			bool hasInfo = false;

			for (oCInfo* info : _this->infoList)
				if (info->instance == i)
				{
					hasInfo = true;
					break;
				}

			if (hasInfo)
				continue;

			oCInfo* info = new oCInfo{};
			parser->CreateInstance(i, info->GetDataAdr());
			info->SetInstance(i);
			info->DoCheck();
			_this->infoList.InsertSort(info);
		}
	}
}