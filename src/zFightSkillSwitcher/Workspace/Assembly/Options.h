namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(IncreaseSkillKey, KeyCombo({ { KEY_RSHIFT, KEY_NUMPAD1 } }));
		ZOPTION(DecreaseSkillKey, KeyCombo({ { KEY_RSHIFT, KEY_NUMPAD0 } }));
	}

	namespace Options
	{
		Sub addTrivia(ZSUB(GameEvent::Execute), []()
			{
				IncreaseSkillKey.endTrivia += A"... key for weapon skill increasing";
				DecreaseSkillKey.endTrivia += A"... key for weapon skill decreasing";
			});

		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				ActiveOptionBase::LoadAll();
			});
	}
}
