namespace NAMESPACE
{
	namespace Settings
	{
		ZOPTION(NameToDescCats, VectorOption<string>({ "MAGIC", "POTION", "RUNE" }));
		ZOPTION(PrintZenFocus, true);
		ZOPTION(AppendAmountInfo, true);
		ZOPTION(XChar, A"x");
		ZOPTION(CorrectModelFocusNamePos, true);

		ZOPTION(RemoveKeys, false);
		ZOPTION(SuppressRemoveKeysKey, KeyCombo({ { KEY_LSHIFT } }));

		ZOPTION(FastFood, true);
		ZOPTION(FastFoodKey, KeyCombo({ { KEY_LSHIFT } }));
		ZOPTION(MoveLogEntry, true);
		ZOPTION(DiaHyperskipKey, KeyCombo({ { KEY_LSHIFT } }));

		ActiveValue<bool> HookUpdatePlayerStatus;
		std::unordered_set<int> Cats;
	}

	namespace Settings
	{
		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				NameToDescCats.onChange += []()
				{
					Cats.clear();

					for (string name : *NameToDescCats)
					{
						name.Upper();

						if (name == "NF") Cats.insert(ITM_CAT_NF);
						if (name == "FF") Cats.insert(ITM_CAT_FF);
						if (name == "MUN") Cats.insert(ITM_CAT_MUN);
						if (name == "ARMOR") Cats.insert(ITM_CAT_ARMOR);
						if (name == "FOOD") Cats.insert(ITM_CAT_FOOD);
						if (name == "DOCS") Cats.insert(ITM_CAT_DOCS);
						if (name == "POTION") Cats.insert(ITM_CAT_POTION);
						if (name == "LIGHT") Cats.insert(ITM_CAT_LIGHT);
						if (name == "RUNE") Cats.insert(ITM_CAT_RUNE);
						if (name == "MAGIC") Cats.insert(ITM_CAT_MAGIC);
					}
				};

				auto setUpdatePlayerStatusHook = []()
				{
					HookUpdatePlayerStatus = PrintZenFocus || !Cats.empty() || AppendAmountInfo || CorrectModelFocusNamePos;
				};

				PrintZenFocus.onChange += setUpdatePlayerStatusHook;
				NameToDescCats.onChange += setUpdatePlayerStatusHook;
				AppendAmountInfo.onChange += setUpdatePlayerStatusHook;
				CorrectModelFocusNamePos.onChange += setUpdatePlayerStatusHook;

				ActiveOptionBase::LoadAll();
			});
	}
}
