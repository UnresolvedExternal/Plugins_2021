namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(Debug, false);
	}

	namespace Options
	{
		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				ActiveOptionBase::LoadAll();
			});
	}
}
