#if ENGINE >= Engine_G2

namespace NAMESPACE
{
	void __fastcall oCNpc_TurnToEnemy_Hook(oCNpc* npc, void* vtable)
	{
		if (!Options::JumpBackNoTurn || npc != player)
			return npc->TurnToEnemy();

		oCAniCtrl_Human* const aniCtrl = npc->GetAnictrl();

		if (!aniCtrl)
			return npc->TurnToEnemy();

		zCModelAni* const protoAni = COA(npc, GetModel(), GetActiveAni(aniCtrl->hitAniID), protoAni);

		if (!protoAni || !protoAni->aniName.EndWith("PARADEJUMPB"))
			return npc->TurnToEnemy();
	}

	Sub hookTurnToEnemy(ZSUB(GameEvent::Entry), []()
		{
#pragma pack(1)
			struct CallNearInstruction
			{
				byte e8;
				int relativeAddress;
			};

			Unlocked<CallNearInstruction> call = ZENDEF(0x00000000, 0x00000000, 0x0063FA69, 0x0069C239);
			call->e8 = 0xE8;
			call->relativeAddress = reinterpret_cast<int>(&oCNpc_TurnToEnemy_Hook) - call.GetNextAddress();
		});
}

#endif