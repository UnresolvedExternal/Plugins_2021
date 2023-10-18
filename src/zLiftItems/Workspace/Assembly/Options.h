namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(KeyEnablePlugin, KeyCombo({ { KEY_LSHIFT, KEY_U }, { KEY_RSHIFT, KEY_U } }));
		ZOPTION(KeyGoToPrevItem, KeyCombo{ KEY_LEFT });
		ZOPTION(KeyGoToNextItem, KeyCombo{ KEY_RIGHT });
		ZOPTION(KeyToggleLift, KeyCombo{ MOUSE_BUTTONRIGHT });
	}

	namespace Options
	{
		Sub addTrivia(ZSUB(GameEvent::Execute), []()
			{
				KeyEnablePlugin.endTrivia += A"... key for enabling/disabling the plugin functionality";
				KeyGoToPrevItem.endTrivia += A"... key for teleporting player to previous affected item";
				KeyGoToNextItem.endTrivia += A"... key for teleporting player to next affected item";
				KeyToggleLift.endTrivia += A"... key for undo/redo items moving";
			});

		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				ActiveOptionBase::LoadAll();
			});
	}
}
