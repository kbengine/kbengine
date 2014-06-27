2014-06-07  i decide to write a wrapper for which can be used in cocos2dx. To make we can use cocos2dx implement client, kbe work as game server. This project implement the wrapper library. This is pure cpp way, instead of js technology. if you interest with js, you should check html5 client of kbe and cocos2dx-js. it should work also.   

First, it is windows version. the roadmap will include ios platform. soon is andriod platform.  today is my birthday. another reason is i have woring on bxxworld engine for several years. choose kbe can help me reuse my experience on mmorpg project. that's all. 

2014-06-11 i will correct it, first is  ios library. since it is no difficult create win32 project without any change of kbe source  code. 

if you have any suggestion, mail me. cnsoft#gmail.com. i am always here. 


RoadMap: 
  Step1: do some r&d.  
    1. read code find the enterpoint. 
    
    2. do some tech test. 
    
    3. read kbe source code and get familier with its protocol design. since we will work on it. 
    
  Step2: do small demo.
  
    3. implement and test client connection with server. 
    
    4. implement a chat client with cocso2dx.
    
  Step3: write much more bigger demo.
  
    5. do more works to implement a base game framework or demos.




History:
  2014-06-13 compiled a ios library project. maybe it can't used . but it is a start point. after this , we can keep logic function, remove socket dependented source , replace with  cross platform socket works with cocos2d-x. 
  tip:  SOL_TCP is linux only. it seems we should choose  BSD Socket. 

  2014-06-23  KBE packet encode library. 80% . auto login to loginapp , prepare system embed messages. (datatypes not done) next will debug login to base_gateway. (but, today my debug server offline .so faint) . 
   
  2014-06-27  Let's summary the new progress of this week (30% on this project) . 
      My debug server has not been fixed. so i have to setup a debug server on windows. it cost some out of planning time.
      Port from CPP or C# version confuse me. i am not sure the percentage of each part. a mixture thing. 
      System Level method call is handled almost done. some is only read and write stream of the socket packet data. 
      After this week, we  have a client can connected to KBE server .  it can start reqCreateAvatar request, got response. 
      Next step: write some hard code style to decode AvatarInfo data. so we can start port avatar feature , after that, almost 
       
      
  
