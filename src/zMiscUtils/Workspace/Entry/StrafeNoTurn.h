#include <array>

namespace NAMESPACE
{
	void SetStrafeTurnings()
	{
		struct Code
		{
			std::array<byte, 5> left;
			std::array<byte, 5> right;

			Code(const std::array<byte, 5>& left, const std::array<byte, 5>& right) :
				left{ left },
				right{ right }
			{

			}
		};

		static std::optional<Code> originalCode;

		// if option has never set do not touch the engine
		if (!Options::StrafeNoTurn && !originalCode.has_value())
			return;

		Unlocked<std::array<byte, 5>> left = ZENDEF(0x00614AC3, 0x006378D6, 0x0063E4E7, 0x0069AD37);
		Unlocked<std::array<byte, 5>> right = ZENDEF(0x00614B4F, 0x00637967, 0x0063E557, 0x0069ADA7);

		if (!originalCode.has_value())
			originalCode.emplace(left, right);

		if (Options::StrafeNoTurn)
		{
			left = { 0x31, 0xC0, 0x90, 0x90, 0x90 };
			right = { 0x31, 0xC0, 0x90, 0x90, 0x90 };
		}
		else
		{
			left = originalCode->left;
			right = originalCode->right;
			originalCode.reset();
		}
	}

	Sub listenStrafeNoTurn(ZSUB(GameEvent::Execute), []()
		{
			Options::StrafeNoTurn.onChange += &SetStrafeTurnings;
		});
}