namespace NAMESPACE
{
	namespace Options
	{
		ActiveOption<bool> UseGothic1Controls(zoptions, "GAME", "useGothic1Controls", true);
		ZOPTION(IgnoreUseGothic1Controls, true);

		ActiveValue<bool> DoStuff;
	}

	namespace Options
	{
		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				IgnoreUseGothic1Controls.endTrivia += A"... forces plugin to work despite 'UseGothic1Controls' option value";

				auto checkDoStuff = []
				{
					DoStuff = UseGothic1Controls || IgnoreUseGothic1Controls;
				};

				UseGothic1Controls.onChange += checkDoStuff;
				IgnoreUseGothic1Controls.onChange += checkDoStuff;

				ActiveOptionBase::LoadAll();
			});
	}
}
