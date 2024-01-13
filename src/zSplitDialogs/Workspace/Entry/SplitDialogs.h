#include <filesystem>
#include <fstream>
#include <unordered_map>

namespace NAMESPACE
{
	class Output
	{
	private:
		std::ofstream warn;
		std::ofstream svm;
		std::unordered_map<int, std::unique_ptr<std::ofstream>> npcs;
		const std::filesystem::path path;

		Output() :
			path{ (zoptions->GetDirString(DIR_ROOT) + "\\Dialogs\\").ToChar() }
		{
			namespace fs = std::filesystem;

			cmd << "Clearing the directory started: " << path.c_str() << endl;

			std::error_code code{};
			fs::remove_all(path, code);

			cmd << "Remove directory: " << code.message() << endl;

			do
			{
				fs::create_directory(path, code);
				cmd << "Create directory: " << code.message() << endl;
			} while (!fs::exists(path, code));

			cmd << "Directory created" << endl;
			warn.open((path / "_warn.txt").string(), std::ios::out);
			svm.open((path / "_svm.txt").string(), std::ios::out);
		}

	public:
		static Output& GetInstance()
		{
			static Output output;
			return output;
		}

		std::ostream& Warn()
		{
			return warn;
		}

		std::ostream& Svm()
		{
			return svm;
		}

		std::ostream& Npc(int index)
		{
			if (npcs.size() > 50u)
				npcs.clear();

			auto it = npcs.find(index);

			if (it != npcs.end())
				return *it->second;

			string name = "_HERO.TXT";

			if (index == -1)
				name = "_AMBIENT.TXT";
			else
				if (Symbol sym{ parser, index })
					name = A(sym.GetSymbol()->name + Z".TXT");

			std::string filePath = (path / name.GetVector()).string();

			auto file = std::make_unique<std::ofstream>();
			file->open(filePath, std::ios::in | std::ios::ate);

			if (file->fail())
			{
				file->close();
				file->open(filePath);
			}
			
			return *npcs.insert({ index, std::move(file) }).first->second;
		}
	};

#define WARN() Output::GetInstance().Warn()
#define NPC(index) Output::GetInstance().Npc(index)
#define SVM() Output::GetInstance().Svm()

	enum class InstructionType
	{
		PushInt = zPAR_TOK_PUSHINT,
		PushFloat = zPAR_TOK_FLOAT,
		PushVar = zPAR_TOK_PUSHVAR,
		PushArrayVar = zPAR_TOK_PUSHVAR + zPAR_TOK_FLAGARRAY,
		PushInst = zPAR_TOK_PUSHINST,

		AssignString = zPAR_TOK_ASSIGNSTR,
		AssignStringPtr = zPAR_TOK_ASSIGNSTRP,
		AssignFunc = zPAR_TOK_ASSIGNFUNC,
		AssignFloat = zPAR_TOK_ASSIGNFLOAT,
		AssignInt = zPAR_OP_IS,
		AssignPlus = zPAR_OP_ISPLUS,
		AssignMinus = zPAR_OP_ISMINUS,
		AssignMul = zPAR_OP_ISMUL,
		AssignDiv = zPAR_OP_ISDIV,
		AssignInst = zPAR_TOK_ASSIGNINST,

		Plus = zPAR_OP_PLUS,
		Minus = zPAR_OP_MINUS,
		Mul = zPAR_OP_MUL,
		Div = zPAR_OP_DIV,
		Mod = zPAR_OP_MOD,
		ShiftLeft = zPAR_OP_SHIFTL,
		ShiftRight = zPAR_OP_SHIFTR,

		BitOr = zPAR_OP_OR,
		BitAnd = zPAR_OP_AND,
		BitNot = zPAR_OP_UN_NEG,

		Less = zPAR_OP_LOWER,
		More = zPAR_OP_HIGHER,
		LessEqual = zPAR_OP_LOWER_EQ,
		Equal = zPAR_OP_EQUAL,
		NotEqual = zPAR_OP_NOTEQUAL,
		MoreEqual = zPAR_OP_HIGHER_EQ,

		LogOr = zPAR_OP_LOG_OR,
		LogAnd = zPAR_OP_LOG_AND,
		LogNot = zPAR_OP_UN_NOT,

		UnaryMinus = zPAR_OP_UN_MINUS,
		UnaryPlus = zPAR_OP_UN_PLUS,

		Call = zPAR_TOK_CALL,
		CallExternal = zPAR_TOK_CALLEXTERN,

		Return = zPAR_TOK_RET,
		Jump = zPAR_TOK_JUMP,
		JumpFalse = zPAR_TOK_JUMPF,

		SetInstance = zPAR_TOK_SETINSTANCE,
	};

	struct Instruction
	{
		int address;
		int size;
		InstructionType token;
		int argument;
		byte index;
		int arity;
		int prio;
		bool ret;

		void Read(zCParser* parser, int address, bool isReturn)
		{
			this->address = address;
			this->token = (InstructionType)parser->stack.stack[address++];

			switch (this->token)
			{
			case InstructionType::PushInt:
				this->size = 5;
				this->argument = *(int*)&parser->stack.stack[address];
				this->index = 0;
				this->arity = 0;
				this->prio = 99;
				this->ret = true;
				break;

			case InstructionType::PushVar:
				this->size = 5;
				this->argument = *(int*)&parser->stack.stack[address];
				this->index = 0;
				this->arity = 0;
				this->prio = 99;
				this->ret = true;
				break;

			case InstructionType::PushArrayVar:
				this->size = 6;
				this->argument = *(int*)&parser->stack.stack[address];
				this->index = *(byte*)&parser->stack.stack[address + 4];
				this->arity = 0;
				this->prio = 99;
				this->ret = true;
				break;

			case InstructionType::PushInst:
				this->size = 5;
				this->argument = *(int*)&parser->stack.stack[address];
				this->index = 0;
				this->arity = 0;
				this->prio = 99;
				this->ret = true;
				break;

			case InstructionType::AssignString:
			case InstructionType::AssignStringPtr:
			case InstructionType::AssignFunc:
			case InstructionType::AssignFloat:
			case InstructionType::AssignInt:
			case InstructionType::AssignPlus:
			case InstructionType::AssignMinus:
			case InstructionType::AssignMul:
			case InstructionType::AssignDiv:
			case InstructionType::AssignInst:
				this->size = 1;
				this->argument = 0;
				this->index = 0;
				this->arity = 2;
				this->prio = 0;
				this->ret = false;
				break;

			case InstructionType::Plus:
			case InstructionType::Minus:
			case InstructionType::ShiftLeft:
			case InstructionType::ShiftRight:
			case InstructionType::BitOr:
			case InstructionType::BitAnd:
			case InstructionType::LogOr:
				this->size = 1;
				this->argument = 0;
				this->index = 0;
				this->arity = 2;
				this->prio = 2;
				this->ret = true;
				break;

			case InstructionType::Mul:
			case InstructionType::Div:
			case InstructionType::Mod:
			case InstructionType::LogAnd:
				this->size = 1;
				this->argument = 0;
				this->index = 0;
				this->arity = 2;
				this->prio = 3;
				this->ret = true;
				break;

			case InstructionType::Less:
			case InstructionType::More:
			case InstructionType::LessEqual:
			case InstructionType::Equal:
			case InstructionType::NotEqual:
			case InstructionType::MoreEqual:
				this->size = 1;
				this->argument = 0;
				this->index = 0;
				this->arity = 2;
				this->prio = 1;
				this->ret = true;
				break;

			case InstructionType::BitNot:
			case InstructionType::LogNot:
				this->size = 1;
				this->argument = 0;
				this->index = 0;
				this->arity = 1;
				this->prio = 4;
				this->ret = true;
				break;

			case InstructionType::UnaryMinus:
			case InstructionType::UnaryPlus:
				this->size = 1;
				this->argument = 0;
				this->index = 0;
				this->arity = 1;
				this->prio = 1;
				this->ret = true;
				break;

			case InstructionType::Call:
			{
				this->size = 5;
				this->argument = *(int*)&parser->stack.stack[address];
				this->index = 0;
				zCPar_Symbol* func = parser->SearchFuncWithStartAddress(this->argument);
				
				//if (!func)
					//WARN() << "No function found at address: " << this->argument << std::endl;

				this->arity = func ? func->ele : 0;
				this->prio = 99;
				this->ret = func && func->HasFlag(zPAR_FLAG_RETURN);
				break;
			}

			case InstructionType::CallExternal:
				this->size = 5;
				this->argument = *(int*)&parser->stack.stack[address];
				this->index = 0;
				this->arity = parser->GetSymbol(this->argument)->ele;
				this->prio = 99;
				this->ret = parser->GetSymbol(this->argument)->HasFlag(zPAR_FLAG_RETURN);
				break;

			case InstructionType::Return:
				this->size = 1;
				this->argument = 0;
				this->index = 0;
				this->arity = isReturn;
				this->prio = 0;
				this->ret = false;
				break;

			case InstructionType::Jump:
				this->size = 5;
				this->argument = *(int*)&parser->stack.stack[address];
				this->index = 0;
				this->arity = 0;
				this->prio = 0;
				this->ret = false;
				break;

			case InstructionType::JumpFalse:
				this->size = 5;
				this->argument = *(int*)&parser->stack.stack[address];
				this->index = 0;
				this->arity = 1;
				this->prio = 0;
				this->ret = true;
				break;

			case InstructionType::SetInstance:
				this->size = 5;
				this->argument = *(int*)&parser->stack.stack[address];
				this->index = 0;
				this->arity = 0;
				this->prio = 0;
				this->ret = false;
				break;

			default:
				ASSERT(false);
				break;
			}
		}
	};

	struct Replique
	{
		static constexpr int HeroNpc = -2;

		int sourceNpc;
		int targetNpc;
		string name;
		string text;

		Replique() :
			sourceNpc{ -1 },
			targetNpc{ -1 }
		{

		}

		void SetName(const string& name)
		{
			this->name = name;
			this->text = "";

			const int number = ogame->GetCutsceneManager()->LibValidateOU(Z name);

			if (number < 0)
				return;

			zCCSBlock* block = ogame->csMan->LibGet(number);
			zCCSBlockBase* blockBase = block->GetChild(0);
			zCCSAtomicBlock* atomic = blockBase->CastTo<zCCSAtomicBlock>();
			oCMsgConversation* message = atomic->command->CastTo<oCMsgConversation>();
			this->text = A message->text;

			return;
		}

		bool IsSoundExists()
		{
			string name = this->name;
			name += ".WAV";
			return vdf_fexists(const_cast<char*>(name.GetVector()), VDF_DEFAULT);
		}
	};

	class Splitter
	{
	private:
		const int ai_output;
		const int info_addchoice;
		const int self;
		const int other;
		const int hero;
		const int c_info;
		const int c_info_npc;
		const int c_info_information;

		std::unordered_map<int, Symbol> functions;

		std::vector<Instruction> GetInstructions(int stackPos)
		{
			std::vector<Instruction> instructions;
			instructions.reserve(128u);

			int address = stackPos;
			int minAddress = address + 1;

			while (true)
			{
				instructions += {};
				instructions.back().Read(parser, address, false);
				address += instructions.back().size;

				if (instructions.size() == 1 && instructions.back().token == InstructionType::Jump)
					return {};

				if (instructions.back().token == InstructionType::JumpFalse)
					minAddress = std::max(minAddress, instructions.back().argument);

				if (address >= parser->stack.stacksize)
					break;

				if (instructions.back().token == InstructionType::Return && instructions.back().address >= minAddress)
					break;
			}

			return instructions;
		}

		bool TryParseInfo(const Symbol& info, int& npc, int& information)
		{
			if (info.GetType() != Symbol::Type::Instance)
				return false;

			if (info.GetSymbol()->parent != parser->GetSymbol(c_info) && COA(info.GetSymbol(), parent, parent) != parser->GetSymbol(c_info))
				return false;

			npc = -1;
			information = -1;

			std::vector<Instruction> instructions = GetInstructions(info.GetValue<int>(0));

			for (size_t i = 2; i < instructions.size(); i++)
			{
				if (instructions[i].token != InstructionType::AssignInt && instructions[i].token != InstructionType::AssignFunc)
					continue;

				if (instructions[i - 1].token != InstructionType::PushVar)
					continue;

				if (instructions[i - 2].token != InstructionType::PushInt)
					continue;

				if (instructions[i - 1].argument == c_info_npc)
					npc = instructions[i - 2].argument;
				else
					if (instructions[i - 1].argument == c_info_information)
						information = instructions[i - 2].argument;
			}

			if (information != -1)
				return true;

			WARN() << "Can't parse " << info.GetSymbol()->name.ToChar() << std::endl;

			return false;
		}

		void ExtractRecursive(int stackPos, int npc, std::vector<Replique>& repliques, std::vector<int>& poses)
		{
			std::vector<Instruction> instructions = GetInstructions(stackPos);

			for (size_t i = 0; i < instructions.size(); i++)
			{
				if (instructions[i].token == InstructionType::CallExternal && instructions[i].argument == info_addchoice)
				{
					if (!i)
					{
						WARN() << "Can't parse Info_AddChoice at " << instructions[i].address << "!. " << "It is first instruction" << std::endl;
						continue;
					}

					if (instructions[i - 1].token != InstructionType::PushInt)
					{
						WARN() << "Can't parse Info_AddChoice at " << instructions[i].address << "!. " << "No PushInt before" << std::endl;
						continue;
					}

					Symbol callee{ parser, instructions[i - 1].argument };

					if (callee.GetType() != Symbol::Type::Func)
					{
						WARN() << "Can't parse Info_AddChoice at " << instructions[i].address << "!. " << "No function with specified index" << std::endl;
						continue;
					}

					if (!(poses & callee.GetValue<int>(0)))
					{
						poses += callee.GetValue<int>(0);
						ExtractRecursive(callee.GetValue<int>(0), npc, repliques, poses);
					}

					continue;
				}

				if (instructions[i].token == InstructionType::Call && !(poses & instructions[i].argument))
				{
					poses += stackPos;
					ExtractRecursive(instructions[i].argument, npc, repliques, poses);
					continue;
				}

				if (instructions[i].token == InstructionType::CallExternal && instructions[i].argument == ai_output)
				{
					if (i < 3)
					{
						WARN() << "AI_Output is too early" << std::endl;
						continue;
					}

					if
					(
						instructions[i - 3].token != InstructionType::PushInst ||
						instructions[i - 2].token != InstructionType::PushInst ||
						instructions[i - 1].token != InstructionType::PushVar ||
						Symbol{ parser, instructions[i - 1].argument }.GetType() != Symbol::Type::VarString
					)
					{
						WARN() << "AI_Output arguments failed" << std::endl;
						continue;
					}

					int sourceNpc = instructions[i - 3].argument;
					int targetNpc = instructions[i - 2].argument;
					string svmName = Symbol{ parser, instructions[i - 1].argument }.GetValue<string>(0);

					if (sourceNpc != self && sourceNpc != other && sourceNpc != hero)
					{
						WARN() << "AI_Output sourceNpc failed: " << COA(parser->GetSymbol(sourceNpc), name).ToChar() << std::endl;
						continue;
					}

					if (targetNpc != self && targetNpc != other && targetNpc != hero)
					{
						WARN() << "AI_Output targetNpc failed: " << COA(parser->GetSymbol(targetNpc), name).ToChar() << std::endl;
						continue;
					}

					if (sourceNpc == self)
						sourceNpc = npc;
					else
						sourceNpc = Replique::HeroNpc;

					if (targetNpc == self)
						targetNpc = npc;
					else
						targetNpc = Replique::HeroNpc;

					Replique replique;
					replique.sourceNpc = sourceNpc;
					replique.targetNpc = targetNpc;
					replique.SetName(svmName);

					if (!Options::UnvoicedOnly || !replique.IsSoundExists())
						repliques += std::move(replique);
				}
			}
		}

		bool TryGetRepliques(const Symbol& info, int& npc, std::vector<Replique>& repliques)
		{
			int information;

			if (!TryParseInfo(info, npc, information))
				return false;

			repliques.clear();
			repliques.reserve(64u);

			std::vector<int> poses;
			poses.reserve(64u);
			poses += Symbol{ parser, information }.GetValue<int>(0);

			ExtractRecursive(poses.back(), npc, repliques, poses);
			return true;
		}

		void WriteRepliques(const Symbol& function)
		{
			int npc;
			std::vector<Replique> repliques;

			if (!TryGetRepliques(function, npc, repliques))
				return;

			if (repliques.empty())
				return;

			NPC(npc) << std::endl;
			NPC(npc) << "\t\t\t" << function.GetSymbol()->name.ToChar() << std::endl;

			for (const Replique& replique : repliques)
			{
				string line = A"[" + replique.name + "] \"" + replique.text + "\"";

				if (replique.sourceNpc != npc)
					NPC(npc) << "\t";

				NPC(npc) << line << std::endl;
			}
		}

	public:
		Splitter() :
			ai_output{ parser->GetIndex("AI_OUTPUT") },
			info_addchoice{ parser->GetIndex("INFO_ADDCHOICE") },
			self{ parser->GetIndex("SELF") },
			other{ parser->GetIndex("OTHER") },
			hero{ parser->GetIndex("HERO") },
			c_info{ parser->GetIndex("C_INFO") },
			c_info_npc{ parser->GetIndex("C_INFO.NPC") },
			c_info_information{ parser->GetIndex("C_INFO.INFORMATION") }
		{
			std::vector<int> indexes = { ai_output, info_addchoice, self, other, hero, c_info, c_info_npc, c_info_information };
			
			for (int index : indexes)
				ASSERT(index != -1);

			for (int i = 0; i < parser->symtab.GetNumInList(); i++)
			{
				Symbol symbol{ parser, i };

				if (symbol.GetType() != Symbol::Type::Instance && symbol.GetType() != Symbol::Type::Prototype &&
					symbol.GetType() != Symbol::Type::Func)
				{
					continue;
				}

				functions[symbol.GetValue<int>(0)] = symbol;
			}
		}

		void Execute()
		{
			for (auto it = functions.begin(); it != functions.end(); it++)
				WriteRepliques(it->second);

			std::unordered_set<string> svms;
			svms.reserve(64u);

			const string cantUseItem = "$SC_CANTUSEITEM";

			if (svms.insert(cantUseItem).second)
				SVM() << cantUseItem << std::endl;

			for (int i = 0; i < parser->symtab.GetNumInList(); i++)
			{
				Symbol symbol{ parser, i };

				if (symbol.GetType() != Symbol::Type::VarString || !symbol.IsGlobal())
					continue;

				for (int k = 0; k < symbol.GetValuesCount(); k++)
				{
					string value = symbol.GetValue<string>(k);

					if (value.Length() <= 1 || value[0] != '$')
						continue;

					value.Upper();

					if (svms.insert(value).second)
						SVM() << value << std::endl;
				}
			}
		}
	};

	extern Sub<void> splitIt;
	Sub<void> splitIt(ZSUB(MenuLoop), []()
		{
			Splitter{}.Execute();
			splitIt = {};
		});
}