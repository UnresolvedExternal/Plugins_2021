#if ENGINE >= Engine_G2

namespace NAMESPACE
{
	void SetState(unsigned short logicalKey, bool state)
	{
		zinput->SetState(logicalKey, state);

		if (state)
			return;

		zSKeyMapping helper;
		helper.logicalID = logicalKey;

		const int index = zinput->mapList.Search(&helper);

		if (index == -1)
			return;

		for (auto key : zinput->mapList[index]->controlValues)
			zinput->SetKey(key, state);
	}

	int __fastcall Hook_oCAIHuman_StandActions(oCAIHuman*, void*);
	Hook<int(__thiscall*)(oCAIHuman*), ActiveValue<bool>> Ivk_oCAIHuman_StandActions(ZENFOR(0x00612840, 0x006359A0, 0x0063C630, 0x00698EA0), &Hook_oCAIHuman_StandActions, HookMode::Patch, Options::DoStuff);
	int __fastcall Hook_oCAIHuman_StandActions(oCAIHuman* ai, void* vtable)
	{
		int result = Ivk_oCAIHuman_StandActions(ai);

		if (result || dynamic_cast<oCMobInter*>(ai->npc->GetFocusVob()))
			SetState(GAME_UP, false);

		return result;
	}

	void __fastcall Hook_oCAniCtrl_Human_PC_JumpForward(oCAniCtrl_Human*, void*);
	Hook<void(__thiscall*)(oCAniCtrl_Human*), ActiveValue<bool>> Ivk_oCAniCtrl_Human_PC_JumpForward(ZENFOR(0x00628C90, 0x0064DEB0, 0x00655470, 0x006B1E00), &Hook_oCAniCtrl_Human_PC_JumpForward, HookMode::Patch, Options::DoStuff);
	void __fastcall Hook_oCAniCtrl_Human_PC_JumpForward(oCAniCtrl_Human* controller, void* vtable)
	{
		if (zinput->GetState(GAME_UP))
		{
			Ivk_oCAniCtrl_Human_PC_JumpForward(controller);
			return;
		}
		
		if (zinput->GetState(GAME_SMOVE) && controller->IsWalking())
			controller->_Stand();

		if (!zinput->GetState(GAME_SMOVE))
		{
			if (!controller->targetVob)
				controller->StopLookAtTarget();

			controller->do_jump = false;
			return;
		}

		controller->StopTurnAnis();

		if (controller->CanJumpLedge() <= 0)
			return;

		controller->SetLookAtTarget(controller->GetLedgeInfo()->point);
	}

	void __fastcall Hook_oCAniCtrl_Human_PC_GoForward(oCAniCtrl_Human*, void*);
	Hook<void(__thiscall*)(oCAniCtrl_Human*), ActiveValue<bool>> Ivk_oCAniCtrl_Human_PC_GoForward(ZENFOR(0x00628C00, 0x0064DE20, 0x006553E0, 0x006B1D70), &Hook_oCAniCtrl_Human_PC_GoForward, HookMode::Patch, Options::DoStuff);
	void __fastcall Hook_oCAniCtrl_Human_PC_GoForward(oCAniCtrl_Human* controller, void* vtable)
	{
		if (controller->vob == player && zinput->GetState(GAME_ACTION) || zinput->GetState(GAME_SMOVE))
			return controller->_Stand();

		Ivk_oCAniCtrl_Human_PC_GoForward(controller);
	}

	void __fastcall Hook_oCAniCtrl_Human_PC_GoBackward(oCAniCtrl_Human*, void*);
	Hook<void(__thiscall*)(oCAniCtrl_Human*), ActiveValue<bool>> Ivk_oCAniCtrl_Human_PC_GoBackward(ZENFOR(0x00628C50, 0x0064DE70, 0x00655430, 0x006B1DC0), &Hook_oCAniCtrl_Human_PC_GoBackward, HookMode::Patch, Options::DoStuff);
	void __fastcall Hook_oCAniCtrl_Human_PC_GoBackward(oCAniCtrl_Human* controller, void* vtable)
	{
		if (controller->vob == player && zinput->GetState(GAME_ACTION))
			return controller->_Stand();

		Ivk_oCAniCtrl_Human_PC_GoBackward(controller);
	}

	int __fastcall Hook_oCAIHuman_PC_Strafe(oCAIHuman*, void*, int);
	Hook<int(__thiscall*)(oCAIHuman*, int), ActiveValue<bool>> Ivk_oCAIHuman_PC_Strafe(ZENFOR(0x00614A40, 0x00637850, 0x0063E430, 0x0069AC80), &Hook_oCAIHuman_PC_Strafe, HookMode::Patch, Options::DoStuff);
	int __fastcall Hook_oCAIHuman_PC_Strafe(oCAIHuman* controller, void* vtable, int pressed)
	{
		if (pressed && zinput->GetState(GAME_ACTION))
		{
			SetState(GAME_STRAFELEFT, false);
			SetState(GAME_STRAFERIGHT, false);
			SetState(GAME_LEFT, false);
			SetState(GAME_RIGHT, false);
			return true;
		}

		int result = Ivk_oCAIHuman_PC_Strafe(controller, pressed);

		if (!result && controller->npc && controller->npc->GetFocusNpc())
			if (controller->npc->GetFocusNpc()->state.IsInState(parser->GetIndex("ZS_TALK")))
				return true;

		return result;
	}

	void __fastcall Hook_oCNpc_ToggleFocusVob(oCNpc*, void*, int);
	Hook<void(__thiscall*)(oCNpc*, int), ActiveValue<bool>> Ivk_oCNpc_ToggleFocusVob(ZENFOR(0x00690910, 0x006C1D70, 0x006D4F90, 0x007335B0), &Hook_oCNpc_ToggleFocusVob, HookMode::Patch, Options::DoStuff);
	void __fastcall Hook_oCNpc_ToggleFocusVob(oCNpc* npc, void* vtable, int force)
	{
		Ivk_oCNpc_ToggleFocusVob(npc, force);

		SetState(GAME_STRAFELEFT, false);
		SetState(GAME_STRAFERIGHT, false); 
		SetState(GAME_LEFT, false);
		SetState(GAME_RIGHT, false);
	}

	// WARNING: supported versions are G2, G2A
	void __fastcall Hook_oCAIHuman_CheckFocusVob(oCAIHuman*, void*, int);
	Hook<void(__thiscall*)(oCAIHuman*, int), ActiveValue<bool>> Ivk_oCAIHuman_CheckFocusVob(ZENFOR(0x00000000, 0x00000000, 0x0063EFD0, 0x0069B7A0), &Hook_oCAIHuman_CheckFocusVob, HookMode::Patch, Options::DoStuff);
	void __fastcall Hook_oCAIHuman_CheckFocusVob(oCAIHuman* ai, void* vtable, int force)
	{
		Ivk_oCAIHuman_CheckFocusVob(ai, force);

		if (ai->vob != player)
			return;

		if (zinput->GetState(GAME_ACTION) && oCInformationManager::GetInformationManager().HasFinished())
			ai->SetLookAtTarget(ai->npc->GetFocusVob());
		else
			if (!zinput->GetState(GAME_LOOK))
				ai->StopLookAtTarget();
	}

	Sub patchControl(ZSUB(GameEvent::Execute), []
		{
			Options::DoStuff.onChange += []
			{
				Unlocked<byte> key = ZENDEF(0x00000000, 0x00000000, 0x0063D825, 0x0069A075);
				key = Options::DoStuff ? GAME_UP : GAME_ACTION;

				Unlocked<byte> jnz = ZENDEF(0x00000000, 0x00000000, 0x0063F008, 0x0069B7D8);
				jnz = Options::DoStuff ? 0xEB : 0x75;
			};
		});

	int __fastcall Hook_oCGame_HandleEvent(oCGame*, void*, int);
	Hook<int(__thiscall*)(oCGame*, int), ActiveValue<bool>> Ivk_oCGame_HandleEvent(ZENFOR(0x0065EEE0, 0x0068A300, 0x0069E980, 0x006FC170), &Hook_oCGame_HandleEvent, HookMode::Patch, Options::DoStuff);
	int __fastcall Hook_oCGame_HandleEvent(oCGame* _this, void* vtable, int a0)
	{
		int result = Ivk_oCGame_HandleEvent(_this, a0);

		if (result)
			return result;

		if (player->GetEM()->IsEmpty(true))
			if (a0 == KEY_GRAVE && zinput->KeyToggled(KEY_GRAVE))
			{
				COA(player, GetSpellBook(), Close(true));

				if (player->inventory2.IsOpen())
					player->inventory2.Close();

				if (oCMobContainer* mob = player->interactMob->CastTo<oCMobContainer>())
				{
					mob->Close(player);
					mob->Reset();
				}

				if (player->GetWeaponMode() == NPC_WEAPON_FIST)
					player->GetEM()->OnMessage(new oCMsgWeapon(oCMsgWeapon::EV_REMOVEWEAPON, 0, 0), player);
				else if (player->GetWeaponMode() == NPC_WEAPON_NONE && player->CanDrawWeapon())
					player->GetEM()->OnMessage(new oCMsgWeapon(oCMsgWeapon::EV_DRAWWEAPON, NPC_WEAPON_FIST, 1), player);
				else if (player->GetWeaponMode() > NPC_WEAPON_FIST)
				{
					player->GetEM()->OnMessage(new oCMsgWeapon(oCMsgWeapon::EV_REMOVEWEAPON, 0, 0), player);
					player->GetEM()->OnMessage(new oCMsgWeapon(oCMsgWeapon::EV_DRAWWEAPON, NPC_WEAPON_FIST, 1), player);
				}

				return true;
			}

		return result;
	}

	Sub resetLookTarget(ZSUB(GameEvent::Loop), Options::DoStuff, []
		{
			if (zCVob* target = player->GetAnictrl()->targetVob)
				if (target->GetHomeWorld() != player->GetHomeWorld())
					player->GetAnictrl()->StopLookAtTarget();
		});
}

#endif