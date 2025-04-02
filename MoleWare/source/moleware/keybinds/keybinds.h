#pragma once
#include <vector>
#include <unordered_map>

enum KeybindMode {
    TOGGLE,
    HOLD
};

struct Keybind {
    bool& var;
    int   key;
    KeybindMode mode;
    bool  isListening;
    bool  skipFrame;

    Keybind(bool& v, int k = 0, KeybindMode m = TOGGLE);
};


class Keybinds {
public:
	Keybinds();
	void pollInputs();

	void menuButton(bool& var);
private:
	std::vector<Keybind> keybinds;
    std::unordered_map<int, bool> keyStates;
};

extern Keybinds keybind;