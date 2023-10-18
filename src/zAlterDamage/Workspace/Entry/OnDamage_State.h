namespace NAMESPACE
{
	FASTHOOK_OPT(oCNpc, OnDamage_State, enablePlugin);

	void oCNpc::OnDamage_State_Union(oSDamageDescriptor& desc)
	{
		// ���� �������� �������, ������������� ����������� ��������� ����
		if
		(
			!HasBodyStateModifier(BS_MOD_BURNING) &&
			HasFlag(desc.enuModeDamage, oEDamageType_Fire) &&
			desc.aryDamageEffective[oEDamageIndex_Fire] >= 2.0f &&
			COA(GetAnictrl(), GetWaterLevel()) == 0
		)
		{
			SetBodyStateModifier(BS_MOD_BURNING);
		}
	}
}
