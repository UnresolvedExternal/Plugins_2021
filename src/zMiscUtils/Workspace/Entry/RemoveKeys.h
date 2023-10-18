namespace NAMESPACE
{
	const char* const scriptCheckName = "C_CanRemoveKey";

	bool NeedKey(const zSTRING& keyInstance)
	{
		for (zCVob* vob : COA(ogame, GetGameWorld(), voblist))
		{
			oCMobLockable* mob = vob->CastTo<oCMobLockable>();
		
			if (mob && mob->locked && mob->keyInstance.CompareI(keyInstance))
				return true;
		}

		return false;
	}

	bool TryRemoveKey(oCNpc* npc, const zSTRING& instance)
	{
		if (!npc->IsAPlayer())
			return false;

		if (!instance.Length())
			return false;

		if (Options::SuppressRemoveKeysKey->GetPressed())
			return false;

		oCItem* key = npc->IsInInv(instance, 0);

		if (!key)
			return false;

		if (NeedKey(instance))
			return false;

		bool firstTime = true;

		while (key)
		{
			static int scriptCheck = parser->GetIndex(scriptCheckName);

			if (scriptCheck != -1 && firstTime)
			{
				firstTime = false;
				ParserScope scope{ parser };			
				parser->SetInstance("ITEM", key);
				
				if (!CallParser<bool>(parser, scriptCheck))
					return false;
			}

			key = npc->RemoveFromInv(key, key->amount);
			ogame->GetWorld()->RemoveVob(key);
			key = npc->IsInInv(instance, 0);
		}

		return true;
	}

	void ProcessKeyRemoval(oCNpc* npc, const zSTRING& keyInstance)
	{
		if (TryRemoveKey(npc, keyInstance) && ogame->GetTextView() && parser->GetIndex(scriptCheckName) == -1)
			ogame->GetTextView()->PrintTimedCXY(Z"Key removed: " + keyInstance, 5000.0f, nullptr);
	}

	void __fastcall Hook_oCMobContainer_Open(oCMobContainer*, void*, oCNpc*);
	Hook<void(__thiscall*)(oCMobContainer*, oCNpc*), ActiveOption<bool>> Ivk_oCMobContainer_Open(ZENFOR(0x006843D0, 0x006B3FA0, 0x006C8470, 0x00726500), &Hook_oCMobContainer_Open, HookMode::Patch, Options::RemoveKeys);
	void __fastcall Hook_oCMobContainer_Open(oCMobContainer* chest, void* vtable, oCNpc* npc)
	{
		ProcessKeyRemoval(npc, chest->keyInstance);
		Ivk_oCMobContainer_Open(chest, npc);
	}

	void __fastcall Hook_oCMobDoor_Open(oCMobDoor*, void*, oCNpc*);
	Hook<void(__thiscall*)(oCMobDoor*, oCNpc*), ActiveOption<bool>> Ivk_oCMobDoor_Open(ZENFOR(0x00679250, 0x006A7890, 0x006BC3A0, 0x0071A430), &Hook_oCMobDoor_Open, HookMode::Patch, Options::RemoveKeys);
	void __fastcall Hook_oCMobDoor_Open(oCMobDoor* door, void* vtable, oCNpc* npc)
	{
		ProcessKeyRemoval(npc, door->keyInstance);
		Ivk_oCMobDoor_Open(door, npc);
	}
}
