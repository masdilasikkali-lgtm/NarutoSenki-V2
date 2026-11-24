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
	SimpleAudioEngine::sharedEngine()->playEffect("Audio/Menu/confirm.ogg");

	auto visibleSize = Director::getInstance()->getVisibleSize();
	_mapSelectLayer = LayerColor::create(Color4B(0, 0, 0, 0), visibleSize.width, visibleSize.height);
	this->addChild(_mapSelectLayer, 9999);

	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->setSwallowTouches(true);
	touchListener->onTouchBegan = [](Touch* touch, Event* event) { return true; };
	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, _mapSelectLayer);

	auto fadeBlack = FadeTo::create(0.5f, 255);
	auto wait2Sec = DelayTime::create(1.5f);
	auto showUI = CallFunc::create(CC_CALLBACK_0(SelectLayer::showMapSelectLayer, this));

	_mapSelectLayer->runAction(Sequence::create(fadeBlack, wait2Sec, showUI, nullptr));
}

void SelectLayer::showMapSelectLayer()
{
	if (!_mapSelectLayer)
		return;

	auto visibleSize = Director::getInstance()->getVisibleSize();
	_selectedMap = 0;
	UserDefault::getInstance()->setIntegerForKey("selected_map_id", 0);

	auto titleLabel = Label::createWithSystemFont("PILIH ARENA / MAP", "Arial", 26);
	titleLabel->setColor(Color3B::YELLOW);
	titleLabel->setPosition(Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.78f));
	_mapSelectLayer->addChild(titleLabel);

	_mapLabel = Label::createWithSystemFont("< MAP: RANDOM >", "Arial", 22);
	_mapLabel->setColor(Color3B::WHITE);
	auto mapItem = MenuItemLabel::create(_mapLabel, CC_CALLBACK_1(SelectLayer::onChangeMap, this));
	mapItem->setPosition(Vec2(visibleSize.width * 0.5f, visibleSize.height * 0.52f));

	auto returnLabel = Label::createWithSystemFont("[ RETURN ]", "Arial", 20);
	returnLabel->setColor(Color3B(255, 100, 100));
	auto returnItem = MenuItemLabel::create(returnLabel, CC_CALLBACK_1(SelectLayer::onMapReturn, this));
	returnItem->setPosition(Vec2(visibleSize.width * 0.25f, visibleSize.height * 0.22f));

	auto startLabel = Label::createWithSystemFont("[ START ]", "Arial", 20);
	startLabel->setColor(Color3B(100, 255, 100));
	auto startItem = MenuItemLabel::create(startLabel, CC_CALLBACK_1(SelectLayer::onMapStart, this));
	startItem->setPosition(Vec2(visibleSize.width * 0.75f, visibleSize.height * 0.22f));

	auto menu = Menu::create(mapItem, returnItem, startItem, nullptr);
	menu->setPosition(Vec2::ZERO);
	_mapSelectLayer->addChild(menu);
}

void SelectLayer::onChangeMap(Ref* sender)
{
	SimpleAudioEngine::sharedEngine()->playEffect("Audio/Menu/select.ogg");

	int totalMaps = getMapCount();
	if (totalMaps <= 0)
		totalMaps = 1;

	_selectedMap = (_selectedMap + 1) % (totalMaps + 1);

	if (_selectedMap == 0) {
		_mapLabel->setString("< MAP: RANDOM >");
	} else {
		_mapLabel->setString(StringUtils::format("< MAP: %d >", _selectedMap));
	}

	UserDefault::getInstance()->setIntegerForKey("selected_map_id", _selectedMap);
	UserDefault::getInstance()->flush();
}

void SelectLayer::onMapReturn(Ref* sender)
{
	SimpleAudioEngine::sharedEngine()->playEffect("Audio/Menu/cancel.ogg");

	if (_mapSelectLayer) {
		_mapSelectLayer->removeFromParentAndCleanup(true);
		_mapSelectLayer = nullptr;
		_mapLabel = nullptr;
	}

	isStart = false;
	setKeypadEnabled(true);
}

void SelectLayer::onMapStart(Ref* sender)
{
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
}

void SelectLayer::keyBackClicked()
{
	if (_mapSelectLayer) {
		onMapReturn(nullptr);
		return;
	}

	setKeypadEnabled(false);
	lua_call_func(UiFlowKeys::kBackToStartMenu);
}
