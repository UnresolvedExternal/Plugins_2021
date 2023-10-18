namespace NAMESPACE
{
	unsigned short keyTorchLogicalID = 0;

	bool NeedLogicalKeyTest()
	{
		static bool isArcholosMod = (ENGINE == Engine_G2A) && parser->GetSymbol("KURT_ARCHOLOS_HISTORY");
		return !isArcholosMod && Options::UseEngineHotkey;
	}

	bool GetToggled()
	{
		if (*Options::AdditionalHotkey)
			return Options::AdditionalHotkey->GetToggled();

		return NeedLogicalKeyTest() && zinput->GetToggled(keyTorchLogicalID);
	}

	// WARNING: supported versions are G2, G2A
	CInvoke<void(__cdecl*)(short, zSTRING, zCArray<unsigned short>&)> Ivk___GetDefaultBinding(ZenDef<TInstance>(0x00000000, 0x00000000, 0x004CD6A0, 0x004CFC00), nullptr, IVK_DISABLED);

	Sub useTorch(ZSUB(GameEvent::Loop), Options::EnableHotkey, []()
		{
			static int ItLsTorch = parser->GetIndex("ITLSTORCH");
			static int ItLsTorchBurning = parser->GetIndex("ITLSTORCHBURNING");

			if (ItLsTorch < 0 || ItLsTorchBurning < 0)
				return;

			if (!player || !ogame || ogame->singleStep || player->interactItem || player->interactMob)
				return;

			if (player->sleepingMode != zTVobSleepingMode::zVOB_AWAKE)
				return;

			if (zCConsole::cur_console)
				return;

			int bodyState = player->GetBodyState();

			if (bodyState == BS_JUMP || bodyState == BS_FALL || bodyState == BS_SWIM ||
				bodyState == BS_DIVE || player->IsUnconscious() || player->attribute[NPC_ATR_HITPOINTS] <= 0)
			{
				return;
			}

			if (player->GetWeaponMode() != NPC_WEAPON_NONE)
				return;

			if (COA(player, anictrl, IsInWeaponChoose()))
				return;

			if (!oCInformationManager::GetInformationManager().HasFinished())
				return;

			if (!player->GetEM()->IsEmpty(true))
				return;

			oCItem* leftHand = dynamic_cast<oCItem*>(player->GetLeftHand());

			if (leftHand && leftHand->instanz == ItLsTorchBurning)
			{
				if (!GetToggled())
					return;

				oCItem* item = static_cast<oCItem*>(player->DropFromSlot(NPC_NODE_LEFTHAND));
				
				item->AddRef();
				ogame->GetGameWorld()->RemoveVobSubtree(item);
				ogame->GetGameWorld()->RemoveVob(item);
				item->Release();

				player->SetTorchAni(false, true);

				player->CreateItems(ItLsTorch, 1);
				return;
			}

			if (leftHand)
				return;

			if (oCItem* inInventory = player->IsInInv(ItLsTorch, 1))
				if (GetToggled())
					player->UseItem(inInventory);
		});

	void BindTorchOptionKey(zCInput* _this, int mode)
	{
		zCArray<unsigned short> ctrls;
		ctrls.DeleteList();
		ctrls.Insert(KEY_T);
		ctrls.Insert(KEY_NUMPAD9);

#if ENGINE >= Engine_G2
		Ivk___GetDefaultBinding(mode, "keyTorch", ctrls);
#endif

		keyTorchLogicalID = 0;

		for (const zSKeyMapping* mapping : zinput->mapList)
			if (mapping)
				keyTorchLogicalID = std::max(keyTorchLogicalID, mapping->logicalID);

		if (keyTorchLogicalID == std::numeric_limits<decltype(keyTorchLogicalID)>::max())
			Message::Error("Cannot provide valid logical ID for keyTorch", "zTorchControl");

		keyTorchLogicalID += 1;

		_this->BindOption("keyTorch", keyTorchLogicalID, ctrls);
	}

	int lastKeyMode = 0;

	void __fastcall Hook_zCInput_BindKeys(zCInput*, void*, int);
	Hook<void(__thiscall*)(zCInput*, int), ActiveOption<bool>> Ivk_zCInput_BindKeys(ZENFOR(0x004C5930, 0x004D5420, 0x004CD890, 0x004CFE00), &Hook_zCInput_BindKeys, HookMode::Patch, Options::UseEngineHotkey);
	void __fastcall Hook_zCInput_BindKeys(zCInput* _this, void* vtable, int mode)
	{
		Ivk_zCInput_BindKeys(_this, mode);
		BindTorchOptionKey(_this, mode);
		lastKeyMode = mode;
	}

	Sub bindKey(ZSUB(GameEvent::Execute), []()
		{
			Options::UseEngineHotkey.onChange += []()
			{
				if (keyTorchLogicalID != 0)
					zinput->Unbind(keyTorchLogicalID);

				if (Options::UseEngineHotkey)
					BindTorchOptionKey(zinput, lastKeyMode);
			};
		});
}