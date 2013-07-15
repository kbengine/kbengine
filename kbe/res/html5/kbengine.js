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
			
			if(this.rpos + i >= this.buffer.byteLength)
				throw(new Error("KBE_MEMORYSTREAM::readString: rpos(" + (this.rpos + i) + ")>=" + 
					this.buffer.byteLength + " overflow!"));
		}
		
		this.rpos += i;
		return s;
	}

	this.readBlob = function()
	{
		size = this.readUint32();
		var buf = new Uint8Array(this.buffer, this.rpos, size);
		this.rpos += size;
		return buf;
	}

	this.readStream = function()
	{
		var buf = new Uint8Array(this.buffer, this.rpos, this.buffer.byteLength - this.rpos);
		this.rpos = this.buffer.byteLength;
		return new KBE_MEMORYSTREAM(buf);
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
	this.readEOF = function()
	{
		return this.buffer.byteLength - this.rpos <= 0;
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
var g_reader = new KBE_MEMORYSTREAM(0);
var datatype2id = {};
datatype2id["STRING"] = 1;
datatype2id["STD::STRING"] = 1;

datatype2id["UINT8"] = 2;
datatype2id["BOOL"] = 2;
datatype2id["DATATYPE"] = 2;
datatype2id["CHAR"] = 2;
datatype2id["DETAIL_TYPE"] = 2;
datatype2id["MAIL_TYPE"] = 2;

datatype2id["UINT16"] = 3;
datatype2id["UNSIGNED SHORT"] = 3;
datatype2id["SERVER_ERROR_CODE"] = 3;
datatype2id["ENTITY_TYPE"] = 3;
datatype2id["ENTITY_PROPERTY_UID"] = 3;
datatype2id["ENTITY_METHOD_UID"] = 3;
datatype2id["ENTITY_SCRIPT_UID"] = 3;
datatype2id["DATATYPE_UID"] = 3;

datatype2id["UINT32"] = 4;
datatype2id["UINT"] = 4;
datatype2id["UNSIGNED INT"] = 4;
datatype2id["ARRAYSIZE"] = 4;
datatype2id["SPACE_ID"] = 4;
datatype2id["GAME_TIME"] = 4;
datatype2id["TIMER_ID"] = 4;

datatype2id["UINT64"] = 5;
datatype2id["DBID"] = 5;
datatype2id["COMPONENT_ID"] = 5;

datatype2id["INT8"] = 6;
datatype2id["COMPONENT_ORDER"] = 6;

datatype2id["INT16"] = 7;
datatype2id["SHORT"] = 7;

datatype2id["INT32"] = 8;
datatype2id["INT"] = 8;
datatype2id["ENTITY_ID"] = 8;
datatype2id["CALLBACK_ID"] = 8;
datatype2id["COMPONENT_TYPE"] = 8;

datatype2id["INT64"] = 9;

datatype2id["PYTHON"] = 10;
datatype2id["PY_DICT"] = 10;
datatype2id["PY_TUPLE"] = 10;
datatype2id["PY_LIST"] = 10;
datatype2id["MAILBOX"] = 10;

datatype2id["BLOB"] = 11;

datatype2id["UNICODE"] = 12;

datatype2id["FLOAT"] = 13;

datatype2id["DOUBLE"] = 14;

datatype2id["VECTOR2"] = 15;

datatype2id["VECTOR3"] = 16;

datatype2id["VECTOR4"] = 17;

datatype2id["FIXED_DICT"] = 18;

datatype2id["ARRAY"] = 19;


bindwriter = function(writer, argType)
{
	if(argType == datatype2id["UINT8"])
	{
		return writer.writeUint8;
	}
	else if(argType == datatype2id["UINT16"])
	{
		return writer.writeUint16;
	}
	else if(argType == datatype2id["UINT32"])
	{
		return writer.writeUint32;
	}
	else if(argType == datatype2id["UINT64"])
	{
		return writer.writeUint64;
	}
	else if(argType == datatype2id["INT8"])
	{
		return writer.writeInt8;
	}
	else if(argType == datatype2id["INT16"])
	{
		return writer.writeInt16;
	}
	else if(argType == datatype2id["INT32"])
	{
		return writer.writeInt32;
	}
	else if(argType == datatype2id["INT64"])
	{
		return writer.writeInt64;
	}
	else if(argType == datatype2id["FLOAT"])
	{
		return writer.writeFloat;
	}
	else if(argType == datatype2id["DOUBLE"])
	{
		return writer.writeDouble;
	}
	else if(argType == datatype2id["STRING"])
	{
		return writer.writeString;
	}
	else if(argType == datatype2id["FIXED_DICT"])
	{
		return writer.writeStream;
	}
	else if(argType == datatype2id["ARRAY"])
	{
		return writer.writeStream;
	}
	else
	{
		return writer.writeStream;
	}
}

bindReader = function(argType)
{
	if(argType == datatype2id["UINT8"])
	{
		return g_reader.readUint8;
	}
	else if(argType == datatype2id["UINT16"])
	{
		return g_reader.readUint16;
	}
	else if(argType == datatype2id["UINT32"])
	{
		return g_reader.readUint32;
	}
	else if(argType == datatype2id["UINT64"])
	{
		return g_reader.readUint64;
	}
	else if(argType == datatype2id["INT8"])
	{
		return g_reader.readInt8;
	}
	else if(argType == datatype2id["INT16"])
	{
		return g_reader.readInt16;
	}
	else if(argType == datatype2id["INT32"])
	{
		return g_reader.readInt32;
	}
	else if(argType == datatype2id["INT64"])
	{
		return g_reader.readInt64;
	}
	else if(argType == datatype2id["FLOAT"])
	{
		return g_reader.readFloat;
	}
	else if(argType == datatype2id["DOUBLE"])
	{
		return g_reader.readDouble;
	}
	else if(argType == datatype2id["STRING"])
	{
		return g_reader.readString;
	}
	else if(argType == datatype2id["FIXED_DICT"])
	{
		return g_reader.readStream;
	}
	else if(argType == datatype2id["ARRAY"])
	{
		return g_reader.readStream;
	}
	else
	{
		return g_reader.readStream;
	}
}
	
function KBE_MESSAGE(id, name, length, args, handler)
{
	this.id = id;
	this.name = name;
	this.length = length;

	// 绑定执行
	for(i=0; i<args.length; i++)
	{
		args[i] = bindReader(args[i]);
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
g_messages["Baseapp_importClientEntityDef"] = new KBE_MESSAGE(208, "importClientEntityDef", 0, new Array(), null);
g_messages["onImportClientMessages"] = new KBE_MESSAGE(518, "onImportClientMessages", -1, new Array(), null);

/*-----------------------------------------------------------------------------------------
												entity
-----------------------------------------------------------------------------------------*/
function KBEENTITY()
{
	this.id = 0;
	this.classtype = "";
	this.position = [0.0, 0.0, 0.0];
	this.direction = [0.0, 0.0, 0.0];
	this.velocity = 0.0
		
	this.cell = null;
	this.base = null;
	
	KBEENTITY.prototype.baseCall = function()
	{
		if(arguments.length < 1)
		{
			console.error('KBEENTITY::baseCall: not fount interfaceName!');  
			return;
		}
		
		var method = g_moduledefs[this.classtype].base_methods[arguments[0]];
		var methodID = method[0];
		var args = method[2];
		
		if(arguments.length - 1 != args.length)
		{
			console.error("KBEENTITY::baseCall: args(" + (arguments.length - 1) + "!= " + args.length + ") size is error!");  
			return;
		}
		
		this.base.newMail();
		this.base.bundle.writeUint16(methodID);
		
		try
		{
			for(var i=0; i<args.length; i++)
			{
				bindwriter(args[i])(arguments[i + 1]);
			}
		}
		catch(e)
		{
			console.error('KBEENTITY::baseCall: args is error!');  
			this.base.bundle = null;
			return;
		}
		
		this.base.postMail();
	}
	
	KBEENTITY.prototype.cellCall = function()
	{
		if(arguments.length < 1)
		{
			console.error('KBEENTITY::cellCall: not fount interfaceName!');  
			return;
		}
		
		var method = g_moduledefs[this.classtype].base_methods[arguments[0]];
		var methodID = method[0];
		var args = method[2];
		
		if(arguments.length - 1 != args.length)
		{
			console.error("KBEENTITY::cellCall: args(" + (arguments.length - 1) + "!= " + args.length + ") size is error!");  
			return;
		}
		
		this.cell.newMail();
		this.cell.bundle.writeUint16(methodID);
		
		try
		{
			for(var i=0; i<args.length; i++)
			{
				bindwriter(args[i])(arguments[i + 1]);
			}
		}
		catch(e)
		{
			console.error('KBEENTITY::cellCall: args is error!');  
			this.cell.bundle = null;
			return;
		}
		
		this.cell.postMail();
	}
}

/*-----------------------------------------------------------------------------------------
												entity
-----------------------------------------------------------------------------------------*/
var MAILBOX_TYPE_CELL = 0;
var MAILBOX_TYPE_BASE = 1;

function KBEMAILBOX()
{
	this.id = 0;
	this.classtype = "";
	this.type = MAILBOX_TYPE_CELL;
	this.networkInterface = g_kbengine;
	
	this.bundle = null;
	
	this.isBase = function()
	{
		return this.type == MAILBOX_TYPE_BASE;
	}

	this.isCell = function()
	{
		return this.type == MAILBOX_TYPE_CELL;
	}
	
	this.newMail = function()
	{  
		if(this.bundle == null)
			this.bundle = new KBE_BUNDLE();
		
		if(this.type == MAILBOX_TYPE_CELL)
			this.bundle.newMessage(g_messages.Baseapp_onRemoteCallCellMethodFromClient);
		else
			this.bundle.newMessage(g_messages.Base_onRemoteMethodCall);

		this.bundle.writeInt32(this.id);
		
		return this.bundle;
	}
	
	this.postMail = function(bundle)
	{
		if(bundle == undefined)
			bundle = this.bundle;
		
		bundle.send(this.networkInterface);
		this.bundle = null;
	}
}

/*-----------------------------------------------------------------------------------------
												entitydef
-----------------------------------------------------------------------------------------*/
var g_moduledefs = {}

/*-----------------------------------------------------------------------------------------
												system
-----------------------------------------------------------------------------------------*/
function KBENGINE()
{
	this.username = "kbengine";
	this.password = "123456";
	this.loginappMessageImported = false;
	this.baseappMessageImported = false;
	this.entitydefImported = false;
	
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
		
		//if(g_kbengine.currserver != "loginapp")
		//	g_kbengine.reset();
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
			g_kbengine.socket.onmessage = g_kbengine.Client_onImportClientMessages;  
			console.info("KBENGINE::onOpenLoginapp: start importClientMessages ...");
		}
		else
		{
			g_kbengine.onImportClientMessagesCompleted();
		}
	}
	
	this.onImportClientMessagesCompleted = function()
	{
		console.info("KBENGINE::onImportClientMessagesCompleted: successfully!");
		g_kbengine.socket.onmessage = g_kbengine.onmessage; 
		g_kbengine.hello();
		
		if(g_kbengine.currserver == "loginapp")
		{
			g_kbengine.login_loginapp(false);
			g_kbengine.loginappMessageImported = true;
		}
		else
		{
			g_kbengine.baseappMessageImported = true;
			
			if(!g_kbengine.entitydefImported)
			{
				console.info("KBENGINE::onImportClientMessagesCompleted: start importEntityDef ...");
				var bundle = new KBE_BUNDLE();
				bundle.newMessage(g_messages.Baseapp_importClientEntityDef);
				bundle.send(g_kbengine);
			}
			else
			{
				g_kbengine.onImportEntityDefCompleted();
			}
		}
	}
	
	this.Client_onImportClientEntityDef = function(stream)
	{
		while(!stream.readEOF())
		{
			var scriptmethod_name = stream.readString();
			var propertysize = stream.readUint16();
			var methodsize = stream.readUint16();
			var base_methodsize = stream.readUint16();
			var cell_methodsize = stream.readUint16();
			
			console.info("KBENGINE::Client_onImportClientEntityDef: import(" + scriptmethod_name + "), propertys(" + propertysize + "), " +
					"clientMethods(" + methodsize + "), baseMethods(" + base_methodsize + "), cellMethods(" + cell_methodsize + ")!");
			
			g_moduledefs[scriptmethod_name] = {};
			g_moduledefs[scriptmethod_name]["propertys"] = {};
			g_moduledefs[scriptmethod_name]["methods"] = {};
			g_moduledefs[scriptmethod_name]["base_methods"] = {};
			g_moduledefs[scriptmethod_name]["cell_methods"] = {};
			
			while(propertysize > 0)
			{
				propertysize--;
				
				var properUtype = stream.readUint16();
				var name = stream.readString();
				var defaultValStr = stream.readString();
				var utype = bindReader(stream.readUint8());
				
				g_moduledefs[scriptmethod_name]["propertys"][name] = [properUtype, name, defaultValStr, utype];
				console.info("KBENGINE::Client_onImportClientEntityDef: add(" + scriptmethod_name + "), property(" + name + ").");
			};
			
			while(methodsize > 0)
			{
				methodsize--;
				
				var methodUtype = stream.readUint16();
				var name = stream.readString();
				var argssize = stream.readUint8();
				var args = [];
				
				while(argssize > 0)
				{
					argssize--;
					args.push(bindReader(stream.readUint8()));
				};
				
				var savedata = [methodUtype, name, args];
				g_moduledefs[scriptmethod_name]["methods"][name] = savedata;
				g_moduledefs[scriptmethod_name]["methods"][methodUtype] = savedata;
				console.info("KBENGINE::Client_onImportClientEntityDef: add(" + scriptmethod_name + "), method(" + name + ").");
			};

			while(base_methodsize > 0)
			{
				base_methodsize--;
				
				var methodUtype = stream.readUint16();
				var name = stream.readString();
				var argssize = stream.readUint8();
				var args = [];
				
				while(argssize > 0)
				{
					argssize--;
					args.push(stream.readUint8());
				};
				
				g_moduledefs[scriptmethod_name]["base_methods"][name] = [methodUtype, name, args];
				console.info("KBENGINE::Client_onImportClientEntityDef: add(" + scriptmethod_name + "), base_method(" + name + ").");
			};
			
			while(cell_methodsize > 0)
			{
				cell_methodsize--;
				
				var methodUtype = stream.readUint16();
				var name = stream.readString();
				var argssize = stream.readUint8();
				var args = [];
				
				while(argssize > 0)
				{
					argssize--;
					args.push(stream.readUint8());
				};
				
				g_moduledefs[scriptmethod_name]["cell_methods"][name] = [methodUtype, name, args];
				console.info("KBENGINE::Client_onImportClientEntityDef: add(" + scriptmethod_name + "), cell_method(" + name + ").");
			};
			
			try
			{
				defmethod = eval("KBE" + scriptmethod_name);
			}
			catch(e)
			{
				console.error("KBENGINE::Client_onImportClientEntityDef: module(KBE" + scriptmethod_name + ") not found!");
				defmethod = undefined;
			}
			
			for(name in g_moduledefs[scriptmethod_name].propertys)
			{
				var infos = g_moduledefs[scriptmethod_name].propertys[name];
				var properUtype = infos[0];
				var name = infos[1];
				var defaultValStr = infos[2];
				var utype = infos[3];
				
				if(defmethod != undefined)
					defmethod.prototype[name] = eval(defaultValStr);
			};

			for(name in g_moduledefs[scriptmethod_name].methods)
			{
				var infos = g_moduledefs[scriptmethod_name].methods[name];
				var properUtype = infos[0];
				var name = infos[1];
				var args = infos[2];
				
				if(defmethod != undefined && defmethod.prototype[name] == undefined)
				{
					console.warn("KBE" + scriptmethod_name + ":: method(" + name + ") no implement!");
				}
			};
		}
		
		g_kbengine.onImportEntityDefCompleted();
	}

	this.onImportEntityDefCompleted = function()
	{
		console.info("KBENGINE::onImportEntityDefCompleted: successfully!");
		g_kbengine.entitydefImported = true;
		g_kbengine.login_baseapp(false);
	}
	
	this.Client_onImportClientMessages = function(msg)
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
						// console.info("KBENGINE::onImportClientMessages: import(" + msgname + ") successfully!");
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
			};

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
			g_kbengine.socket.onmessage = g_kbengine.Client_onImportClientMessages;  
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
	
	this.Client_onLoginGatewayFailed = function(failedcode)
	{
		console.error("KBENGINE::Client_onLoginGatewayFailed: failedcode(" + failedcode + ")!");
	}
	
	this.entityclass = {};
	this.getentityclass = function(entityType)
	{
		entityType = "KBE" + entityType;
		var runclass = g_kbengine.entityclass[entityType];
		if(runclass == undefined)
		{
			runclass = eval(entityType);
			if(runclass == undefined)
			{
				console.error("KBENGINE::getentityclass: entityType(" + entityType + ") is error!");
				return runclass;
			}
			else
				g_kbengine.entityclass[entityType] = runclass;
		}

		return runclass;
	}
	
	this.Client_onCreatedProxies = function(rndUUID, eid, entityType)
	{
		console.info("KBENGINE::Client_onCreatedProxies: eid(" + eid + "), entityType(" + entityType + ")!");
		g_kbengine.entity_uuid = rndUUID;
		g_kbengine.entity_id = eid;
		
		var runclass = this.getentityclass(entityType);
		if(runclass == undefined)
			return;
		
		var entity = new runclass();
		entity.id = eid;
		entity.classtype = entityType;
		
		entity.base = new KBEMAILBOX();
		entity.base.id = eid;
		entity.base.classtype = entityType;
		entity.base.type = MAILBOX_TYPE_BASE;
		
		g_kbengine.entities[eid] = entity;
		
		entity.__init__();
	}
	
	this.Client_onUpdatePropertys = function(stream)
	{
		alert(stream);
		var eid = stream.readInt32();
		alert(eid);
	}

	this.Client_onRemoteMethodCall = function(stream)
	{
		var eid = stream.readInt32();
		var entity = g_kbengine.entities[eid];
		
		if(entity == undefined)
		{
			console.error("KBENGINE::Client_onRemoteMethodCall: entity(" + eid + ") not found!");
			return;
		}
		
		var methodUtype = stream.readUint16();
		var methoddata = g_moduledefs[entity.classtype].methods[methodUtype];
		var args = [];
		var argsdata = methoddata[2];
		for(var i=0; i<argsdata.length; i++)
		{
			args.push(argsdata[i].call(stream));
		}
		
		entity[methoddata[1]].apply(entity, args);
	}
		
	this.Client_onEntityEnterWorld = function(eid, scriptType, spaceID)
	{
	}
	
	this.Client_onEntityLeaveWorld = function(eid, spaceID)
	{
	}
	
	this.Client_onEntityEnterSpace = function(spaceID, eid)
	{
	}
	
	this.Client_onEntityLeaveSpace = function(spaceID, eid)
	{
	}

	this.Client_onSetEntityPosAndDir = function(stream)
	{
	}

	this.Client_onCreateAccountResult = function(stream)
	{
	}
	
	
}

var g_kbengine = new KBENGINE();

