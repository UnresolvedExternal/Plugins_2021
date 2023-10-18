namespace NAMESPACE
{
	FASTHOOK_PATCH_OPT(oCNpc, OnDamage, enablePlugin);

	void oCNpc::OnDamage_Union(oSDamageDescriptor& desc)
	{
		// �������� ����������� ���� �� ��������������
		desc.bOnce = !HasFlag(desc.dwFieldsValid, oEDamageDescFlag_OverlayActivate);

		// ���� ����� � �������� ������, ������� ��������� � �������
		const bool inDialog = !oCInformationManager::GetInformationManager().HasFinished() && (IsSelfPlayer() || COA(desc.pNpcAttacker, IsSelfPlayer()));

		// ����� �� ���������� �������� �������������� �����
		desc.bFinished = false;

		// ���������� ������������� ���� � ��������, �� ��������� ������� ��� �� ���������� �������,
		// ���� ������� �������
		if (!desc.bOnce)
			desc.bFinished = inDialog || desc.fTimeDuration <= 0.0f || COA(desc.pVisualFX, IsFinished());

		// � �������� ���� �� ������� ��������� ������ �������������� �����
		if (inDialog)
			return;

		// ������ � ��������������� ��������� �������� �����-���� ����
		// ����� ��������� ���������� �������
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
