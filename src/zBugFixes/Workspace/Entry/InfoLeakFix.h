namespace NAMESPACE
{
	void __fastcall Hook_oCInfoManager_Destructor(oCInfoManager*, void*);
	Hook<void(__thiscall*)(oCInfoManager*), ActiveOption<bool>> Ivk_oCInfoManager_Destructor(ZENFOR(0x00664810, 0x00691A80, 0x006A4C90, 0x00702720), &Hook_oCInfoManager_Destructor, HookMode::Patch, Options::InfoLeakFix);
	void __fastcall Hook_oCInfoManager_Destructor(oCInfoManager* _this, void* vtable)
	{
		for (oCInfo*& info : _this->infoList)
		{
			delete info;
			info = nullptr;
		}

		Ivk_oCInfoManager_Destructor(_this);
	}
}