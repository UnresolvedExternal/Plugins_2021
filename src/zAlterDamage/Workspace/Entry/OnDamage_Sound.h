namespace NAMESPACE
{
	FASTHOOK_PATCH_OPT(oCNpc, OnDamage_Sound, enablePlugin);

	void oCNpc::OnDamage_Sound_Union(oSDamageDescriptor& desc)
	{
		// проверяем, включен ли звук
		if (!zsound)
			return;

		// определяем название звука
		zSTRING soundName = Z"SVM_" + Z voice;

		if (desc.bIsDead)
			soundName += "_DEAD";
		// звуков боли может быть несколько вариантов...
		else
		{
			soundName += "_AARGH";

			int variations = 5;

			if (Symbol variationsSymbol{ parser, "NPC_VOICE_VARIATION_MAX" })
				variations = variationsSymbol.GetValue<int>(0);

			if (const int random = (variations <= 1) ? 0 : (rand() % variations))
				soundName += Z"_" + Z random;
		}

		zCSoundSystem::zTSound3DParams params;
		params.SetDefaults();
		params.pitchOffset = voicePitch;

		listOfVoiceHandles.InsertEnd(zsound->PlaySound3D(soundName, this, 2, &params));
	}
}
