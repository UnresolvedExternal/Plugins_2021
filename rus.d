
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
		Doc_PrintLine(nDocID,0,"»спытание ќгнем");
		Doc_PrintLine(nDocID,0,"");
		Doc_PrintLines(nDocID,0,"’от€ послушник может чувствовать себ€ готовым пройти »спытание ћагией, из этого не следует, что он об€зательно будет выбран. ≈сли, однако, он прин€л это решение после зрелого размышлени€ и если он настаивает на своем решении, он наделен правом требовать прохождени€ этого »спытани€, и ни один маг не может отказать ему в этом. Ќо послушник об€зан не только пройти »спытание ћагией, но также найти просвещение через огонь.");
