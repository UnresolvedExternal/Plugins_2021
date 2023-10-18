#include <regex>
#include <sstream>

namespace NAMESPACE
{
	namespace Options
	{
		struct AniSpeedEntry
		{
			std::string regex;
			float speed;

			AniSpeedEntry() :
				speed{ 1.0f }
			{

			}

			AniSpeedEntry(const std::string& regex, float speed) :
				regex{ regex },
				speed{ speed }
			{
				
			}

			AniSpeedEntry(const AniSpeedEntry&) = default;
			AniSpeedEntry(AniSpeedEntry&&) = default;
			AniSpeedEntry& operator=(const AniSpeedEntry&) = default;
			AniSpeedEntry& operator=(AniSpeedEntry&&) = default;

			bool operator==(const AniSpeedEntry& y) const
			{
				return regex == y.regex && speed == y.speed;
			}

			bool operator!=(const AniSpeedEntry& y) const
			{
				return !(*this == y);
			}
		};

		std::ostream& operator<<(std::ostream& out, const AniSpeedEntry& value)
		{
			return out << value.regex << ":" << value.speed;
		}

		std::istream& operator>>(std::istream& in, AniSpeedEntry& value)
		{
			std::string line;
			std::getline(in, line);

			for (int i = static_cast<int>(line.size()) - 1; i >= 0; i--)
				if (line[i] == ':')
				{
					const int suffixLength = static_cast<int>(line.size()) - i - 1;

					if (suffixLength)
					{
						std::string suffix = line.substr(i + 1, suffixLength);
						std::istringstream suffixInput{ suffix };
						suffixInput >> value.speed;

						if (value.speed > 0.0f)
						{
							value.regex = line.substr(0u, i);
							return in;
						}
					}

					break;
				}

			Message::Error(A"Wrong input: " + line.c_str(), PROJECT_NAME);
			value.regex = "^$";
			value.speed = 1.0f;
			return in;
		}

		ZOPTION(Animations,
		(
			VectorOption<AniSpeedEntry>
			{ 
				{ "^T_STAND_2_IGET$", 2.0f }, 
				{ "^T_IGET_2_STAND$", 2.0f },
				{ "^T_FOOD", 2.0f },
				{ "^T_POTION_", 2.5f},
				{ "^T_POTIONFAST", 0.75f },
				{ "^T_RICE", 2.5f }
			}
		));
	}

	namespace Options
	{
		Sub addTrivia(ZSUB(GameEvent::Execute), []
			{
				Animations.endTrivia += A"... player's animation names (RegEx) and their speed scalings";
			});

		Sub load(ZSUB(GameEvent::DefineExternals), []
			{
				ActiveOptionBase::LoadAll();
			});
	}
}
