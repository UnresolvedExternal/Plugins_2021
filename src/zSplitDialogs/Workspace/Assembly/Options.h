namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(UnvoicedOnly, false);
	}

	namespace Options
	{
		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				ActiveOptionBase::LoadAll();
			});
	}
}
