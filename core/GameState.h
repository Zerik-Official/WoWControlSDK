#pragma once

namespace GameState {

enum class Screen {
    UNKNOWN,
    LOGIN,
    CHARSELECT,
    WORLD
};

void        initialize();
char*       getStateJson();
const char* getRealmList();
Screen      getScreen();
void        setScreen(Screen s);
bool        isLoginLatched();
void        setLoginLatch(bool value);
void        setPendingCharacter(const char* name);
const char* getPendingCharacter();

}