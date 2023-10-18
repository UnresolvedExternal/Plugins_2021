#include <iomanip>

namespace NAMESPACE
{
	struct AniInfo
	{
		std::string name;
		int value;

		AniInfo(const std::string& name, int value) :
			name{ name },
			value{ value }
		{
			for (char& ch : this->name)
				switch (ch)
				{
				case '[':
					ch = '(';
					break;
				case ']':
					ch = ')';
					break;
				default:
					break;
				}
		}

		AniInfo(const AniInfo&) = default;
		AniInfo(AniInfo&&) = default;
		AniInfo& operator=(const AniInfo&) = default;
		AniInfo& operator=(AniInfo&&) = default;

		bool IsActive(oCNpc* npc) const
		{
			return npc->GetModel()->IsAniActive(npc->GetModel()->GetAniFromAniID(value));
		}

		bool IsStatic() const
		{
			return name.find('(') == std::string::npos;
		}

		std::string ToString(oCNpc* npc) const
		{
			zCModelAni* const ani = npc->GetModel()->GetAniFromAniID(value);
			zCModelAniActive* const activeAni = npc->GetModel()->GetActiveAni(ani);

			std::ostringstream out;
			out << (IsActive(npc) ? "+" : "-");
			out << name << " N=" << value;

			if (ani)
				out << " " << ani->aniName;

			if (activeAni)
				out << " " << std::fixed << std::setprecision(1) << activeAni->actFrame << " / " << ani->numFrames;

			return out.str();
		}
	};

	std::vector<AniInfo> GetAnis(oCNpc* npc)
	{
		oCAniCtrl_Human* const controller = npc->GetAnictrl();
		const int wmode = npc->fmode;

		std::vector<AniInfo> anis;
		anis.reserve(64u);

		anis += { "s_dead1", controller->s_dead1 };                               // sizeof 04h    offset 2C8h
		anis += { "s_dead2", controller->s_dead2 };                               // sizeof 04h    offset 2CCh
		anis += { "s_hang", controller->s_hang };                                // sizeof 04h    offset 2D0h
		anis += { "t_hang_2_stand", controller->t_hang_2_stand };                        // sizeof 04h    offset 2D4h
		anis += { "s_run[wmode]", controller->s_run[wmode] };                        // sizeof 28h    offset 2D8h
		anis += { "t_run_2_runl[wmode]", controller->t_run_2_runl[wmode] };                 // sizeof 28h    offset 300h
		anis += { "t_runl_2_run[wmode]", controller->t_runl_2_run[wmode] };                 // sizeof 28h    offset 328h
		anis += { "s_runl[wmode]", controller->s_runl[wmode] };                       // sizeof 28h    offset 350h
		anis += { "t_runl_2_runr[wmode]", controller->t_runl_2_runr[wmode] };                // sizeof 28h    offset 378h
		anis += { "t_runr_2_runl[wmode]", controller->t_runr_2_runl[wmode] };                // sizeof 28h    offset 3A0h
		anis += { "s_runr[wmode]", controller->s_runr[wmode] };                       // sizeof 28h    offset 3C8h
		anis += { "t_runr_2_run[wmode]", controller->t_runr_2_run[wmode] };                 // sizeof 28h    offset 3F0h
		anis += { "t_runturnl[wmode]", controller->t_runturnl[wmode] };                   // sizeof 28h    offset 418h
		anis += { "t_runturnr[wmode]", controller->t_runturnr[wmode] };                   // sizeof 28h    offset 440h
		anis += { "t_runstrafel[wmode]", controller->t_runstrafel[wmode] };                 // sizeof 28h    offset 468h
		anis += { "t_runstrafer[wmode]", controller->t_runstrafer[wmode] };                 // sizeof 28h    offset 490h
		anis += { "t_run_2_runbl[wmode]", controller->t_run_2_runbl[wmode] };                // sizeof 28h    offset 4B8h
		anis += { "t_runbl_2_run[wmode]", controller->t_runbl_2_run[wmode] };                // sizeof 28h    offset 4E0h
		anis += { "s_runbl[wmode]", controller->s_runbl[wmode] };                      // sizeof 28h    offset 508h
		anis += { "t_runbl_2_runbr[wmode]", controller->t_runbl_2_runbr[wmode] };              // sizeof 28h    offset 530h
		anis += { "t_runbr_2_runbl[wmode]", controller->t_runbr_2_runbl[wmode] };              // sizeof 28h    offset 558h
		anis += { "s_runbr[wmode]", controller->s_runbr[wmode] };                      // sizeof 28h    offset 580h
		anis += { "t_runbr_2_run[wmode]", controller->t_runbr_2_run[wmode] };                // sizeof 28h    offset 5A8h
		anis += { "t_jumpb[wmode]", controller->t_jumpb[wmode] };                      // sizeof 28h    offset 5D0h
		anis += { "t_run_2_walk[wmode]", controller->t_run_2_walk[wmode] };                 // sizeof 28h    offset 5F8h
		anis += { "t_walk_2_run[wmode]", controller->t_walk_2_run[wmode] };                 // sizeof 28h    offset 620h
		anis += { "t_run_2_sneak[wmode]", controller->t_run_2_sneak[wmode] };                // sizeof 28h    offset 648h
		anis += { "t_sneak_2_run[wmode]", controller->t_sneak_2_run[wmode] };                // sizeof 28h    offset 670h
		anis += { "s_walk[wmode]", controller->s_walk[wmode] };                       // sizeof 28h    offset 698h
		anis += { "t_walk_2_walkl[wmode]", controller->t_walk_2_walkl[wmode] };               // sizeof 28h    offset 6C0h
		anis += { "t_walkl_2_walk[wmode]", controller->t_walkl_2_walk[wmode] };               // sizeof 28h    offset 6E8h
		anis += { "s_walkl[wmode]", controller->s_walkl[wmode] };                      // sizeof 28h    offset 710h
		anis += { "t_walkl_2_walkr[wmode]", controller->t_walkl_2_walkr[wmode] };              // sizeof 28h    offset 738h
		anis += { "t_walkr_2_walkl[wmode]", controller->t_walkr_2_walkl[wmode] };              // sizeof 28h    offset 760h
		anis += { "s_walkr[wmode]", controller->s_walkr[wmode] };                      // sizeof 28h    offset 788h
		anis += { "t_walkr_2_walk[wmode]", controller->t_walkr_2_walk[wmode] };               // sizeof 28h    offset 7B0h
		anis += { "t_walkturnl[wmode]", controller->t_walkturnl[wmode] };                  // sizeof 28h    offset 7D8h
		anis += { "t_walkturnr[wmode]", controller->t_walkturnr[wmode] };                  // sizeof 28h    offset 800h
		anis += { "t_walkstrafel[wmode]", controller->t_walkstrafel[wmode] };                // sizeof 28h    offset 828h
		anis += { "t_walkstrafer[wmode]", controller->t_walkstrafer[wmode] };                // sizeof 28h    offset 850h
		anis += { "t_walk_2_walkbl[wmode]", controller->t_walk_2_walkbl[wmode] };              // sizeof 28h    offset 878h
		anis += { "t_walkbl_2_walk[wmode]", controller->t_walkbl_2_walk[wmode] };              // sizeof 28h    offset 8A0h
		anis += { "s_walkbl[wmode]", controller->s_walkbl[wmode] };                     // sizeof 28h    offset 8C8h
		anis += { "t_walkbl_2_walkbr[wmode]", controller->t_walkbl_2_walkbr[wmode] };            // sizeof 28h    offset 8F0h
		anis += { "t_walkbr_2_walkbl[wmode]", controller->t_walkbr_2_walkbl[wmode] };            // sizeof 28h    offset 918h
		anis += { "s_walkbr[wmode]", controller->s_walkbr[wmode] };                     // sizeof 28h    offset 940h
		anis += { "t_walkbr_2_walk[wmode]", controller->t_walkbr_2_walk[wmode] };              // sizeof 28h    offset 968h
		anis += { "t_runl_2_jump[wmode]", controller->t_runl_2_jump[wmode] };                // sizeof 28h    offset 990h
		anis += { "t_runr_2_jump[wmode]", controller->t_runr_2_jump[wmode] };                // sizeof 28h    offset 9B8h
		anis += { "t_jump_2_runl[wmode]", controller->t_jump_2_runl[wmode] };                // sizeof 28h    offset 9E0h
		anis += { "t_stand_2_jumpuplow", controller->t_stand_2_jumpuplow };                   // sizeof 04h    offset A08h
		anis += { "s_jumpuplow", controller->s_jumpuplow };                           // sizeof 04h    offset A0Ch
		anis += { "t_jumpuplow_2_stand", controller->t_jumpuplow_2_stand };                   // sizeof 04h    offset A10h
		anis += { "t_stand_2_jumpupmid", controller->t_stand_2_jumpupmid };                   // sizeof 04h    offset A14h
		anis += { "s_jumpupmid", controller->s_jumpupmid };                           // sizeof 04h    offset A18h
		anis += { "t_jumpupmid_2_stand", controller->t_jumpupmid_2_stand };                   // sizeof 04h    offset A1Ch
		anis += { "s_sneak[wmode]", controller->s_sneak[wmode] };                      // sizeof 28h    offset A20h
		anis += { "t_sneak_2_sneakl[wmode]", controller->t_sneak_2_sneakl[wmode] };             // sizeof 28h    offset A48h
		anis += { "t_sneakl_2_sneak[wmode]", controller->t_sneakl_2_sneak[wmode] };             // sizeof 28h    offset A70h
		anis += { "s_sneakl[wmode]", controller->s_sneakl[wmode] };                     // sizeof 28h    offset A98h
		anis += { "t_sneakl_2_sneakr[wmode]", controller->t_sneakl_2_sneakr[wmode] };            // sizeof 28h    offset AC0h
		anis += { "t_sneakr_2_sneakl[wmode]", controller->t_sneakr_2_sneakl[wmode] };            // sizeof 28h    offset AE8h
		anis += { "s_sneakr[wmode]", controller->s_sneakr[wmode] };                     // sizeof 28h    offset B10h
		anis += { "t_sneakr_2_sneak[wmode]", controller->t_sneakr_2_sneak[wmode] };             // sizeof 28h    offset B38h
		anis += { "t_sneakturnl[wmode]", controller->t_sneakturnl[wmode] };                 // sizeof 28h    offset B60h
		anis += { "t_sneakturnr[wmode]", controller->t_sneakturnr[wmode] };                 // sizeof 28h    offset B88h
		anis += { "t_sneakstrafel[wmode]", controller->t_sneakstrafel[wmode] };               // sizeof 28h    offset BB0h
		anis += { "t_sneakstrafer[wmode]", controller->t_sneakstrafer[wmode] };               // sizeof 28h    offset BD8h
		anis += { "t_sneak_2_sneakbl[wmode]", controller->t_sneak_2_sneakbl[wmode] };            // sizeof 28h    offset C00h
		anis += { "t_sneakbl_2_sneak[wmode]", controller->t_sneakbl_2_sneak[wmode] };            // sizeof 28h    offset C28h
		anis += { "s_sneakbl[wmode]", controller->s_sneakbl[wmode] };                    // sizeof 28h    offset C50h
		anis += { "t_sneakbl_2_sneakbr[wmode]", controller->t_sneakbl_2_sneakbr[wmode] };          // sizeof 28h    offset C78h
		anis += { "t_sneakbr_2_sneakbl[wmode]", controller->t_sneakbr_2_sneakbl[wmode] };          // sizeof 28h    offset CA0h
		anis += { "s_sneakbr[wmode]", controller->s_sneakbr[wmode] };                    // sizeof 28h    offset CC8h
		anis += { "t_sneakbr_2_sneak[wmode]", controller->t_sneakbr_2_sneak[wmode] };            // sizeof 28h    offset CF0h
		anis += { "t_walkl_2_aim[wmode]", controller->t_walkl_2_aim[wmode] };                // sizeof 28h    offset D18h
		anis += { "t_walkr_2_aim[wmode]", controller->t_walkr_2_aim[wmode] };                // sizeof 28h    offset D40h
		anis += { "t_walk_2_aim[wmode]", controller->t_walk_2_aim[wmode] };                 // sizeof 28h    offset D68h
		anis += { "s_aim[wmode]", controller->s_aim[wmode] };                        // sizeof 28h    offset D90h
		anis += { "t_aim_2_walk[wmode]", controller->t_aim_2_walk[wmode] };                 // sizeof 28h    offset DB8h
		anis += { "t_hitl[wmode]", controller->t_hitl[wmode] };                       // sizeof 28h    offset DE0h
		anis += { "t_hitr[wmode]", controller->t_hitr[wmode] };                       // sizeof 28h    offset E08h
		anis += { "t_hitback[wmode]", controller->t_hitback[wmode] };                    // sizeof 28h    offset E30h
		anis += { "t_hitf[wmode]", controller->t_hitf[wmode] };                       // sizeof 28h    offset E58h
		anis += { "s_hitf[wmode]", controller->s_hitf[wmode] };                       // sizeof 28h    offset E80h
		anis += { "t_aim_2_defend[wmode]", controller->t_aim_2_defend[wmode] };               // sizeof 28h    offset EA8h
		anis += { "s_defend[wmode]", controller->s_defend[wmode] };                     // sizeof 28h    offset ED0h
		anis += { "t_defend_2_aim[wmode]", controller->t_defend_2_aim[wmode] };               // sizeof 28h    offset EF8h
		anis += { "t_paradeL[wmode]", controller->t_paradeL[wmode] };                    // sizeof 28h    offset F20h
		anis += { "t_paradeM[wmode]", controller->t_paradeM[wmode] };                    // sizeof 28h    offset F48h
		anis += { "t_paradeS[wmode]", controller->t_paradeS[wmode] };                    // sizeof 28h    offset F70h
		anis += { "t_hitfrun[wmode]", controller->t_hitfrun[wmode] };                    // sizeof 28h    offset F98h
		anis += { "t_stumble", controller->t_stumble };                             // sizeof 04h    offset FC0h
		anis += { "t_stumbleb", controller->t_stumbleb };                            // sizeof 04h    offset FC4h
		anis += { "t_fallen_2_stand", controller->t_fallen_2_stand };                      // sizeof 04h    offset FC8h
		anis += { "t_fallenb_2_stand", controller->t_fallenb_2_stand };                     // sizeof 04h    offset FCCh
		anis += { "t_walk_2_walkwl", controller->t_walk_2_walkwl };                       // sizeof 04h    offset FD0h
		anis += { "t_walkwl_2_walk", controller->t_walkwl_2_walk };                       // sizeof 04h    offset FD4h
		anis += { "s_walkwl", controller->s_walkwl };                              // sizeof 04h    offset FD8h
		anis += { "t_walkwl_2_walkwr", controller->t_walkwl_2_walkwr };                     // sizeof 04h    offset FDCh
		anis += { "t_walkwr_2_walkwl", controller->t_walkwr_2_walkwl };                     // sizeof 04h    offset FE0h
		anis += { "s_walkwr", controller->s_walkwr };                              // sizeof 04h    offset FE4h
		anis += { "t_walkwr_2_walk", controller->t_walkwr_2_walk };                       // sizeof 04h    offset FE8h
		anis += { "t_walk_2_walkwbl", controller->t_walk_2_walkwbl };                      // sizeof 04h    offset FECh
		anis += { "t_walkwbl_2_walk", controller->t_walkwbl_2_walk };                      // sizeof 04h    offset FF0h
		anis += { "s_walkwbl", controller->s_walkwbl };                             // sizeof 04h    offset FF4h
		anis += { "t_walkwbl_2_walkwbr", controller->t_walkwbl_2_walkwbr };                   // sizeof 04h    offset FF8h
		anis += { "t_walkwbr_2_walkwbl", controller->t_walkwbr_2_walkwbl };                   // sizeof 04h    offset FFCh
		anis += { "s_walkwbr", controller->s_walkwbr };                             // sizeof 04h    offset 1000h
		anis += { "t_walkwbr_2_walk", controller->t_walkwbr_2_walk };                      // sizeof 04h    offset 1004h
		anis += { "_s_walk", controller->_s_walk };                               // sizeof 04h    offset 1008h
		anis += { "_t_walk_2_walkl", controller->_t_walk_2_walkl };                       // sizeof 04h    offset 100Ch
		anis += { "_t_walkl_2_walk", controller->_t_walkl_2_walk };                       // sizeof 04h    offset 1010h
		anis += { "_s_walkl", controller->_s_walkl };                              // sizeof 04h    offset 1014h
		anis += { "_t_walkl_2_walkr", controller->_t_walkl_2_walkr };                      // sizeof 04h    offset 1018h
		anis += { "_t_walkr_2_walkl", controller->_t_walkr_2_walkl };                      // sizeof 04h    offset 101Ch
		anis += { "_s_walkr", controller->_s_walkr };                              // sizeof 04h    offset 1020h
		anis += { "_t_walkr_2_walk", controller->_t_walkr_2_walk };                       // sizeof 04h    offset 1024h
		anis += { "_t_turnl", controller->_t_turnl };                              // sizeof 04h    offset 1028h
		anis += { "_t_turnr", controller->_t_turnr };                              // sizeof 04h    offset 102Ch
		anis += { "_t_strafel", controller->_t_strafel };                            // sizeof 04h    offset 1030h
		anis += { "_t_strafer", controller->_t_strafer };                            // sizeof 04h    offset 1034h
		anis += { "_t_walk_2_walkbl", controller->_t_walk_2_walkbl };                      // sizeof 04h    offset 1038h
		anis += { "_t_walkbl_2_walk", controller->_t_walkbl_2_walk };                      // sizeof 04h    offset 103Ch
		anis += { "_s_walkbl", controller->_s_walkbl };                             // sizeof 04h    offset 1040h
		anis += { "_t_walkbl_2_walkbr", controller->_t_walkbl_2_walkbr };                    // sizeof 04h    offset 1044h
		anis += { "_t_walkbr_2_walkbl", controller->_t_walkbr_2_walkbl };                    // sizeof 04h    offset 1048h
		anis += { "_s_walkbr", controller->_s_walkbr };                             // sizeof 04h    offset 104Ch
		anis += { "_t_walkbr_2_walk", controller->_t_walkbr_2_walk };                      // sizeof 04h    offset 1050h
		anis += { "s_jumpstand", controller->s_jumpstand };                           // sizeof 04h    offset 1054h
		anis += { "t_stand_2_jumpstand", controller->t_stand_2_jumpstand };                   // sizeof 04h    offset 1058h
		anis += { "t_jumpstand_2_stand", controller->t_jumpstand_2_stand };                   // sizeof 04h    offset 105Ch
		anis += { "_t_jumpb", controller->_t_jumpb };                              // sizeof 04h    offset 1060h
		anis += { "_t_stand_2_jump", controller->_t_stand_2_jump };                       // sizeof 04h    offset 1064h
		anis += { "_s_jump", controller->_s_jump };                               // sizeof 04h    offset 1068h
		anis += { "t_jump_2_stand", controller->t_jump_2_stand };                        // sizeof 04h    offset 106Ch
		anis += { "_t_stand_2_jumpup", controller->_t_stand_2_jumpup };                     // sizeof 04h    offset 1070h
		anis += { "_s_jumpup", controller->_s_jumpup };                             // sizeof 04h    offset 1074h
		anis += { "_t_jumpup_2_falldn", controller->_t_jumpup_2_falldn };                    // sizeof 04h    offset 1078h
		anis += { "_t_jump_2_falldn", controller->_t_jump_2_falldn };                      // sizeof 04h    offset 107Ch
		anis += { "t_walkwl_2_swimf", controller->t_walkwl_2_swimf };                      // sizeof 04h    offset 1080h
		anis += { "s_swimf", controller->s_swimf };                               // sizeof 04h    offset 1084h
		anis += { "t_swimf_2_walkwl", controller->t_swimf_2_walkwl };                      // sizeof 04h    offset 1088h
		anis += { "t_walkwbl_2_swimb", controller->t_walkwbl_2_swimb };                     // sizeof 04h    offset 108Ch
		anis += { "s_swimb", controller->s_swimb };                               // sizeof 04h    offset 1090h
		anis += { "t_swimb_2_walkwbl", controller->t_swimb_2_walkwbl };                     // sizeof 04h    offset 1094h
		anis += { "t_swimf_2_swim", controller->t_swimf_2_swim };                        // sizeof 04h    offset 1098h
		anis += { "s_swim", controller->s_swim };                                // sizeof 04h    offset 109Ch
		anis += { "t_swim_2_swimf", controller->t_swim_2_swimf };                        // sizeof 04h    offset 10A0h
		anis += { "t_swim_2_swimb", controller->t_swim_2_swimb };                        // sizeof 04h    offset 10A4h
		anis += { "t_swimb_2_swim", controller->t_swimb_2_swim };                        // sizeof 04h    offset 10A8h
		anis += { "t_warn", controller->t_warn };                                // sizeof 04h    offset 10ACh
		anis += { "t_swim_2_dive", controller->t_swim_2_dive };                         // sizeof 04h    offset 10B0h
		anis += { "s_dive", controller->s_dive };                                // sizeof 04h    offset 10B4h
		anis += { "t_divef_2_swim", controller->t_divef_2_swim };                        // sizeof 04h    offset 10B8h
		anis += { "t_dive_2_divef", controller->t_dive_2_divef };                        // sizeof 04h    offset 10BCh
		anis += { "s_divef", controller->s_divef };                               // sizeof 04h    offset 10C0h
		anis += { "t_divef_2_dive", controller->t_divef_2_dive };                        // sizeof 04h    offset 10C4h
		anis += { "t_dive_2_drowned", controller->t_dive_2_drowned };                      // sizeof 04h    offset 10C8h
		anis += { "s_drowned", controller->s_drowned };                             // sizeof 04h    offset 10CCh
		anis += { "t_swimturnl", controller->t_swimturnl };                           // sizeof 04h    offset 10D0h
		anis += { "t_swimturnr", controller->t_swimturnr };                           // sizeof 04h    offset 10D4h
		anis += { "t_diveturnl", controller->t_diveturnl };                           // sizeof 04h    offset 10D8h
		anis += { "t_diveturnr", controller->t_diveturnr };                           // sizeof 04h    offset 10DCh
		anis += { "_t_walkl_2_aim", controller->_t_walkl_2_aim };                        // sizeof 04h    offset 10E0h
		anis += { "_t_walkr_2_aim", controller->_t_walkr_2_aim };                        // sizeof 04h    offset 10E4h
		anis += { "_t_walk_2_aim", controller->_t_walk_2_aim };                         // sizeof 04h    offset 10E8h
		anis += { "_s_aim", controller->_s_aim };                                // sizeof 04h    offset 10ECh
		anis += { "_t_aim_2_walk", controller->_t_aim_2_walk };                         // sizeof 04h    offset 10F0h
		anis += { "_t_hitl", controller->_t_hitl };                               // sizeof 04h    offset 10F4h
		anis += { "_t_hitr", controller->_t_hitr };                               // sizeof 04h    offset 10F8h
		anis += { "_t_hitback", controller->_t_hitback };                            // sizeof 04h    offset 10FCh
		anis += { "_t_hitf", controller->_t_hitf };                               // sizeof 04h    offset 1100h
		anis += { "_t_hitfstep", controller->_t_hitfstep };                           // sizeof 04h    offset 1104h
		anis += { "_s_hitf", controller->_s_hitf };                               // sizeof 04h    offset 1108h
		anis += { "_t_aim_2_defend", controller->_t_aim_2_defend };                       // sizeof 04h    offset 110Ch
		anis += { "_s_defend", controller->_s_defend };                             // sizeof 04h    offset 1110h
		anis += { "_t_defend_2_aim", controller->_t_defend_2_aim };                       // sizeof 04h    offset 1114h
		anis += { "_t_paradeL", controller->_t_paradeL };                            // sizeof 04h    offset 1118h
		anis += { "_t_paradeM", controller->_t_paradeM };                            // sizeof 04h    offset 111Ch
		anis += { "_t_paradeS", controller->_t_paradeS };                            // sizeof 04h    offset 1120h
		anis += { "_t_hitfrun", controller->_t_hitfrun };                            // sizeof 04h    offset 1124h
		anis += { "t_stand_2_iaim", controller->t_stand_2_iaim };                        // sizeof 04h    offset 1128h
		anis += { "s_iaim", controller->s_iaim };                                // sizeof 04h    offset 112Ch
		anis += { "t_iaim_2_stand", controller->t_iaim_2_stand };                        // sizeof 04h    offset 1130h
		anis += { "t_iaim_2_idrop", controller->t_iaim_2_idrop };                        // sizeof 04h    offset 1134h
		anis += { "s_idrop", controller->s_idrop };                               // sizeof 04h    offset 1138h
		anis += { "t_idrop_2_stand", controller->t_idrop_2_stand };                       // sizeof 04h    offset 113Ch
		anis += { "t_iaim_2_ithrow", controller->t_iaim_2_ithrow };                       // sizeof 04h    offset 1140h
		anis += { "s_ithrow", controller->s_ithrow };                              // sizeof 04h    offset 1144h
		anis += { "t_ithrow_2_stand", controller->t_ithrow_2_stand };                      // sizeof 04h    offset 1148h
		anis += { "t_stand_2_iget", controller->t_stand_2_iget };                        // sizeof 04h    offset 114Ch
		anis += { "s_iget", controller->s_iget };                                // sizeof 04h    offset 1150h
		anis += { "t_iget_2_stand", controller->t_iget_2_stand };                        // sizeof 04h    offset 1154h
		anis += { "s_oget", controller->s_oget };                                // sizeof 04h    offset 1158h
		anis += { "_t_stand_2_torch", controller->_t_stand_2_torch };                      // sizeof 04h    offset 115Ch
		anis += { "_s_torch", controller->_s_torch };                              // sizeof 04h    offset 1160h
		anis += { "_t_torch_2_stand", controller->_t_torch_2_stand };                      // sizeof 04h    offset 1164h
		anis += { "hitani", controller->hitani };                                // sizeof 04h    offset 1168h
		anis += { "help", controller->help };                                  // sizeof 04h    offset 116Ch
		anis += { "help1", controller->help1 };                                 // sizeof 04h    offset 1170h
		anis += { "help2", controller->help2 };                                 // sizeof 04h    offset 1174h
		anis += { "s_fall", controller->s_fall };                                // sizeof 04h    offset 1178h
		anis += { "s_fallb", controller->s_fallb };                               // sizeof 04h    offset 117Ch
		anis += { "s_fallen", controller->s_fallen };                              // sizeof 04h    offset 1180h
		anis += { "s_fallenb", controller->s_fallenb };                             // sizeof 04h    offset 1184h
		anis += { "s_falldn", controller->s_falldn };                              // sizeof 04h    offset 1188h
		anis += { "_t_runl_2_jump", controller->_t_runl_2_jump };                        // sizeof 04h    offset 118Ch
		anis += { "_t_runr_2_jump", controller->_t_runr_2_jump };                        // sizeof 04h    offset 1190h
		anis += { "_t_jump_2_runl", controller->_t_jump_2_runl };                        // sizeof 04h    offset 1194h
		anis += { "s_look", controller->s_look };                                // sizeof 04h    offset 1198h
		anis += { "s_point", controller->s_point };                               // sizeof 04h    offset 119Ch
		anis += { "dummy1", controller->dummy1 };                                // sizeof 04h    offset 11A0h
		anis += { "dummy2", controller->dummy2 };                                // sizeof 04h    offset 11A4h
		anis += { "dummy3", controller->dummy3 };                                // sizeof 04h    offset 11A8h
		anis += { "dummy4", controller->dummy4 };                                // sizeof 04h    offset 11ACh
		anis += { "s_weapon[wmode]", controller->s_weapon[wmode] };                     // sizeof 28h    offset 11B0h
		anis += { "togglewalk", controller->togglewalk };                            // sizeof 04h    offset 11D8h
		anis += { "t_stand_2_cast", controller->t_stand_2_cast };                        // sizeof 04h    offset 11DCh
		anis += { "s_cast", controller->s_cast };                                // sizeof 04h    offset 11E0h
		anis += { "t_cast_2_shoot", controller->t_cast_2_shoot };                        // sizeof 04h    offset 11E4h
		anis += { "t_cast_2_stand", controller->t_cast_2_stand };                        // sizeof 04h    offset 11E8h
		anis += { "s_shoot", controller->s_shoot };                               // sizeof 04h    offset 11ECh
		anis += { "t_shoot_2_stand", controller->t_shoot_2_stand };                       // sizeof 04h    offset 11F0

		return anis;
	}

	void DrawTable(const std::vector<std::vector<std::string>>& table, int startX, int startY)
	{
		if (table.empty())
			return;

		static std::vector<int> sizes;
		sizes.resize(table.front().size());

		for (size_t x = 0; x < sizes.size(); x++)
			for (size_t y = 0; y < table.size(); y++)
			{
				zSTRING text{ table[y][x].c_str() };
				sizes[x] = std::max(sizes[x], screen->FontSize(text) + 100);
			}

		for (size_t x = 0; x < sizes.size(); x++)
		{
			for (size_t y = 0; y < table.size(); y++)
				screen->Print(startX, startY + y * screen->FontY(), table[y][x].c_str());

			startX += sizes[x];
		}
	}

	std::string GetType(zCModelAni* ani)
	{
		std::unordered_map<int, std::string> names;

		names[zMDL_ANI_TYPE_NORMAL] = "NORMAL";
		names[zMDL_ANI_TYPE_BLEND] = "BLEND";
		names[zMDL_ANI_TYPE_SYNC] = "SYNC";
		names[zMDL_ANI_TYPE_ALIAS] = "ALIAS";
		names[zMDL_ANI_TYPE_BATCH] = "BATCH";
		names[zMDL_ANI_TYPE_COMB] = "COMB";
		names[zMDL_ANI_TYPE_DISABLED] = "DISABLED";

		return names[ani->aniType];
	}

	Sub showAnis(ZSUB(GameEvent::Loop), []
		{
			if (!player)
				return;

			std::vector<AniInfo> anis = GetAnis(player);

			std::vector<std::vector<std::string>> table;
			auto& row = table.emplace_back();
			row.emplace_back("L");
			row.emplace_back("ID");
			row.emplace_back("Field");
			row.emplace_back("AniName");
			row.emplace_back("Frame");
			row.emplace_back("BlendIn");
			row.emplace_back("BlendOut");
			row.emplace_back("Type");
			row.emplace_back("NextAni");
			row.emplace_back("Proto");

			std::unordered_set<zCModelAniActive*> visitedAnis;

			for (int wmode = 0; wmode <= 0; wmode++)
			{
				for (const AniInfo& info : anis)
					if (info.IsActive(player) && info.IsStatic() != static_cast<bool>(wmode))
					{
						zCModelAni* const ani = player->GetModel()->GetAniFromAniID(info.value);
						zCModelAniActive* const activeAni = player->GetModel()->GetActiveAni(ani);
						visitedAnis.insert(activeAni);

						auto& row = table.emplace_back();
						row.emplace_back(A ani->layer);
						row.emplace_back(A info.value);
						row.emplace_back(info.name);
						row.emplace_back(A ani->aniName);

						if (activeAni->isFadingOut)
							row.back() += " (F)";

						row.emplace_back((
							std::ostringstream{} <<
							std::fixed << std::setw(4) << std::setprecision(1) << activeAni->actFrame <<
							" / " << ani->numFrames
							).str()
						);

						std::ostringstream out;
						out << std::setprecision(2) << activeAni->blendInOverride << " / " << (ani ? ani->blendInSpeed : 0.0f);
						row.emplace_back(out.str());

						out = {};
						out << std::setprecision(2) << activeAni->blendOutOverride << " / " << (ani ? ani->blendOutSpeed : 0.0f);
						row.emplace_back(out.str());

						row.emplace_back(GetType(ani));

						if (activeAni->nextAniOverride)
							row.emplace_back(activeAni->nextAniOverride->aniName);
						else
							row.emplace_back(activeAni->nextAni ? activeAni->nextAni->aniName : "");

						row.emplace_back(AHEX32(ani).GetVector());
					}
			}

			for (int i = 0; i < player->GetModel()->numActiveAnis; i++)
				if (zCModelAniActive* activeAni = &player->GetModel()->activeAniList[i])
					if (visitedAnis.find(activeAni) == visitedAnis.end())
					{
						zCModelAni* ani = activeAni->protoAni;

						if (!ani)
							continue;

						auto& row = table.emplace_back();
						row.emplace_back(ani ? A ani->layer : A"");
						row.emplace_back(ani ? A ani->aniID : A"");
						row.emplace_back("");
						row.emplace_back(ani ? A ani->aniName : A"");

						if (activeAni->isFadingOut)
							row.back() += " (F)";

						row.emplace_back((
							std::ostringstream{} <<
							std::fixed << std::setw(4) << std::setprecision(1) << activeAni->actFrame <<
							" / " << (ani ? ani->numFrames : -1)
							).str()
						);

						std::ostringstream out;
						out << std::setprecision(2) << activeAni->blendInOverride << " / " << (ani ? ani->blendInSpeed : 0.0f);
						row.emplace_back(out.str());

						out = {};
						out << std::setprecision(2) << activeAni->blendOutOverride << " / " << (ani ? ani->blendOutSpeed : 0.0f);
						row.emplace_back(out.str());

						row.emplace_back(GetType(ani));

						if (activeAni->nextAniOverride)
							row.emplace_back(activeAni->nextAniOverride->aniName);
						else
							row.emplace_back(activeAni->nextAni ? activeAni->nextAni->aniName : "");

						row.emplace_back(AHEX32(ani).GetVector());
					}

			DrawTable(table, 64, 4000);
		});
}
