#if ENGINE >= Engine_G2

namespace NAMESPACE
{
	void ResetMusicZone()
	{
		oCZoneMusic::s_daytime = oCZoneMusic::IsDaytime();
		oCZoneMusic::s_herostatus = oCZoneMusic::GetHerostatus();
		oCZoneMusic::s_musiczone = nullptr;
	}

	void __fastcall Hook_zCMusicSys_DirectMusic_Stop(zCMusicSys_DirectMusic*, void*);
	Hook<void(__thiscall*)(zCMusicSys_DirectMusic*), ActiveOption<bool>> Ivk_zCMusicSys_DirectMusic_Stop(ZENFOR(0x004DCBF0, 0x004EE0F0, 0x004E7410, 0x004E9F20), &Hook_zCMusicSys_DirectMusic_Stop, HookMode::Patch, Options::DayMusicFix);
	void __fastcall Hook_zCMusicSys_DirectMusic_Stop(zCMusicSys_DirectMusic* _this, void* vtable)
	{
		Ivk_zCMusicSys_DirectMusic_Stop(_this);
		ResetMusicZone();
	}
}

#endif