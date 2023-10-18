namespace NAMESPACE
{
	FASTHOOK_PATCH_OPT(oCNpc, OnDamage_Effects_End, enablePlugin);

	void oCNpc::OnDamage_Effects_End_Union(oSDamageDescriptor& desc)
	{
		// ���������� �������� ������ (� ��������� ��� ������� ���)
		if (desc.pParticleFX)
		{
			desc.pParticleFX->StopEmitterOutput();
			desc.pParticleFX->Release();
			desc.pParticleFX = nullptr;
		}

		// ���������� ��������� � ����������� ������ ��� (� ��������� ��� ���� ������� ���)
		if (desc.pVobParticleFX)
		{
			desc.pVobParticleFX->Release();
			desc.pVobParticleFX = nullptr;
		}

		// ���������� ������� ������
		if (desc.pVisualFX)
		{
			desc.pVisualFX->Stop(true);
			desc.SetVisualFX(nullptr);
		}

		// ������� ���� �������
		if (HasFlag(desc.enuModeDamage, oEDamageType_Fire))
			this->ClrBodyStateModifier(BS_MOD_BURNING);
	}
}
