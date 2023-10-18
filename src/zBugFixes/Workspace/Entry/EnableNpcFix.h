namespace NAMESPACE
{
	void(__fastcall* oCAIHuman_HostVobAddedToWorld)(oCAIHuman* ai, void* vtable, zCVob* vob, zCWorld* world);

	void __fastcall Hook_oCAIHuman_HostVobAddedToWorld(oCAIHuman* ai, void* vtable, zCVob* vob, zCWorld* world)
	{
		if (vob == ai->vob)
			ai->world = world;

		if (oCAIHuman_HostVobAddedToWorld)
			oCAIHuman_HostVobAddedToWorld(ai, vtable, vob, world);
	}

	Sub enableNpcFix(ZSUB(GameEvent::Execute), []
		{
			Options::EnableNpcFix.onChange += []
			{
				static bool firstTime = true;
				vfunc& func = vftable_oCAIHuman::GetTable().names.f10_HostVobAddedToWorld;

				if (firstTime)
				{
					oCAIHuman_HostVobAddedToWorld = reinterpret_cast<decltype(oCAIHuman_HostVobAddedToWorld)>(func.pointer);
					firstTime = false;
				}

				func = Options::EnableNpcFix ? TInstance{ &Hook_oCAIHuman_HostVobAddedToWorld } : TInstance{ oCAIHuman_HostVobAddedToWorld };
			};
		});
}
