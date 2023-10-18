namespace NAMESPACE
{
	oCItem* __fastcall Hook_oCNpc_PutInInv(oCNpc*, void*, oCItem*);
	Hook<oCItem*(__thiscall*)(oCNpc*, oCItem*), ActiveValue<bool>> Ivk_oCNpc_PutInInv(ZENFOR(0x006A4FF0, 0x006D7AC0, 0x006EA870, 0x00749350), &Hook_oCNpc_PutInInv, HookMode::Patch, Options::enablePutInInvFix);
	oCItem* __fastcall Hook_oCNpc_PutInInv(oCNpc* _this, void* vtable, oCItem* item)
	{
		if (!item || item->objectName != "ITLSTORCHBURNING")
			return Ivk_oCNpc_PutInInv(_this, item);

		item->AddRef();
		ZOwner<oCItem> owner{ item };

		COA(_this, GetHomeWorld(), CastTo<oCWorld>(), DisableVob(item));
		COA(_this, GetHomeWorld(), RemoveVob(item));

#if ENGINE == Engine_G2A
		COA(item, effectVob, RemoveVobFromWorld());
#endif

		return Ivk_oCNpc_PutInInv(_this, item);
	}
}
