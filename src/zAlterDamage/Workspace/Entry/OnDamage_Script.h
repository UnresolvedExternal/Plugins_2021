namespace NAMESPACE
{
	FASTHOOK_PATCH_OPT(oCNpc, OnDamage_Script, enablePlugin);

	void oCNpc::OnDamage_Script_Union(oSDamageDescriptor& desc)
	{
		// прерываются все кроме игрока, а также сам игрок, если находится в небоевом режиме
		if (!IsSelfPlayer() || GetWeaponMode() == NPC_WEAPON_NONE)
			Interrupt(true, false);

		// активируем восприятие AssessDamage окружающим NPC при несмертельном уроне
		if (!desc.bIsDead)
			AssessDamage_S(desc.pNpcAttacker, static_cast<int>(desc.fDamageEffective + 0.5f));
	}
}
