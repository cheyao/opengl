#include "ui/screens/mainUI.hpp"

#include "game.hpp"
#include "opengl/renderer.hpp"
#include "third_party/Eigen/Core"
#include "ui/UIScreen.hpp"
#include "ui/components/backgroundComponent.hpp"
#include "ui/components/buttonComponent.hpp"
#include "ui/components/textComponent.hpp"
#include "utils.hpp"

MainUI::MainUI(class Game* game) : UIScreen(game) {
	game->setPause(true);
	game->getRenderer()->setWindowRelativeMouseMode(SDL_FALSE);

	new BackgroundComponent(this, Eigen::Vector3f(1.0f, 0.0f, 0.0f));

	ButtonComponent* start = new ButtonComponent(this, getGame()->getTexture("ui" SEPARATOR "start.png"),
						     Eigen::Vector2f(CENTER, CENTER));

	/* FIXME:
	 * /Users/ray/Developer/opengl/src/ui/UIScreen.cpp:42:14: runtime error: member call on address 0x7ffbd9e18910
	 * which does not point to an object of type 'UIComponent' 0x7ffbd9e18910: note: object has invalid vptr ff 7f
	 * 00 00  00 00 00 00 00 00 00 80  17 26 9e bd ff 07 00 80  03 00 74 61 72 74 5f 67  61 6d 65 00
	 * 	      ^~~~~~~~~~~~~~~~~~~~~~~
	 * 	      invalid vptr
	 * SUMMARY: UndefinedBehaviorSanitizer: undefined-behavior /Users/ray/Developer/opengl/src/ui/UIScreen.cpp:42:14
	 * UndefinedBehaviorSanitizer:DEADLYSIGNAL
	 * ==27428==ERROR: UndefinedBehaviorSanitizer: SEGV on unknown address (pc 0x0001038b4dee bp 0x7ffeec6175b0 sp
	 * 0x7ffeec617500 T35930999)
	 * ==27428==The signal is caused by a READ memory access.
	 * ==27428==Hint: this fault was caused by a dereference of a high value address (see register values below).
	 * Disassemble the provided pc to learn which register was used. #0 0x1038b4dee in UIScreen::touch(unsigned long
	 * long const&, float, float, bool) UIScreen.cpp:42 #1 0x1037aa3ac in EventManager::manageEvent(SDL_Event
	 * const&) eventManager.cpp:76 #2 0x1035f2ca0 in Game::event(SDL_Event const&) game.cpp:345 #3 0x1035ec300 in
	 * SDL_AppEvent main.cpp:69 #4 0x104ca62f4 in SDL_DispatchMainCallbackEvent+0x34
	 * (libSDL3.0.dylib:x86_64+0x892f4) #5 0x104ca61f7 in SDL_DispatchMainCallbackEvents+0xa7
	 * (libSDL3.0.dylib:x86_64+0x891f7) #6 0x104ca60de in SDL_IterateMainCallbacks+0x1e
	 * (libSDL3.0.dylib:x86_64+0x890de) #7 0x104eb6fb0 in SDL_EnterAppMainCallbacks_REAL+0xa0
	 * (libSDL3.0.dylib:x86_64+0x299fb0) #8 0x104c5172e in SDL_EnterAppMainCallbacks+0x3e
	 * (libSDL3.0.dylib:x86_64+0x3472e) #9 0x1035ebab0 in SDL_main SDL_main_impl.h:59 #10 0x104ca6350 in
	 * SDL_RunApp_REAL+0x40 (libSDL3.0.dylib:x86_64+0x89350) #11 0x104c68a03 in SDL_RunApp_DEFAULT+0x33
	 * (libSDL3.0.dylib:x86_64+0x4ba03) #12 0x104c56b9e in SDL_RunApp+0x2e (libSDL3.0.dylib:x86_64+0x39b9e) #13
	 * 0x1035ec4c6 in main SDL_main_impl.h:208 #14 0x7fff20471f3c in start+0x0 (libdyld.dylib:x86_64+0x15f3c)
	 *
	 * ==27428==Register values:
	 * rax = 0x8000000000000000  rbx = 0x0000000000000000  rcx = 0x0000000000000000  rdx = 0x0000000000000000
	 * rdi = 0x00007ffbd9e18910  rsi = 0x00007ffeec617930  rbp = 0x00007ffeec6175b0  rsp = 0x00007ffeec617500
	 *  r8 = 0x0000000106cbe7c0   r9 = 0x0000000000000000  r10 = 0x00007fff20423e82  r11 = 0x0000000000000206
	 * r12 = 0x0000000000000000  r13 = 0x0000000000000000  r14 = 0x0000000000000000  r15 = 0x0000000000000000
	 * UndefinedBehaviorSanitizer can not provide additional info.
	 * SUMMARY: UndefinedBehaviorSanitizer: SEGV UIScreen.cpp:42 in UIScreen::touch(unsigned long long const&,
	 * float, float, bool)
	 * ==27428==ABORTING
	 */
	start->onClick([this] {
		getGame()->setPause(false);
		getGame()->removeUI(this);
		getGame()->getRenderer()->setWindowRelativeMouseMode(SDL_TRUE);
		delete this;
	});

	// Test "Hello 你好 こんにちは Bonne journée"
	// TextComponent* text = new TextComponent(this, "Hello");
	// (void)text;
	// new TextComponent(this, U"Hello! 你好！こんにちは！Bonjour! привет! ¡Hola!", Eigen::Vector2f(CENTER, -10));
	new TextComponent(this, "start_game", Eigen::Vector2f(CENTER, CENTER));
}
