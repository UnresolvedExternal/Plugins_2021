namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(FilterSellItems, true);
	}

	namespace Options
	{
		Sub addTrivia(ZSUB(GameEvent::Execute), []()
			{
				FilterSellItems.endTrivia += A"... enables player's items filtering in trade dialog";
			});

		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				ActiveOptionBase::LoadAll();
			});
	}
}
