namespace NAMESPACE
{
	zSTRING GetFocusText(oCItem* item)
	{
		zSTRING text;

		if (Options::Cats.find(item->mainflag) != Options::Cats.end())
			text += item->description;
		else
			text += item->name;

		if (Options::AppendAmountInfo && item->amount != 1)
		{
			text += " (";
			text += Options::XChar->GetVector();
			text += item->amount;
			text += ")";
		}

		return text;
	}

	void __fastcall Hook_oCGame_UpdatePlayerStatus(oCGame*, void*);
	Hook<void(__thiscall*)(oCGame*), ActiveValue<bool>> Ivk_oCGame_UpdatePlayerStatus(ZENFOR(0x00638F90, 0x0065F4E0, 0x00666640, 0x006C3140), &Hook_oCGame_UpdatePlayerStatus, HookMode::Patch, Options::HookUpdatePlayerStatus);
	void __fastcall Hook_oCGame_UpdatePlayerStatus(oCGame* _this, void* vtable)
	{
#define RET return Ivk_oCGame_UpdatePlayerStatus(_this)

		zCVob* vob = COA(player, focus_vob);

		if (!vob || !COA(ogame, showPlayerStatus))
			RET;

		oCItem* item = vob->CastTo<oCItem>();
		oCMOB* mob = vob->CastTo<oCMOB>();

		if (!item && !mob)
			RET;

		if (vob->visual)
		{
			VarScope<zSTRING> scope;

			if (item)
				scope = AssignTemp(item->name, GetFocusText(item));

			VarScope<zMAT4> posScope;

			if (!Options::CorrectModelFocusNamePos)
				RET;

			if (zCModel* model = vob->visual->CastTo<zCModel>())
			{
				zVEC3 offset = { model->bbox3DLocalFixed.GetCenter()[VX], 0, model->bbox3DLocalFixed.GetCenter()[VZ] };
				offset = vob->trafoObjToWorld.Rotate(offset);
				const zVEC3 newPos = vob->GetPositionWorld() + offset;

				zMAT4 trafo = vob->trafoObjToWorld;
				trafo.SetTranslation(newPos);

				posScope = AssignTemp(vob->trafoObjToWorld, trafo);
			}

			RET;
		}

		if (!Options::PrintZenFocus)
			RET;

		if (zCVob* innerVob = COA(vob, globalVobTreeNode, GetFirstChild(), GetData()))
		{
			zVEC3 textPos = innerVob->bbox3D.GetCenter();
			textPos[VY] += (innerVob->bbox3D.maxs[VY] - innerVob->bbox3D.mins[VY]) * 0.82F;
			Print(screen, textPos, mob ? mob->GetName() : GetFocusText(item));
		}
		else
			Print(screen, vob->GetPositionWorld(), mob ? mob->GetName() : GetFocusText(item));

		RET;

#undef RET
	}
}
