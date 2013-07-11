/*-----------------------------------------------------------------------------------------
												global
-----------------------------------------------------------------------------------------*/
var PACKET_MAX_SIZE	= 1500;
var PACKET_MAX_SIZE_TCP = 1460;
var PACKET_MAX_SIZE_UDP = 1472;

var MESSAGE_ID_LENGTH = 2;
var MESSAGE_LENGTH_LENGTH = 2;

/*-----------------------------------------------------------------------------------------
												number64bits
-----------------------------------------------------------------------------------------*/
function KBE_INT64(hi, lo)
{
	var hi = hi;
	var lo = lo;
}

function KBE_UINT64(hi, lo)
{
	var hi = hi;
	var lo = lo;
}

/*-----------------------------------------------------------------------------------------
												memorystream
-----------------------------------------------------------------------------------------*/
function KBE_MEMORYSTREAM(size_or_buffer)
{
	if(size_or_buffer instanceof ArrayBuffer)
	{
		this.buffer = size_or_buffer;
	}
	else
	{
		this.buffer = new ArrayBuffer(size_or_buffer);
	}

	this.rpos = 0;
	this.wpos = 0;
	
	//---------------------------------------------------------------------------------
	this.readInt8 = function()
	{
		var buf = new Int8Array(this.buffer, this.rpos, 1);
		this.rpos += 1;
		return buf[0];
	}

	this.readInt16 = function()
	{
		var v = this.readUint16();
		if(v >= 32768)
			v -= 65536;
		return v;
	}
		
	this.readInt32 = function()
	{
		var v = this.readUint32();
		if(v >= 2147483648)
			v -= 4294967296;
		return v;
	}

	this.readInt64 = function()
	{
		return new KBE_INT64(this.readInt32(), this.readInt32());
	}
	
	this.readUint8 = function()
	{
		var buf = new Uint8Array(this.buffer, this.rpos, 1);
		this.rpos += 1;
		return buf[0];
	}

	this.readUint16 = function()
	{
		var buf = new Uint8Array(this.buffer, this.rpos);
		this.rpos += 2;
		return ((buf[1] & 0xff) << 8) + (buf[0] & 0xff);
	}
		
	this.readUint32 = function()
	{
		var buf = new Uint8Array(this.buffer, this.rpos);
		this.rpos += 4;
		return (buf[3] << 24) + (buf[2] << 16) + (buf[1] << 8) + buf[0];
	}

	this.readUint64 = function()
	{
		return new KBE_UINT64(this.readUint32(), this.readUint32());
	}
	
	this.readFloat = function()
	{
		try
		{
			var buf = new Float32Array(this.buffer, this.rpos, 1);
		}
		catch(e)
		{
			var buf = new Float32Array(this.buffer.slice(this.rpos, this.rpos + 4));
		}
		
		this.rpos += 4;
		return buf[0];
	}

	this.readDouble = function()
	{
		try
		{
			var buf = new Float64Array(this.buffer, this.rpos, 1);
		}
		catch(e)
		{
			var buf = new Float64Array(this.buffer.slice(this.rpos, this.rpos + 8), 0, 1);
		}
		
		this.rpos += 8;
		return buf[0];
	}
	
	this.readString = function()
	{
		var buf = new Uint8Array(this.buffer, this.rpos);
		var i = 0;
		var s = "";
		
		while(true)
		{
			if(buf[i] != 0)
			{
				s += String.fromCharCode(buf[i]);
			}
			else
			{
				i++;
				break;
			}
			
			i++;
		}
		
		this.rpos += i;
		return s;
	}

	this.readBlob = function(v)
	{
		size = this.readUint32();
		var buf = new Uint8Array(this.buffer, this.rpos, size);
		this.rpos += size;
		return buf;
	}
	
	//---------------------------------------------------------------------------------
	this.writeInt8 = function(v)
	{
		var buf = new Int8Array(this.buffer, this.wpos, 1);
		buf[0] = v;
		this.wpos += 1;
	}

	this.writeInt16 = function(v)
	{	
		this.writeInt8(v & 0xff);
		this.writeInt8(v >> 8 & 0xff);
	}
		
	this.writeInt32 = function(v)
	{
		for(i=0; i<4; i++)
			this.writeInt8(v >> i * 8 & 0xff);
	}

	this.writeInt64 = function(v)
	{
		self.writeInt32(v.hi);
		self.writeInt32(v.lo);
	}
	
	this.writeUint8 = function(v)
	{
		var buf = new Uint8Array(this.buffer, this.wpos, 1);
		buf[0] = v;
		this.wpos += 1;
	}

	this.writeUint16 = function(v)
	{
		this.writeUint8(v & 0xff);
		this.writeUint8(v >> 8 & 0xff);
	}
		
	this.writeUint32 = function(v)
	{
		for(i=0; i<4; i++)
			this.writeUint8(v >> i * 8 & 0xff);
	}

	this.writeUint64 = function(v)
	{
		self.writeUint32(v.hi);
		self.writeUint32(v.lo);
	}
	
	this.writeFloat = function(v)
	{
		try
		{
			var buf = new Float32Array(this.buffer, this.wpos, 1);
			buf[0] = v;
		}
		catch(e)
		{
			var buf = new Float32Array(1);
			buf[0] = v;
			var buf1 = new Uint8Array(this.buffer);
			var buf2 = new Uint8Array(buf.buffer);
			buf1.set(buf2, this.wpos);
		}
		
		this.wpos += 4;
	}

	this.writeDouble = function(v)
	{
		try
		{
			var buf = new Float64Array(this.buffer, this.wpos, 1);
			buf[0] = v;
		}
		catch(e)
		{
			var buf = new Float64Array(1);
			buf[0] = v;
			var buf1 = new Uint8Array(this.buffer);
			var buf2 = new Uint8Array(buf.buffer);
			buf1.set(buf2, this.wpos);
		}

		this.wpos += 8;
	}

	this.writeBlob = function(v)
	{
		size = v.length;
		if(size + 4> this.fillfree())
		{
			console.error("memorystream::writeBlob: no free!");
			return;
		}
		
		this.writeUint32(size);
		var buf = new Uint8Array(this.buffer, this.wpos, size);
		
		if(typeof(v) == "string")
		{
			for(i=0; i<size; i++)
			{
				buf[i] = v.charCodeAt(i);
			}
		}
		else
		{
			for(i=0; i<size; i++)
			{
				buf[i] = v[i];
			}
		}
		
		this.wpos += size;
	}
	
	this.writeString = function(v)
	{
		if(v.length > this.fillfree())
		{
			console.error("memorystream::writeString: no free!");
			return;
		}
		
		var buf = new Uint8Array(this.buffer, this.wpos);
		var i = 0;
		for(idx=0; idx<v.length; idx++)
		{
			buf[i++] = v.charCodeAt(idx);
		}
		
		buf[i++] = 0;
		this.wpos += i;
	}
	
	//---------------------------------------------------------------------------------
	this.readSkip = function(v)
	{
		this.rpos += v;
	}
	
	//---------------------------------------------------------------------------------
	this.fillfree = function()
	{
		return this.buffer.byteLength - this.wpos;
	}

	//---------------------------------------------------------------------------------
	this.opsize = function()
	{
		return this.wpos - this.rpos;
	}

	//---------------------------------------------------------------------------------
	this.totalsize = function()
	{
		return tthis.opsize();
	}

	//---------------------------------------------------------------------------------
	this.opfini = function()
	{
		this.rpos = this.wpos;
	}
	
	//---------------------------------------------------------------------------------
	this.getbuffer = function(v)
	{
		return this.buffer.slice(this.rpos, this.wpos);
	}
}

/*-----------------------------------------------------------------------------------------
												bundle
-----------------------------------------------------------------------------------------*/
function KBE_BUNDLE()
{
	this.memorystreams = new Array();
	this.stream = new KBE_MEMORYSTREAM(PACKET_MAX_SIZE_TCP);
	
	this.numMessage = 0;
	this.messageLengthBuffer = null;
	this.messageLength = 0;
	this.msgtype = null;
	
	//---------------------------------------------------------------------------------
	this.newMessage = function(msgtype)
	{
		this.fini(false);
		
		this.msgtype = msgtype;
		this.numMessage += 1;
		
		if(this.msgtype.length == -1)
		{
			this.messageLengthBuffer = new Uint8Array(this.stream.buffer, this.stream.wpos + MESSAGE_ID_LENGTH, 2);
		}
		
		this.writeUint16(msgtype.id);
		
		if(this.messageLengthBuffer)
		{
			this.writeUint16(0);
			this.messageLengthBuffer[0] = 0;
			this.messageLengthBuffer[1] = 0;
			this.messageLength = 0;
		}
	}

	//---------------------------------------------------------------------------------
	this.writeMsgLength = function(v)
	{
		if(this.messageLengthBuffer)
		{
			this.messageLengthBuffer[0] = v & 0xff;
			this.messageLengthBuffer[1] = v >> 8 & 0xff;
		}
	}
	
	//---------------------------------------------------------------------------------
	this.fini = function(issend)
	{
		if(this.numMessage > 0)
		{
			this.writeMsgLength(this.messageLength);
			if(this.stream)
				this.memorystreams.push(this.stream);
		}
		
		if(issend)
		{
			this.messageLengthBuffer = null;
			this.numMessage = 0;
			this.msgtype = null;
		}
	}
	
	//---------------------------------------------------------------------------------
	this.send = function(network)
	{
		this.fini(true);
		
		for(i=0; i<this.memorystreams.length; i++)
		{
			stream = this.memorystreams[i];
			network.send(stream.getbuffer());
		}
		
		this.memorystreams = new Array();
		this.stream = new KBE_MEMORYSTREAM(PACKET_MAX_SIZE_TCP);
	}
	
	//---------------------------------------------------------------------------------
	this.checkStream = function(v)
	{
		if(v > this.stream.fillfree())
		{
			this.memorystreams.push(this.stream);
			this.stream = new KBE_MEMORYSTREAM(PACKET_MAX_SIZE_TCP);
		}

		this.messageLength += v;
	}
	
	//---------------------------------------------------------------------------------
	this.writeInt8 = function(v)
	{
		this.checkStream(1);
		this.stream.writeInt8(v);
	}

	this.writeInt16 = function(v)
	{
		this.checkStream(2);
		this.stream.writeInt16(v);
	}
		
	this.writeInt32 = function(v)
	{
		this.checkStream(4);
		this.stream.writeInt32(v);
	}

	this.writeInt64 = function(v)
	{
		this.checkStream(8);
		this.stream.writeInt64(v);
	}
	
	this.writeUint8 = function(v)
	{
		this.checkStream(1);
		this.stream.writeUint8(v);
	}

	this.writeUint16 = function(v)
	{
		this.checkStream(2);
		this.stream.writeUint16(v);
	}
		
	this.writeUint32 = function(v)
	{
		this.checkStream(4);
		this.stream.writeUint32(v);
	}

	this.writeUint64 = function(v)
	{
		this.checkStream(8);
		this.stream.writeUint64(v);
	}
	
	this.writeFloat = function(v)
	{
		this.checkStream(4);
		this.stream.writeFloat(v);
	}

	this.writeDouble = function(v)
	{
		this.checkStream(8);
		this.stream.writeDouble(v);
	}
	
	this.writeString = function(v)
	{
		this.checkStream(v.length + 1);
		this.stream.writeString(v);
	}
	
	this.writeBlob = function(v)
	{
		this.checkStream(v.length + 4);
		this.stream.writeBlob(v);
	}
}

/*-----------------------------------------------------------------------------------------
												messages
-----------------------------------------------------------------------------------------*/
function KBE_MESSAGE(id, name, length, args, handler)
{
	this.id = id;
	this.name = name;
	this.length = length;
	this.reader = new KBE_MEMORYSTREAM(0);

	this.datatype2id = function(datatype)
	{	
		if(datatype == "STRING" || datatype == "STD::STRING")
			return 1;
		else if(datatype == "UINT8" || datatype == "BOOL" || datatype == "DATATYPE" || datatype == "CHAR" || datatype == "DETAIL_TYPE" ||
			datatype == "MAIL_TYPE")
			return 2;
		else if(datatype == "UINT16" | datatype == "UNSIGNED SHORT" || datatype == "SERVER_ERROR_CODE" || datatype == "ENTITY_TYPE" ||
			datatype == "ENTITY_PROPERTY_UID" || datatype == "ENTITY_METHOD_UID" || datatype == "ENTITY_SCRIPT_UID" || datatype == "DATATYPE_UID")
			return 3;
		else if(datatype == "UINT32" || datatype == "UINT" || datatype == "UNSIGNED INT" ||datatype == "ARRAYSIZE" || datatype == "SPACE_ID" || datatype == "GAME_TIME" ||
			datatype == "TIMER_ID")
			return 4;
		else if(datatype == "UINT64" || datatype == "DBID" || datatype == "COMPONENT_ID")
			return 5;
		else if(datatype == "INT8" || datatype == "COMPONENT_ORDER")
			return 6;
		else if(datatype == "INT16" || datatype == "SHORT")
			return 7;
		else if(datatype == "INT32" || datatype == "INT" ||datatype == "ENTITY_ID" || datatype == "CALLBACK_ID" || datatype == "COMPONENT_TYPE")
			return 8;
		else if(datatype == "INT64")
			return 9;
		else if(datatype == "PYTHON" || datatype == "PY_DICT" || datatype == "PY_TUPLE" || datatype == "PY_LIST" || datatype == "MAILBOX")
			return 10;
		else if(datatype == "BLOB")
			return 11;
		else if(datatype == "UNICODE")
			return 12;
		else if(datatype == "FLOAT")
			return 13;
		else if(datatype == "DOUBLE")
			return 14;
		else if(datatype == "VECTOR2")
			return 15;
		else if(datatype == "VECTOR3")
			return 16;
		else if(datatype == "VECTOR4")
			return 17;
		else
			alert("datatype2id error!");

		return 0;
	}

	// 绑定执行
	for(i=0; i<args.length; i++)
	{
		argType = args[i];
		
		if(argType == this.datatype2id("UINT8"))
		{
			args[i] = this.reader.readUint8;
		}
		else if(argType == this.datatype2id("UINT16"))
		{
			args[i] = this.reader.readUint16;
		}
		else if(argType == this.datatype2id("UINT32"))
		{
			args[i] = this.reader.readUint32;
		}
		else if(argType == this.datatype2id("UINT64"))
		{
			args[i] = this.reader.readUint64;
		}
		else if(argType == this.datatype2id("INT8"))
		{
			args[i] = this.reader.readInt8;
		}
		else if(argType == this.datatype2id("INT16"))
		{
			args[i] = this.reader.readInt16;
		}
		else if(argType == this.datatype2id("INT32"))
		{
			args[i] = this.reader.readInt32;
		}
		else if(argType == this.datatype2id("INT64"))
		{
			args[i] = this.reader.readInt64;
		}
		else if(argType == this.datatype2id("FLOAT"))
		{
			args[i] = this.reader.readFloat;
		}
		else if(argType == this.datatype2id("DOUBLE"))
		{
			args[i] = this.reader.readDouble;
		}
		else if(argType == this.datatype2id("STRING"))
		{
			args[i] = this.reader.readString;
		}
		else
		{
			args[i] = this.reader.readBlob;
		}
	}
	
	this.args = args;
	this.handler = handler;
	
	this.createFromStream = function(msgstream)
	{
		if(this.args.length <= 0)
			return msgstream;
		
		var result = new Array(this.args.length);
		for(i=0; i<this.args.length; i++)
		{
			result[i] = this.args[i].call(msgstream);
		}
		
		return result;
	}
	
	this.handleMessage = function(msgstream)
	{
		if(this.handler == null)
		{
			console.error("KBE_MESSAGE::handleMessage: interface(" + this.name + ") no implement!");  
			return;
		}

		if(this.args.length <= 0)
			this.handler(msgstream);
		else
			this.handler.apply(g_kbengine, this.createFromStream(msgstream));
	}
}

// 上行消息
var g_messages = {};

g_messages["Loginapp_importClientMessages"] = new KBE_MESSAGE(5, "importClientMessages", 0, new Array(), null);
g_messages["Baseapp_importClientMessages"] = new KBE_MESSAGE(207, "importClientMessages", 0, new Array(), null);
g_messages["onImportClientMessages"] = new KBE_MESSAGE(518, "onImportClientMessages", -1, new Array(), null);

/*-----------------------------------------------------------------------------------------
												entity
-----------------------------------------------------------------------------------------*/
function KBE_ENTITY()
{
	this.id = 0;
	this.classtype = "";
	this.position = [0.0, 0.0, 0.0];
	this.direction = [0.0, 0.0, 0.0];
	this.velocity = 0.0
		
	this.cell = null;
	this.base = null;
}

/*-----------------------------------------------------------------------------------------
												system
-----------------------------------------------------------------------------------------*/
function KBENGINE()
{
	this.username = "kbengine";
	this.password = "123456";
	this.loginappMessageImported = false;
	this.baseappMessageImported = false;
		
	this.reset = function()
	{  
		this.socket = null;
		this.ip = "127.0.0.1";
		this.port = 20013;
		this.currserver = "loginapp";
		this.serverdatas = "";
		this.clientdatas = "";
		this.serverVersion = "";
		this.clientVersion = "0.0.1";
		this.entity_uuid = null;
		this.entity_id = 0;
		this.entity_type = "";
		this.entities = {};
	}
	
	this.reset();
	
	this.hello = function()
	{  
		var bundle = new KBE_BUNDLE();
		if(g_kbengine.currserver == "loginapp")
			bundle.newMessage(g_messages.Loginapp_hello);
		else
			bundle.newMessage(g_messages.Baseapp_hello);
		bundle.writeString(g_kbengine.clientVersion);
		bundle.writeBlob(g_kbengine.clientdatas);
		bundle.send(g_kbengine);
	}

	this.connect = function(addr)
	{
		try{  
			if(g_kbengine.socket != null)
				g_kbengine.socket.close();
		}
		catch(e){ 
		}
		
		g_kbengine.socket = null;
		try{  
			g_kbengine.socket = new WebSocket(addr);  
		}catch(e){  
			console.error('WebSocket init error!');  
			return;  
		}
		
		g_kbengine.socket.binaryType = "arraybuffer";
		g_kbengine.socket.onopen = g_kbengine.onopen;  
		g_kbengine.socket.onerror = g_kbengine.onerror;  
		g_kbengine.socket.onmessage = g_kbengine.onmessage;  
		g_kbengine.socket.onclose = g_kbengine.onclose;
	}

	this.onopen = function(){  
		console.info('connect success!') ; 
	}

	this.onerror = function(evt){  
		console.error('connect error:' + evt.data);
	}
	
	this.onmessage = function(msg)
	{ 
		var stream = new KBE_MEMORYSTREAM(msg.data);
		var msgid = stream.readUint16();
		var msgHandler = g_messages[msgid];
		
		if(!msgHandler)
		{
			console.error("KBENGINE::onmessage: not found msg(" + msgid + ")!");
		}
		else
		{
			var msglen = msgHandler.length;
			if(msglen == -1)
				msglen = stream.readUint16();
			
			msgHandler.handleMessage(stream);
		}
	}  

	this.onclose = function(){  
		console.info('connect close:' + g_kbengine.currserver);
		
		if(g_kbengine.currserver != "loginapp")
			g_kbengine.reset();
	}

	this.send = function(msg)
	{
		g_kbengine.socket.send(msg);
	}

	this.close = function(){  
		g_kbengine.socket.close();  
		g_kbengine.reset();
	}
	
	this.onOpenLoginapp = function()
	{  
		console.info("KBENGINE::onOpenLoginapp: successfully!");
		g_kbengine.currserver = "loginapp";
		
		if(!g_kbengine.loginappMessageImported)
		{
			var bundle = new KBE_BUNDLE();
			bundle.newMessage(g_messages.Loginapp_importClientMessages);
			bundle.send(g_kbengine);
			g_kbengine.socket.onmessage = g_kbengine.onImportClientMessages;  
		}
		else
		{
			g_kbengine.onImportClientMessagesCompleted();
		}
	}
	
	this.onImportClientMessagesCompleted = function()
	{
		g_kbengine.socket.onmessage = g_kbengine.onmessage; 
		g_kbengine.hello();
			
		if(g_kbengine.currserver == "loginapp")
		{
			g_kbengine.login_loginapp(false);
			g_kbengine.loginappMessageImported = true;
		}
		else
		{
			g_kbengine.login_baseapp(false);
			g_kbengine.baseappMessageImported = true;
		}
	}
	
	this.onImportClientMessages = function(msg)
	{
		var stream = new KBE_MEMORYSTREAM(msg.data);
		var msgid = stream.readUint16();

		if(msgid == g_messages.onImportClientMessages.id)
		{
			var msglen = stream.readUint16();
			var msgcount = stream.readUint16();
			
			while(msgcount > 0)
			{
				msgcount--;
				
				msgid = stream.readUint16();
				msglen = stream.readInt16();
				var msgname = stream.readString();
				var argsize = stream.readUint8();
				var argstypes = new Array(argsize);
				
				for(var i=0; i<argsize; i++)
				{
					argstypes[i] = stream.readUint8();
				}
				
				var handler = null;
				
				if(msgname.indexOf("Client_") >= 0)
				{
					handler = g_kbengine[msgname];
					if(handler == null || handler == undefined)
					{
						console.warn("KBENGINE::onImportClientMessages: interface(" + msgname + ") no implement!");
						handler = null;
					}
					else
					{
						console.info("KBENGINE::onImportClientMessages: import(" + msgname + ") successfully!");
					}
				}
			
				if(msgname.length > 0)
				{
					g_messages[msgname] = new KBE_MESSAGE(msgid, msgname, msglen, argstypes, handler);
					g_messages[msgid] = g_messages[msgname];
				}
				else
				{
					g_messages[msgid] = new KBE_MESSAGE(msgid, msgname, msglen, argstypes, handler);
				}
				
			}

			g_kbengine.onImportClientMessagesCompleted();
		}
		else
			console.error("KBENGINE::onmessage: not found msg(" + msgid + ")!");
	}
	
	this.login_loginapp = function(noconnect)
	{  
		if(noconnect)
		{
			console.info("KBENGINE::login_loginapp: start connect to ws://" + g_kbengine.ip + ":" + g_kbengine.port + "!");
			g_kbengine.connect("ws://" + g_kbengine.ip + ":" + g_kbengine.port);
			g_kbengine.socket.onopen = g_kbengine.onOpenLoginapp;  
		}
		else
		{
			var bundle = new KBE_BUNDLE();
			bundle.newMessage(g_messages.Loginapp_login);
			bundle.writeInt8(3); // clientType
			bundle.writeBlob("");
			bundle.writeString(g_kbengine.username);
			bundle.writeString(g_kbengine.password);
			bundle.send(g_kbengine);
		}
	}

	this.onOpenBaseapp = function()
	{
		console.info("KBENGINE::onOpenBaseapp: successfully!");
		g_kbengine.currserver = "baseapp";
		
		if(!g_kbengine.baseappMessageImported)
		{
			var bundle = new KBE_BUNDLE();
			bundle.newMessage(g_messages.Baseapp_importClientMessages);
			bundle.send(g_kbengine);
			g_kbengine.socket.onmessage = g_kbengine.onImportClientMessages;  
		}
		else
		{
			g_kbengine.onImportClientMessagesCompleted();
		}
	}
	
	this.login_baseapp = function(noconnect)
	{  
		if(noconnect)
		{
			console.info("KBENGINE::login_baseapp: start connect to ws://" + g_kbengine.ip + ":" + g_kbengine.port + "!");
			g_kbengine.connect("ws://" + g_kbengine.ip + ":" + g_kbengine.port);
			g_kbengine.socket.onopen = g_kbengine.onOpenBaseapp;  
		}
		else
		{
			var bundle = new KBE_BUNDLE();
			bundle.newMessage(g_messages.Baseapp_loginGateway);
			bundle.writeString(g_kbengine.username);
			bundle.writeString(g_kbengine.password);
			bundle.send(g_kbengine);
		}
	}
	
	this.Client_onHelloCB = function(args)
	{
		g_kbengine.serverVersion = args.readString();
		var ctype = args.readInt32();
		console.info("KBENGINE::Client_onHelloCB: verInfo(" + g_kbengine.serverVersion + "), ctype(" + ctype + ")!");
	}
	
	this.Client_onLoginFailed = function(args)
	{
		var failedcode = args.readUint16();
		g_kbengine.serverdatas = args.readBlob();
		console.error("KBENGINE::Client_onLoginFailed: failedcode(" + failedcode + "), datas(" + g_kbengine.serverdatas.length + ")!");
	}
	
	this.Client_onLoginSuccessfully = function(args)
	{
		var accountName = args.readString();
		g_kbengine.username = accountName;
		g_kbengine.ip = args.readString();
		g_kbengine.port = args.readUint16();
		g_kbengine.serverdatas = args.readBlob();
		
		console.info("KBENGINE::Client_onLoginSuccessfully: accountName(" + accountName + "), addr(" + 
				g_kbengine.ip + ":" + g_kbengine.port + "), datas(" + g_kbengine.serverdatas.length + ")!");
		
		g_kbengine.login_baseapp(true);
	}
	
	this.Client_onLoginGatewayFailed = function(args)
	{
		var failedcode = args.readUint16();
		console.error("KBENGINE::Client_onLoginGatewayFailed: failedcode(" + failedcode + ")!");
	}
	
	this.Client_onCreatedProxies = function(rndUUID, eid, entityType)
	{
		console.info("KBENGINE::Client_onCreatedProxies: eid(" + eid + "), entityType(" + entityType + ")!");
		g_kbengine.entity_uuid = rndUUID;
		g_kbengine.entity_id = eid;
		
		var entity = new KBE_ENTITY();
		entity.id = eid;
		entity.classtype = entityType;
		
		g_kbengine.entities[eid] = entity;
	}
}

var g_kbengine = new KBENGINE();

