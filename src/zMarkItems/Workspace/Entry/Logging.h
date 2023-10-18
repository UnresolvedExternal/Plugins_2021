namespace NAMESPACE
{
	void LogItem(oCItem* item)
	{
		LOG(AHEX32(item));
		
		if (!item)
			return;

		LOG(item->refCtr);

		if (item->refCtr <= 0)
			return;

		LOG(item->instanz);
		LOG(item->objectName);
		LOG(COA(item, homeWorld, objectName));

		if (item->homeWorld)
		{
			zVEC3 pos = item->GetPositionWorld();
			cmd << "pos: " << pos[0] << "/" << pos[1] << "/" << pos[2] << endl;
		}
	}
}