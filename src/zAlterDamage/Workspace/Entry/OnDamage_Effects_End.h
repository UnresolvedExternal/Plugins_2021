namespace NAMESPACE
{
	FASTHOOK_PATCH_OPT(oCNpc, OnDamage_Effects_End, enablePlugin);

	void oCNpc::OnDamage_Effects_End_Union(oSDamageDescriptor& desc)
	{
		// уничтожаем источник частиц (в оригинале его никогда нет)
		if (desc.pParticleFX)
		{
			desc.pParticleFX->StopEmitterOutput();
			desc.pParticleFX->Release();
			desc.pParticleFX = nullptr;
		}

		// уничтожаем связанный с генератором частиц воб (в оригинале его тоже никогда нет)
		if (desc.pVobParticleFX)
		{
			desc.pVobParticleFX->Release();
			desc.pVobParticleFX = nullptr;
		}

		// уничтожаем обычный визуал
		if (desc.pVisualFX)
		{
			desc.pVisualFX->Stop(true);
			desc.SetVisualFX(nullptr);
		}

		// снимаем флаг горения
		if (HasFlag(desc.enuModeDamage, oEDamageType_Fire))
			this->ClrBodyStateModifier(BS_MOD_BURNING);
	}
}
