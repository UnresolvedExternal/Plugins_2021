namespace NAMESPACE
{
	FASTHOOK_PATCH_OPT(oCNpc, OnDamage_Script, enablePlugin);

	void oCNpc::OnDamage_Script_Union(oSDamageDescriptor& desc)
	{
		// ����������� ��� ����� ������, � ����� ��� �����, ���� ��������� � �������� ������
		if (!IsSelfPlayer() || GetWeaponMode() == NPC_WEAPON_NONE)
			Interrupt(true, false);

		// ���������� ���������� AssessDamage ���������� NPC ��� ������������� �����
		if (!desc.bIsDead)
			AssessDamage_S(desc.pNpcAttacker, static_cast<int>(desc.fDamageEffective + 0.5f));
	}
}
