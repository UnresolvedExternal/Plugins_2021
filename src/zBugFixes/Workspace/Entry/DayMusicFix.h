#if ENGINE >= Engine_G2

namespace NAMESPACE
{
	Sub dayMusicFix(ZSUB(GameEvent::LoadEnd), Options::DayMusicFix, []()
		{
			oCZoneMusic::s_daytime = oCZoneMusic::IsDaytime();
			oCZoneMusic::s_herostatus = oCZoneMusic::GetHerostatus();
			oCZoneMusic::s_musiczone = nullptr;
		});
}

#endif