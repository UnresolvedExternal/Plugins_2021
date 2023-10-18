namespace NAMESPACE
{
	FASTHOOK_PATCH_OPT(oCNpc, OnDamage, enablePlugin);

	void oCNpc::OnDamage_Union(oSDamageDescriptor& desc)
	{
		// отличаем однократный урон от периодического
		desc.bOnce = !HasFlag(desc.dwFieldsValid, oEDamageDescFlag_OverlayActivate);

		// если атака с участием игрока, который находится в диалоге
		const bool inDialog = !oCInformationManager::GetInformationManager().HasFinished() && (IsSelfPlayer() || COA(desc.pNpcAttacker, IsSelfPlayer()));

		// нужно ли прекратить действие периодического урона
		desc.bFinished = false;

		// прекращаем периодический урон в диалогах, по истечению времени или по завершении визуала,
		// если таковой имеется
		if (!desc.bOnce)
			desc.bFinished = inDialog || desc.fTimeDuration <= 0.0f || COA(desc.pVisualFX, IsFinished());

		// в диалогах даже не снимаем визульный эффект периодического урона
		if (inDialog)
			return;

		// мёртвые и бессознательные перестают получать какой-либо урон
		// также снимаются визуальные эффекты
		if (!IsConditionValid())
		{
			if (desc.bFinished)
				OnDamage_Effects_End(desc);

			return;
		}

		OnDamage_Hit(desc);
		OnDamage_Condition(desc);

		if (desc.bOnce)
		{
			OnDamage_Anim(desc);
			OnDamage_Effects_Start(desc);
			OnDamage_Script(desc);
			OnDamage_State(desc);
		}

		OnDamage_Events(desc);

		if (desc.bOnce)
			OnDamage_Sound(desc);

		if (desc.bFinished)
			OnDamage_Effects_End(desc);
	}
}
