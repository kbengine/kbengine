Project : KBE Mini Client Based On  KBE Protocol

0. Create Socket Object . SendMessage and Handle Received Message. 

1. Read KBE Code.
   KBE use Bundle to store  a memorystream list, work as data queue, filled sendbuff with aligned stream data.(it seems that can increase the performance).
   KBE use NetworkInterface as a wrapper of socket manager. MsgReader manage read whole message , and call message 's method  handleMessage  process msg . when message object created, a method be passed as handler. (use c# reflection) , so in handleMessage function, the saved handler be called. ( e.g: Client_onImportClientMessages )
   Client_onImportClientMessages is a function of KBEngine. in it, client base cell method message is binded. 
2. Login Part:  
