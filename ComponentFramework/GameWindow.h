#pragma once

class GameWindow
{
	// delete the move and copy constructers
	GameWindow(const GameWindow&) = delete;
	GameWindow(GameWindow&&) = delete;
	GameWindow& operator = (const GameWindow&) = delete;
	GameWindow& operator = (GameWindow&&) = delete;

private:
	bool showDebugGizmos = false;

public:
	explicit GameWindow();
	~GameWindow() {}

	void ShowGameWindow(bool* pOpen);
};

