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
	Hook<int(__thiscall*)(oCAIHuman*), ActiveOption<bool>> Ivk_oCAIHuman_StandActions(ZENFOR(0x00612840, 0x006359A0, 0x0063C630, 0x00698EA0), &Hook_oCAIHuman_StandActions, HookMode::Patch, Options::UseGothic1Controls);
	int __fastcall Hook_oCAIHuman_StandActions(oCAIHuman* ai, void* vtable)
	{
		int result = Ivk_oCAIHuman_StandActions(ai);

		if (result || dynamic_cast<oCMobInter*>(ai->npc->GetFocusVob()))
			SetState(GAME_UP, false);

		return result;
	}

	void __fastcall Hook_oCAniCtrl_Human_PC_JumpForward(oCAniCtrl_Human*, void*);
	Hook<void(__thiscall*)(oCAniCtrl_Human*), ActiveOption<bool>> Ivk_oCAniCtrl_Human_PC_JumpForward(ZENFOR(0x00628C90, 0x0064DEB0, 0x00655470, 0x006B1E00), &Hook_oCAniCtrl_Human_PC_JumpForward, HookMode::Patch, Options::UseGothic1Controls);
	void __fastcall Hook_oCAniCtrl_Human_PC_JumpForward(oCAniCtrl_Human* controller, void* vtable)
	{
		if (zinput->GetState(GAME_UP))
		{
			Ivk_oCAniCtrl_Human_PC_JumpForward(controller);
			return;
		}

		if (!zinput->GetState(GAME_SMOVE))
		{
			if (!controller->targetVob)
				controller->StopLookAtTarget();

			controller->do_jump = false;
			return;
		}

		if (controller->CanJumpLedge() <= 0)
			return;

		controller->SetLookAtTarget(controller->GetLedgeInfo()->point);
		controller->StopTurnAnis();
	}

	void __fastcall Hook_oCAniCtrl_Human_PC_GoForward(oCAniCtrl_Human*, void*);
	Hook<void(__thiscall*)(oCAniCtrl_Human*), ActiveOption<bool>> Ivk_oCAniCtrl_Human_PC_GoForward(ZENFOR(0x00628C00, 0x0064DE20, 0x006553E0, 0x006B1D70), &Hook_oCAniCtrl_Human_PC_GoForward, HookMode::Patch, Options::UseGothic1Controls);
	void __fastcall Hook_oCAniCtrl_Human_PC_GoForward(oCAniCtrl_Human* controller, void* vtable)
	{
		if (controller->vob == player && zinput->GetState(GAME_ACTION)/* || player->HasBodyStateModifier(BS_JUMP))*/)
			return controller->_Stand();

		Ivk_oCAniCtrl_Human_PC_GoForward(controller);
	}

	void __fastcall Hook_oCAniCtrl_Human_PC_GoBackward(oCAniCtrl_Human*, void*);
	Hook<void(__thiscall*)(oCAniCtrl_Human*), ActiveOption<bool>> Ivk_oCAniCtrl_Human_PC_GoBackward(ZENFOR(0x00628C50, 0x0064DE70, 0x00655430, 0x006B1DC0), &Hook_oCAniCtrl_Human_PC_GoBackward, HookMode::Patch, Options::UseGothic1Controls);
	void __fastcall Hook_oCAniCtrl_Human_PC_GoBackward(oCAniCtrl_Human* controller, void* vtable)
	{
		if (controller->vob == player && zinput->GetState(GAME_ACTION))
			return controller->_Stand();

		Ivk_oCAniCtrl_Human_PC_GoBackward(controller);
	}

	int __fastcall Hook_oCAIHuman_PC_Strafe(oCAIHuman*, void*, int);
	Hook<int(__thiscall*)(oCAIHuman*, int), ActiveOption<bool>> Ivk_oCAIHuman_PC_Strafe(ZENFOR(0x00614A40, 0x00637850, 0x0063E430, 0x0069AC80), &Hook_oCAIHuman_PC_Strafe, HookMode::Patch, Options::UseGothic1Controls);
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
	Hook<void(__thiscall*)(oCNpc*, int), ActiveOption<bool>> Ivk_oCNpc_ToggleFocusVob(ZENFOR(0x00690910, 0x006C1D70, 0x006D4F90, 0x007335B0), &Hook_oCNpc_ToggleFocusVob, HookMode::Patch, Options::UseGothic1Controls);
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
	Hook<void(__thiscall*)(oCAIHuman*, int), ActiveOption<bool>> Ivk_oCAIHuman_CheckFocusVob(ZENFOR(0x00000000, 0x00000000, 0x0063EFD0, 0x0069B7A0), &Hook_oCAIHuman_CheckFocusVob, HookMode::Patch, Options::UseGothic1Controls);
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

	int __fastcall Hook_oCAIHuman_PC_CheckSpecialStates(oCAIHuman*, void*);
	Hook<int(__thiscall*)(oCAIHuman*)> Ivk_oCAIHuman_PC_CheckSpecialStates(ZENFOR(0x00612EB0, 0x006361B0, 0x0063CCC0, 0x00699510), &Hook_oCAIHuman_PC_CheckSpecialStates, HookMode::Patch);
	int __fastcall Hook_oCAIHuman_PC_CheckSpecialStates(oCAIHuman* ai, void* vtable)
	{
		const bool state = zinput->GetState(GAME_DOWN);
		SetState(GAME_DOWN, false);
		int result = Ivk_oCAIHuman_PC_CheckSpecialStates(ai);
		SetState(GAME_DOWN, state);
		return result;
	}

	Sub patchControl(ZSUB(GameEvent::Execute), []
		{
			Options::UseGothic1Controls.onChange += []
			{
				Unlocked<byte> key = ZENDEF(0x00000000, 0x00000000, 0x0063D825, 0x0069A075);
				key = Options::UseGothic1Controls ? GAME_UP : GAME_ACTION;

				Unlocked<byte> jnz = ZENDEF(0x00000000, 0x00000000, 0x0063F008, 0x0069B7D8);
				jnz = Options::UseGothic1Controls ? 0xEB : 0x75;
			};
		});

	Sub showKey(ZSUB(GameEvent::Loop), []
		{
			int y = 1000;
			LOGS((int)zinput->GetState(GAME_UP));
			LOGS((int)player->human_ai->Pressed(GAME_UP));
		});
}

#endif