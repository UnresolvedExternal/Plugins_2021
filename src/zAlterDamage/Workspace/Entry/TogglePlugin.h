namespace NAMESPACE
{
	ActiveValue<bool> enablePlugin{ true };

	Sub togglePlugin(ZSUB(GameEvent::Loop), []
		{
			static KeyCombo key{ { KEY_LSHIFT, KEY_P }, { KEY_RSHIFT, KEY_P } };

			if (!key.GetToggled())
				return;

			enablePlugin = !enablePlugin;

			COA(ogame, GetTextView(), Printwin(enablePlugin ? "zAlterDamage enabled..." : "zAlterDamage disabled..."));
		});
}
