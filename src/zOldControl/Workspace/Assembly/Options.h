namespace NAMESPACE
{
	namespace Options
	{
		ActiveOption<bool> UseGothic1Controls(zoptions, "GAME", "useGothic1Controls", true);
	}

	namespace Options
	{
		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				ActiveOptionBase::LoadAll();
			});
	}
}
