namespace NAMESPACE
{
	oCItem* __fastcall Hook_oCNpc_PutInInv(oCNpc*, void*, oCItem*);
	Hook<oCItem*(__thiscall*)(oCNpc*, oCItem*), ActiveOption<bool>> Ivk_oCNpc_PutInInv(ZENFOR(0x006A4FF0, 0x006D7AC0, 0x006EA870, 0x00749350), &Hook_oCNpc_PutInInv, HookMode::Patch, Options::PutInInvFix);
	oCItem* __fastcall Hook_oCNpc_PutInInv(oCNpc* _this, void* vtable, oCItem* a0)
	{
		if (a0)
			a0->AddRef();

		oCItem* result = Ivk_oCNpc_PutInInv(_this, a0);

		if (a0)
		{
			ogame->GetGameWorld()->RemoveVobSubtree(a0);
			ogame->GetGameWorld()->RemoveVob(a0);
			a0->Release();
		}

		return result;
	}
}
