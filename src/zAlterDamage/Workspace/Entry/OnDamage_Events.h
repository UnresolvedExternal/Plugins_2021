namespace NAMESPACE
{
	FASTHOOK_PATCH_OPT(oCNpc, OnDamage_Events, enablePlugin);

	void oCNpc::OnDamage_Events_Union(oSDamageDescriptor& desc)
	{
		if (desc.bIsUnconscious)
			DropUnconscious(desc.fAzimuth, desc.pNpcAttacker);

		if (desc.bIsDead)
			DoDie(desc.pNpcAttacker);

		// останавливаем каст заклинания
		if (desc.fDamageReal >= 1.0f && !IsSelfPlayer() && !bodyStateInterruptableOverride)
			COA(GetSpellBook(), StopSelectedSpell());
	}
}
