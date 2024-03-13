META
{
	Parser = Menu;
	After = zUnionMenu.d;
	Namespace = zBugFixes;
};

const int HEADLINE_Y = 550;
const int DY = 550;
const int TEXT_X = 450;
const int TEXT_Y = 1650;
const int CHOICE_X = 5500;
const int CHOICE_DIMX = 2200;
const int SLIDER_Y = 1500;

prototype C_MENU_ITEM_TEXT(C_MENU_ITEM)
{
	C_MENU_ITEM_DEF();
	fontname = MENU_FONT_SMALL;
	flags = flags | IT_EFFECTS_NEXT;
	posx = TEXT_X;
	posy = TEXT_Y;
	onselaction[0] = SEL_ACTION_UNDEF;
};

prototype C_MENU_ITEM_CHOICE(C_MENU_ITEM)
{
	C_MENU_ITEM_DEF();
	backpic = MENU_CHOICE_BACK_PIC;
	type = MENU_ITEM_CHOICEBOX;
	fontname = MENU_FONT_SMALL;
	flags = flags & ~IT_SELECTABLE;
    flags = flags | IT_TXT_CENTER;
	posx = CHOICE_X;
	posy = TEXT_Y;
	dimx = CHOICE_DIMX;
	onchgsetoptionsection = "zBugFixes";
	
	text[0] = Str_GetLocalizedString(
		"нет|да", 
		"no|yes", 
		"Nein|Ja", 
		"nie|tak"
	);
};

prototype C_MENU_ITEM_SLIDER(C_MENU_ITEM)
{
	C_MENU_ITEM_DEF();
	backpic = MENU_SLIDER_BACK_PIC;
	type = MENU_ITEM_SLIDER;
	userstring[0] = MENU_SLIDER_POS_PIC;
	userfloat[0] = 10.0;
	posx = CHOICE_X;
	posy = SLIDER_Y;
	dimx = CHOICE_DIMX;
	flags = flags & ~IT_SELECTABLE;
	onchgsetoptionsection = "zBugFixes";
};

instance :MenuItem_Union_Auto_zBugFixes(C_MENU_ITEM_UNION_DEF)
{
	text[0] = "zBugFixes";
	
	text[1] = Str_GetLocalizedString(
		"Настроить zBugFixes",
		"Configure zBugFixes",
		"Configure zBugFixes",
		"Configure zBugFixes"
	);
	
	onselaction[0] = SEL_ACTION_STARTMENU;
	onselaction_s[0] = "zBugFixes:Menu_Options";
};

instance Menu_Options(C_MENU_DEF)
{
	backpic = "MENU_INGAME.TGA";
    items[0] = "";
    flags = flags | MENU_SHOW_INFO;
	Menu_SearchItems("zBugFixes:MenuItem_*");
};

instance MenuItem_HeadLine(C_MENU_ITEM_TEXT)
{
	fontname = MENU_FONT_DEFAULT;
	flags = flags & ~IT_SELECTABLE;
    flags = flags | IT_TXT_CENTER;
	posx = 0;
	posy = HEADLINE_Y;
	dimx = 8192;
	
	text[0] = "zBugFixes";
};

instance MenuItem_PackStringFix_Text(C_MENU_ITEM_TEXT)
{
	posy += 0 * DY;
	
	text[0] = Str_GetLocalizedString(
		"Устранить ошибки распаковки инвентаря", 
		"Fix inventory unpacking bugs", 
		"Fix inventory unpacking bugs", 
		"Fix inventory unpacking bugs"
	);
};

instance MenuItem_PackStringFix_Choice(C_MENU_ITEM_CHOICE)
{
	posy += 0 * DY;
	onchgsetoption = "PackStringFix";
};

instance MenuItem_GetAmountFix_Text(C_MENU_ITEM_TEXT)
{
	posy += 1 * DY;
	
	text[0] = Str_GetLocalizedString(
		"Исправить oCNpcInventory::GetAmount", 
		"Fix oCNpcInventory::GetAmount", 
		"Fix oCNpcInventory::GetAmount", 
		"Fix oCNpcInventory::GetAmount"
	);
};

instance MenuItem_GetAmountFix_Choice(C_MENU_ITEM_CHOICE)
{
	posy += 1 * DY;
	onchgsetoption = "GetAmountFix";
};

instance MenuItem_PutInInvFix_Text(C_MENU_ITEM_TEXT)
{
	posy += 2 * DY;

	text[0] = Str_GetLocalizedString(
		"Корректно удалять взятые предметы", 
		"Remove taken items correctly", 
		"Remove taken items correctly", 
		"Remove taken items correctly"
	);
};

instance MenuItem_PutInInvFix_Choice(C_MENU_ITEM_CHOICE)
{
	posy += 2 * DY;
	onchgsetoption = "PutInInvFix";
	
	text[0] = Str_GetLocalizedString(
		"нет|да|авто", 
		"no|yes|auto", 
		"Nein|Ja|Auto", 
		"nie|tak|auto"
	);
};

instance MenuItem_EventThrottling_Text(C_MENU_ITEM_TEXT)
{
	posy += 3 * DY;
	
	text[0] = Str_GetLocalizedString(
		"Дросселировать управление игроком", 
		"Throttle player control commands", 
		"Throttle player control commands", 
		"Throttle player control commands"
	);
};

instance MenuItem_EventThrottling_Choice(C_MENU_ITEM_CHOICE)
{
	posy += 3 * DY;
	onchgsetoption = "EventThrottling";
	
	text[0] = Str_GetLocalizedString(
		"нет|да|слабо", 
		"no|yes|weak", 
		"Nein|Ja|Weak", 
		"nie|tak|weak"
	);
};

instance MenuItem_EnableNpcFix_Text(C_MENU_ITEM_TEXT)
{
	posy += 4 * DY;
	
	text[0] = Str_GetLocalizedString(
		"Убрать вылет при вставке NPC", 
		"Eliminate crash on NPC insertion", 
		"Eliminate crash on NPC insertion", 
		"Eliminate crash on NPC insertion"
	);
};

instance MenuItem_EnableNpcFix_Choice(C_MENU_ITEM_CHOICE)
{
	posy += 4 * DY;
	onchgsetoption = "EnableNpcFix";
};

instance MenuItem_ClearCriticalStatesOnLoad_Text(C_MENU_ITEM_TEXT)
{
	posy += 5 * DY;
	
	text[0] = Str_GetLocalizedString(
		"Исправить загрузку в инвентаре/диалоге", 
		"Fix loading during dialog/inventory", 
		"Fix loading during dialog/inventory", 
		"Fix loading during dialog/inventory"
	);
};

instance MenuItem_ClearCriticalStatesOnLoad_Choice(C_MENU_ITEM_CHOICE)
{
	posy += 5 * DY;
	onchgsetoption = "ClearCriticalStatesOnLoad";
};

instance MenuItem_Back(C_MENU_ITEM_DEF)
{
	Union_MenuItem_Back();
};