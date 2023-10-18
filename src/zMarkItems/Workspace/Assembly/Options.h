namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(TrackReadDocs, true);
		ZOPTION(TrackNewItems, true);
		ZOPTION(StaleItemOnSelectOnly, false);

		ZOPTION(UnreadDocPrio, 1);
		ZOPTION(UnreadDocTexName, A"GreenBook.tga");
		ZOPTION(UnreadDocTexPos, VectorOption<int>({ 1000, 4500, 4000, 7500 }));
		ZOPTION(UnreadDocOpacity, 96);

		ZOPTION(NewItemPrio, 3);
		ZOPTION(NewItemTexName, A"GreenPlus.tga");
		ZOPTION(NewItemTexPos, VectorOption<int>({ 3900, 900, 7100, 4100 }));
		ZOPTION(NewItemOpacity, 200);

		ZOPTION(AddItemPrio, 2);
		ZOPTION(AddItemTexName, A"ArrowUp.tga");
		ZOPTION(AddItemTexPos, VectorOption<int>({ 4200, 1200, 6800, 3800 }));
		ZOPTION(AddItemOpacity, 128);

		ActiveValue<bool> HookRenderContainer;
		ActiveValue<bool> HookRenderItem;
		ActiveValue<bool> HookGetState;
	}

	namespace Options
	{
		Sub addTrivia(ZSUB(GameEvent::Execute), []()
			{
				TrackReadDocs.endTrivia += A"... enables (1) or disables (0) tracking a set of read documents";
				TrackReadDocs.endTrivia += A"when disabled marking unread documents is impossible";
				TrackReadDocs.endTrivia += A"if disabled during the game all the collected data may be lost";
				TrackReadDocs.endTrivia += A"a document is considered read when it's onstate[0] function gets executed";

				TrackNewItems.endTrivia += A"... enables (1) or disables (0) tracking the hero's inventory";
				TrackNewItems.endTrivia += A"when disabled marking new/added items is impossible";
				TrackNewItems.endTrivia += A"if disabled during the game all the collected data may be lost";
				TrackNewItems.endTrivia += A"a snapshot of hero's inventory is made when the inventory gets closed while in focus";

				StaleItemOnSelectOnly.endTrivia += A"... if enabled, new items are demarked only when selected in the inventory";
				StaleItemOnSelectOnly.endTrivia += A"otherwise, every time the inventory gets closed";

				UnreadDocPrio.startTrivia += A"If a document is unread it is marked in container cells using the following options";

				UnreadDocPrio.endTrivia += A"... sets the render order of the texture or disables the feature (0)";
				UnreadDocPrio.endTrivia += A"elements with high value are rendered last";
				UnreadDocPrio.endTrivia += A"an item itself has zero priority, so elements with negative priority are rendered behind the item";

				UnreadDocTexName.endTrivia += A"... the name of the texture unread documents will be marked by";
				
				UnreadDocTexPos.endTrivia += A"... position of the texture x1|y1|x2|y2";
				UnreadDocTexPos.endTrivia += A"a container cell bounds are 0|0|8192|8192";
				
				UnreadDocOpacity.endTrivia += A"... sets the texture opacity [0-255]";

				NewItemPrio.automaticallyAddEmptyLines = false;
				NewItemPrio.startTrivia += A"New item instances are marked in hero's inventory using the following options";

				AddItemPrio.automaticallyAddEmptyLines = false;
				AddItemPrio.startTrivia += A"";
				AddItemPrio.startTrivia += A"Old item instances which amount was increased are marked in hero's inventory using the following options";
			});

		Sub listenOptions(ZSUB(GameEvent::Execute), []()
			{
				auto setRenderHooks = []()
				{
					HookRenderItem = (NewItemPrio || AddItemPrio) && TrackNewItems || UnreadDocPrio && TrackReadDocs;
					HookRenderContainer = HookRenderItem || TrackNewItems;
				};

				TrackNewItems.onChange += setRenderHooks;
				TrackReadDocs.onChange += setRenderHooks;
				UnreadDocPrio.onChange += setRenderHooks;
				NewItemPrio.onChange += setRenderHooks;
				AddItemPrio.onChange += setRenderHooks;

				auto setGetStateFunc = []()
				{
					HookGetState = UnreadDocPrio || TrackReadDocs;
				};

				TrackReadDocs.onChange += setGetStateFunc;
				UnreadDocPrio.onChange += setGetStateFunc;
			});

		Sub load(ZSUB(GameEvent::DefineExternals), &ActiveOptionBase::LoadAll);
	}
}
