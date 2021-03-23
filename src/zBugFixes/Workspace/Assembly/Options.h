namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(MidnightFix, true);
		ZOPTION(UnconsciousFix, true);
		ZOPTION(WaterWeaponFix, true);
		ZOPTION(PackStringFix, true);
		ZOPTION(GetAmountFix, true);
		ZOPTION(PutInInvFix, true);
		ZOPTION(EventThrottling, 1);
	}

	namespace Options
	{
		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				ActiveOptionBase::LoadAll();
			});
	}
}
