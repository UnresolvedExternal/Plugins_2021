namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(KeyPress, KeyCombo{ MOUSE_BUTTONRIGHT });
		ZOPTION(KeyToggle, KeyCombo({ { KEY_LSHIFT, MOUSE_BUTTONRIGHT }, { KEY_RSHIFT, MOUSE_BUTTONRIGHT } }));
		ZOPTION(ScanRange, 5000);
		ZOPTION(ScanPeriodMs, 200);
		ZOPTION(EffectsPerFrame, 2);
		ZOPTION(EffectName, A"SPELLFX_BOLT");
	}

	namespace Options
	{
		Sub addTrivia(ZSUB(GameEvent::Execute), []()
			{
				KeyPress.endTrivia += A"... enables/disables highlighting while this key is pressed";
				KeyToggle.endTrivia += A"... enables/disables highlighting";
				ScanRange.endTrivia += A"... item scan range around the camera";
				ScanPeriodMs.endTrivia += A"... item scan period in milliseconds";
				EffectsPerFrame.endTrivia += A"... maximum effects added per frame";
				EffectName.endTrivia += A"... higlighting effect name";
			});

		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				ActiveOptionBase::LoadAll();
			});
	}
}
