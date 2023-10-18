// Supported with union (c) 2020 Union team

// User API for zCModel
// Add your methods here

void StartAni_Union(zCModelAni*, int);

inline int IsAniActive(int aniID)
{
	zCModelAni* ani = GetAniFromAniID(aniID);

	if (!ani)
		return false;

	zCModelAniActive* activeAni = GetActiveAni(ani);

	if (!activeAni)
		return false;

	return !activeAni->isFadingOut;
}

inline void StartAni(int aniID)
{
	zCModelAni* ani = GetAniFromAniID(aniID);
	StartAni(ani, zMDL_STARTANI_DEFAULT);
}

inline int IsStateActive(int aniID)
{
	return IsStateActive(GetAniFromAniID(aniID));
}

inline zCModelAni* GetNextAni(zCModelAni* modelAni)
{
	if (!modelAni->nextAni)
	{
		// tatsaechlich keine NextAni angegeben?
		if (modelAni->GetAniName().Length() <= 0) return 0;

		// Diese Ani hat in diesem ModelProto keine nextAni. Falls alerdings im .MDS explizit eine NextAni
		// angegeben wurde und dieses ModelProto als Overlay benutzt wurde, kann/sollte es sein, dass die nextAni 
		// ueber ein tiefer liegendes ModelProto aufgeloest werden kann.
		// In diesem Fall wird die im baseModelProto eingetragene nextAni genommen. Diese kann theoretisch von der im 
		// Overlay spezifizierten abweichen, sollte es aber nicht.
		const int aniID = modelAni->aniID;
		modelAni = modelProtoList[0]->protoAnis[aniID];
		if (!modelAni->nextAni)	return 0;
	};

	return GetAniFromAniID(modelAni->nextAni->aniID);
}