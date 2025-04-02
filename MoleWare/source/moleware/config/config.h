#pragma once
#include "../../../external/imgui/imgui.h"

// CBA to make proper atm, it's 03:42 right now.
// For now just stores config values don't mind it too much
//
// (FYI THIS IS A HORRID SOLUTION BUT FUNCTIONS) 

namespace Config {
	extern bool esp;
	extern bool glow;
	extern bool showHealth;
	extern bool showArmor;
	extern bool showDead;
	extern bool showHead;
	extern ImVec4 headDotColor;
	extern float headDotRadius;
	extern bool distance;
	extern bool teamCheck;
	extern bool espFill;
	extern float espThickness;
	extern float espFillOpacity;
	extern ImVec4 espColor;
	extern bool showNameTags;
	extern bool showWeapon;

	extern bool Night;
	extern ImVec4 NightColor;

	extern bool enemyChamsInvisible;
	extern bool enemyChams;
	extern bool teamChams;
	extern bool teamChamsInvisible;
	extern int chamsMaterial;

	extern ImVec4 colVisualChams;
	extern ImVec4 colVisualChamsIgnoreZ;
	extern ImVec4 teamcolVisualChamsIgnoreZ;
	extern ImVec4 teamcolVisualChams;

	extern bool armChams;
	extern bool viewmodelChams;
	extern ImVec4 colViewmodelChams;
	extern ImVec4 colArmChams;

	extern bool fovEnabled;
	extern float fov;

	extern bool antiflash;

	extern bool Night;

	extern bool aimbot;
	extern bool aimbotTeamCheck;
	extern float aimbotSmoothness;
	extern float aimbot_fov;
	extern bool rcs;
	extern bool fov_circle;
	extern ImVec4 fovCircleColor;

	extern bool bunnyHop;

	extern bool triggerBot;
	extern int triggerBotDelay;
}
