#if ENGINE >= Engine_G2

namespace NAMESPACE
{
	void __fastcall oCNpc_TurnToEnemy_Hook(oCNpc* _this, void* vtable)
	{
		if (_this != player)
			return _this->TurnToEnemy();

		oCAniCtrl_Human* aniCtrl = _this->GetAnictrl();

		if (!aniCtrl)
			return _this->TurnToEnemy();

		zCModelAniActive* hitAni = COA(_this, GetModel(), GetActiveAni(aniCtrl->hitAniID));
		zCModelAni* protoAni = COA(hitAni, protoAni);

		if (!protoAni || !protoAni->aniName.HasWord("JUMPB"))
			return _this->TurnToEnemy();
	}

	Sub listenJumpBackNoTurn(ZSUB(GameEvent::Execute), []()
		{
			Options::JumpBackNoTurn.onChange += []()
			{
				static std::optional<int> originalValue;

				if (!Options::JumpBackNoTurn && !originalValue.has_value())
					return;

				int address = ZENDEF(0x00000000, 0x00000000, 0x0063FA6A, 0x0069C23A);
				Unlocked<int> relativeAddress = address;
				
				if (!originalValue.has_value())
					originalValue = relativeAddress;

				if (Options::JumpBackNoTurn)
					relativeAddress = reinterpret_cast<int>(&oCNpc_TurnToEnemy_Hook) - (address + 4);
				else
				{
					relativeAddress = originalValue.value();
					originalValue.reset();
				}
			};
		});
}

#endif