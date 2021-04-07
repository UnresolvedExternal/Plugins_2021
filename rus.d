
func void Use_Bookstand_01_S1()
{
	var int nDocID;
	if(C_NpcIsHero(self))
	{
		nDocID = Doc_Create();
		Doc_SetPages(nDocID,2);
		Doc_SetPage(nDocID,0,"Book_Mage_L.tga",0);
		Doc_SetPage(nDocID,1,"Book_Mage_R.tga",0);
		Doc_SetFont(nDocID,-1,FONT_Book);
		Doc_SetMargins(nDocID,0,275,20,30,20,1);
		Doc_PrintLine(nDocID,0,"Испытание Огнем");
		Doc_PrintLine(nDocID,0,"");
		Doc_PrintLines(nDocID,0,"Хотя послушник может чувствовать себя готовым пройти Испытание Магией, из этого не следует, что он обязательно будет выбран. Если, однако, он принял это решение после зрелого размышления и если он настаивает на своем решении, он наделен правом требовать прохождения этого Испытания, и ни один маг не может отказать ему в этом. Но послушник обязан не только пройти Испытание Магией, но также найти просвещение через огонь.");
		Doc_SetMargins(nDocID,-1,30,20,275,20,1);
		Doc_PrintLine(nDocID,1,"");
		Doc_PrintLine(nDocID,1,"");
		Doc_PrintLines(nDocID,1,"Это Испытание должно позволять дать оценку мудрости, силе и ловкости послушника. Следовательно, он должен пройти три испытания, каждое из которых дается ему одним из магов Высшего Совета, прежде чем ему будет позволено принять Клятву Огня и присоединиться к Соглашению Огня.");
		Doc_PrintLine(nDocID,1,"");
		Doc_PrintLine(nDocID,1,"");
		Doc_PrintLines(nDocID,1,"Такова воля Инноса и так тому и быть.");
		Doc_PrintLine(nDocID,1,"");
		Doc_PrintLine(nDocID,1,"");
		Doc_PrintLine(nDocID,1,"");
		Doc_PrintLines(nDocID,1,"Высший Cовет");
		Doc_Show(nDocID);
		if((hero.guild == GIL_NOV) && (KNOWS_FIRE_CONTEST == FALSE))
		{
			KNOWS_FIRE_CONTEST = TRUE;
			Log_CreateTopic(TOPIC_FireContest,LOG_MISSION);
			Log_SetTopicStatus(TOPIC_FireContest,LOG_Running);
			B_LogEntry(TOPIC_FireContest,"Будучи послушником, я имею право требовать прохождения Испытания Огнем. Каждый из трех магов Высшего Совета должен дать мне задание. Если я пройду эти испытания, я буду принят в Круг Огня.");
		};
	};
};


var int FinalDragonEquipment_Once;


