namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(NameToDescCats, VectorOption<string>({ "MAGIC", "POTION", "RUNE" }));
		ZOPTION(PrintZenFocus, true);
		ZOPTION(AppendAmountInfo, true);
		ZOPTION(XChar, A"x");
		ZOPTION(CorrectModelFocusNamePos, true);

		ZOPTION(RemoveKeys, false);
		ZOPTION(SuppressRemoveKeysKey, KeyCombo{ KEY_LSHIFT });

		ZOPTION(FastFood, true);
		ZOPTION(FastFoodKey, KeyCombo{ KEY_LSHIFT });
		ZOPTION(DiaHyperskipKey, KeyCombo{ KEY_LSHIFT });

		ZOPTION(MoveLogEntry, true);

		ZOPTION(SaveTimedOverlays, true);
		ZOPTION(ReapplyOverlays, VectorOption<string>{ "HUMANS_SPRINT.MDS" });
		ZOPTION(SecondsPerGameHour, 250.0f);

#if ENGINE >= Engine_G2
		ZOPTION(PostLoadDelay, 750);
		ZOPTION(JumpBackNoTurn, true);
#endif

		ZOPTION(StrafeNoTurn, true);

		ActiveValue<bool> HookUpdatePlayerStatus;
		std::unordered_set<int> Cats;
	}

	namespace Options
	{
		Sub addTrivia(ZSUB(GameEvent::Execute), []()
			{
				NameToDescCats.endTrivia += A"... for items from listed categories the value of 'description' field will be used as focus name";
				NameToDescCats.endTrivia += A"example: 'Scroll' -> 'Firebolt'";
				NameToDescCats.endTrivia += A"all possible categories: NF|FF|MUN|ARMOR|FOOD|DOCS|POTION|LIGHT|RUNE|MAGIC";
				NameToDescCats.endTrivia += A"leave single symbol '|' to disable the feature";

				PrintZenFocus.endTrivia += A"... enables focus name drawing for objects with ZEN visuals (such as burning torches)";
				AppendAmountInfo.endTrivia += A"... adds amount info to focus names of items (example: 'Gold' -> 'Gold (x99)`)";
				XChar.endTrivia += A"... string that is used for amount info displaying";
				CorrectModelFocusNamePos.endTrivia += A"... corrects horizontal position of focus names";
				
				RemoveKeys.endTrivia += A"... opening a chest or a door forces the key removal (if the key is useless in the current world)";
				RemoveKeys.endTrivia += A"script logic isn't checked, so the option is disabled by default to preclude quest bugs";

				SuppressRemoveKeysKey.endTrivia += A"... RemoveKeys option is disabled while the key is pressed";
				FastFood.endTrivia += A"... allows hero to repeatedly use healing items with x5 speed and no action from the player";
				FastFoodKey.endTrivia += A"... consuming a healing item while this key is pressed triggers FastFood effect (if it is enabled)";
				
				DiaHyperskipKey.endTrivia += A"... skipping a dialog replique while this key is pressed incurs skipping the whole repliques chain";
				DiaHyperskipKey.endTrivia += A"write KEY_NONE to disable the feature";

				MoveLogEntry.endTrivia += A"... moves last updated topics in diary to the top";
				SaveTimedOverlays.endTrivia += A"... saves hero's temporary overlays such as sprint from potion of haste";

				ReapplyOverlays.endTrivia += A"... forces the listed overlays to be applied last";
				ReapplyOverlays.endTrivia += A"the default value may help to avoid bugs with ring of haste";

				SecondsPerGameHour.endTrivia += A"... sets the duration of game hour";
				SecondsPerGameHour.endTrivia += A"the engine default is 250.0";
				SecondsPerGameHour.endTrivia += A"non-positive value precludes the engine modification or recovers it";

#if ENGINE >= Engine_G2
				PostLoadDelay.endTrivia += A"... sets post-load delay (in milliseconds, G2 & G2A only)";
				PostLoadDelay.endTrivia += A"the engine default value is 2500";
				PostLoadDelay.endTrivia += A"set negative value to preclude the engine modification or to recover it";

				JumpBackNoTurn.endTrivia += A"... disables automatical turning to enemy while in jump back (G2 & G2A only)";
#endif

				StrafeNoTurn.endTrivia += A"... enables straightforward strafing even if an enemy is in focus";
			});

		Sub listenOptions(ZSUB(GameEvent::Execute), []()
			{
				NameToDescCats.onChange += []()
				{
					Cats.clear();

					for (string name : *NameToDescCats)
					{
						name.Shrink().Upper();

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
			});

		Sub load(ZSUB(GameEvent::DefineExternals), &ActiveOptionBase::LoadAll);
	}
}
