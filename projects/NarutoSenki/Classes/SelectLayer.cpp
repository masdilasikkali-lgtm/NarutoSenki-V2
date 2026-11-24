#include <time.h>
#include "StartMenu.h"
#include "Constants/UiFlowKeys.hpp"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <jni.h>
#include "../../../cocos2dx/platform/android/jni/JniHelper.h"
#endif

SelectLayer::~SelectLayer()
{
	CC_SAFE_RELEASE(_selectList);

	auto handler = getGameModeHandler();
	handler->selectLayer = nullptr;
	if (!isStart)
		handler->resetCheats();
}

void SelectLayer::onGameStart()
{
	if (isStart)
		return;
	else
		isStart = true;

	setKeypadEnabled(false);

	SimpleAudioEngine::sharedEngine()->stopBackgroundMusic(true);
	SimpleAudioEngine::sharedEngine()->playEffect("Audio/Menu/confirm.ogg");

	Scene *loadScene = Scene::create();
	LoadLayer *loadLayer = LoadLayer::create();

	auto gameModeHandler = getGameModeHandler();
	gameModeHandler->selectLayer = this;
	gameModeHandler->onInitHeros();
	loadLayer->preloadAudio();
	loadScene->addChild(loadLayer);

	Director::sharedDirector()->replaceScene(TransitionFade::create(1.25f, loadScene));
}

void SelectLayer::onExitTransitionDidStart()
{
	Layer::onExitTransitionDidStart();
	setKeypadEnabled(false);
}

void SelectLayer::onEnterTransitionDidFinish()
{
	Layer::onEnterTransitionDidFinish();
	setKeypadEnabled(true);

	// --- MULAI UI PILIH MAP ---
	_selectedMap = 0;
	UserDefault::getInstance()->setIntegerForKey("selected_map_id", 0);

	_mapLabel = Label::createWithSystemFont("MAP: 1 (KLIK GANTI)", "Arial", 18);
	_mapLabel->enableOutline(Color4B::BLACK, 2);

	auto mapMenuItem = MenuItemLabel::create(_mapLabel, CC_CALLBACK_1(SelectLayer::onChangeMap, this));
	auto visibleSize = Director::getInstance()->getVisibleSize();
	mapMenuItem->setPosition(Vec2(visibleSize.width * 0.5f, 30));

	auto mapMenu = Menu::create(mapMenuItem, nullptr);
	mapMenu->setPosition(Vec2::ZERO);
	this->addChild(mapMenu, 999);
	// --- SELESAI UI PILIH MAP ---
}

void SelectLayer::keyBackClicked()
{
	setKeypadEnabled(false);

	lua_call_func(UiFlowKeys::kBackToStartMenu);
}

void SelectLayer::onChangeMap(Ref* sender)
{
	_selectedMap = (_selectedMap + 1) % 3;

	if (_selectedMap == 0) {
		_mapLabel->setString("MAP: 1 (KLIK GANTI)");
	} else if (_selectedMap == 1) {
		_mapLabel->setString("MAP: 2 (KLIK GANTI)");
	} else if (_selectedMap == 2) {
		_mapLabel->setString("MAP: 3 (KLIK GANTI)");
	}

	UserDefault::getInstance()->setIntegerForKey("selected_map_id", _selectedMap);
	UserDefault::getInstance()->flush();
}
