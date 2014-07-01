#include "HelloWorldScene.h"

USING_NS_CC;

//#include "network/network_interface.hpp"
//#include "cstdkbe/cstdkbe.hpp"
//#include "cstdkbe/platform.hpp"

#include "KBEGameSocket.h"
#include "KMessage.h"
#include "..\LibKBEClient\KBundle.h"
#include "KBEApplication.h"
#include "KAccount.h"


void testKBE(){
	

	KBEGameSocket& game_sock = KBEGameSocket::getInstance();
	std::string s_ip = "23.239.157.86";
	s_ip = "192.168.10.101";
	bool c_ok = game_sock.connectionServer(s_ip.c_str(),10013);
	if(!c_ok)
		CCMessageBox("not connected","net");
	
	//KMessage::bindmessage();
	KBE_init();
	//send ["Loginapp_importClientMessages"]);
	KBEngineClient::KBundle* bundle = new KBundle();

	KNetworkInterface* network = new KNetworkInterface();
	
	bundle->newmessage( *KMessage::messages["Loginapp_importClientMessages"] );
	bundle->send(*network);

	//next we should recv huge client methods.... 


	/*KBEngineClient::Message msg;
	msg.setOpcode(100);
	game_sock.sendMessage(msg);*/
	//TODO works like a clientapp. say hello. send importEntiityDef message.
	//pack1:
	//Message.messages["Loginapp_importClientMessages"] = new Message(5, "importClientMessages", 0, 0, new List<Byte>(), null);
	//bundle.newMessage(Message.messages["Loginapp_importClientMessages"]);
	//bundle.send(networkInterface_);
	//bundle get stream from message.
	//scene->addChild(game_sock ,19999,19999);
}



CCScene* HelloWorld::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // 'layer' is an autorelease object
    HelloWorld *layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !CCLayer::init() )
    {
        return false;
    }
    
    CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
    CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    CCMenuItemImage *pCloseItem = CCMenuItemImage::create(
                                        "CloseNormal.png",
                                        "CloseSelected.png",
                                        this,
                                        menu_selector(HelloWorld::menuCloseCallback));
    
	pCloseItem->setPosition(ccp(origin.x + visibleSize.width - pCloseItem->getContentSize().width/2 ,
                                origin.y + pCloseItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    CCMenu* pMenu = CCMenu::create(pCloseItem, NULL);
    pMenu->setPosition(CCPointZero);
    this->addChild(pMenu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    
    CCLabelTTF* pLabel = CCLabelTTF::create("Hello World", "Arial", 24);
    
    // position the label on the center of the screen
    pLabel->setPosition(ccp(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - pLabel->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(pLabel, 1);

    // add "HelloWorld" splash screen"
    CCSprite* pSprite = CCSprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    pSprite->setPosition(ccp(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

    // add the sprite as a child to this layer
    this->addChild(pSprite, 0);
    
	//start test kbe connection.
	testKBE();
	
	this->setTouchEnabled(true);
    return true;
}


void HelloWorld::menuCloseCallback(CCObject* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT) || (CC_TARGET_PLATFORM == CC_PLATFORM_WP8)
	CCMessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
#else
    CCDirector::sharedDirector()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
#endif
}


void HelloWorld::ccTouchesBegan( CCSet *pTouches, CCEvent *pEvent )
{
	KAccount* account = (KAccount*) KBEngineClient::ClientApp::getInstance().pPlayer();
	account->sendMsg(" cocos2dx coming! ");
}
