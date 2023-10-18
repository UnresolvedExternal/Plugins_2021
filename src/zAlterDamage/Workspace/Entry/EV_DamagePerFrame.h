namespace NAMESPACE
{
	FASTHOOK_PATCH_OPT(oCNpc, EV_DamagePerFrame, enablePlugin);

	int oCNpc::EV_DamagePerFrame_Union(oCMsgDamage* message)
	{
		oSDamageDescriptor& desc = message->descDamage;

		// �������������� ��� ������ ������
		if (!message->IsInUse())
		{
			message->SetInUse(true);

			desc.dwFieldsValid |= oEDamageDescFlag_OverlayActivate;
			desc.fTimeCurrent = desc.fTimeInterval;
			desc.fDamageTotal = desc.fDamagePerInterval;
		}

		float timeDebt = ztimer->frameTimeFloat;

		// ���� �� ���������� �� ����� ����� � OnDamage �� �������������� �� ���������...
		while (timeDebt > 0.0f && !desc.bFinished)
		{
			// ������������ �� �����, �� �� ����� �������� ���������
			const float dt = std::min(timeDebt, desc.fTimeCurrent);

			// ��������� �������
			timeDebt -= dt;
			desc.fTimeCurrent -= dt;
			desc.fTimeDuration -= dt;

			// ���� �������� ����, �� �������� OnDamage
			if (desc.fTimeCurrent <= 0.0f)
			{
				OnDamage(desc);
				desc.fTimeCurrent += desc.fTimeInterval;
			}
		}

		// ���������� true, ���� ��������� ��������� ���������� � ��� ����� �������
		return desc.bFinished;
	}
}
