/*-----------------------------------------------------------------------------------------
												global
-----------------------------------------------------------------------------------------*/
var PACKET_MAX_SIZE	= 1500
var PACKET_MAX_SIZE_TCP = 1460
var PACKET_MAX_SIZE_UDP = 1472

var MESSAGE_ID_LENGTH = 2
var MESSAGE_LENGTH_LENGTH = 2

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
			throw(new Error("memorystream::writeBlob: no free!"))
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
		
		this.wpos +=size;
	}
	
	this.writeString = function(v)
	{
		if(v.length > this.fillfree())
		{
			throw(new Error("memorystream::writeString: no free!"))
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
function KBE_MESSAGE(id, name, length, args)
{
	this.id = id;
	this.name = name;
	this.length = length;
	this.reader = new KBE_MEMORYSTREAM(0);

	// 绑定执行
	for(i=0; i<args.length; i++)
	{
		argName = args[i][0];
		argType = args[i][1];
		
		if(argType == "UINT8")
		{
			args[i][1] = this.reader.readUint8;
		}
		else if(argType == "UINT16")
		{
			args[i][1] = this.reader.readUint16;
		}
		else if(argType == "UINT32")
		{
			args[i][1] = this.reader.readUint32;
		}
		else if(argType == "UINT64")
		{
			args[i][1] = this.reader.readUint64;
		}
		else if(argType == "INT8")
		{
			args[i][1] = this.reader.readInt8;
		}
		else if(argType == "INT16")
		{
			args[i][1] = this.reader.readInt16;
		}
		else if(argType == "INT32")
		{
			args[i][1] = this.reader.readInt32;
		}
		else if(argType == "INT64")
		{
			args[i][1] = this.reader.readInt64;
		}
		else if(argType == "FLOAT")
		{
			args[i][1] = this.reader.readFloat;
		}
		else if(argType == "DOUBLE")
		{
			args[i][1] = this.reader.readDouble;
		}
		else if(argType == "STRING")
		{
			args[i][1] = this.reader.readString;
		}
		else
		{
			args[i][1] = this.reader.readBlob;
		}
	}
	
	this.args = args;
	this.handler = null;
	
	this.createFromStream = function(msgstream)
	{
		var result = new Array(this.args.length);
		for(i=0; i<this.args.length; i++)
		{
			result[i] = this.args[i][1].call(msgstream);
		}
		
		return result;
	}
	
	this.handleMessage = function(entity, msgstream)
	{
		this.handler.call(entity, createFromStream(msgstream));
	}
}

// 上行消息
var g_messages = {};

g_messages["Loginapp_importClientMessages"] = new KBE_MESSAGE(5, "importClientMessages", 0, new Array());
g_messages["Baseapp_importClientMessages"] = new KBE_MESSAGE(207, "importClientMessages", 0, new Array());
g_messages["onImportClientMessages"] = new KBE_MESSAGE(518, "onImportClientMessages", -1, new Array());

/*-----------------------------------------------------------------------------------------
												network
-----------------------------------------------------------------------------------------*/
function KBENGINE()
{
	this.socket;
	
	this.hello = function()
	{  
		var bundle = new KBE_BUNDLE();
		bundle.newMessage(g_messages.Loginapp_hello);
		bundle.writeString("0.0.1");
		bundle.writeBlob("");
		bundle.send(this);
	}

	this.connect = function(addr)
	{
		try{  
			this.socket = new WebSocket(addr);  
		}catch(e){  
			alert('WebSocket init error!');  
			return;  
		}
		
		this.socket.binaryType = "arraybuffer";
		this.socket.onopen = this.onopen;  
		this.socket.onerror = this.onerror;  
		this.socket.onmessage = this.onmessage;  
		this.socket.onclose = this.onclose;
	}

	this.onopen = function(){  
		alert('connect success!') ; 
	}

	this.onerror = function(evt){  
		alert('connect error:' + evt.data);
	}
	
	this.onmessage = function(msg)
	{ 
		var stream = new KBE_MEMORYSTREAM(msg.data);
		var msgid = stream.readUint16();
		var msgHandler = g_messages[msgid];
		
		if(!msgHandler)
		{
			alert("KBE_NETWORK::onmessage: not found msg(" + msgid + ")!");
		}
		else
		{
			var msglen = msgHandler.length;
			if(msglen == -1)
				msglen = stream.readUint16();
			
			alert(msgid + "_" + msgHandler.args.length+ "_"+ msgHandler.name);
			alert(msgHandler.createFromStream(stream));
		}
	}  

	this.onclose = function(){  
		alert('connect close');
	}

	this.send = function(msg)
	{
		this.socket.send(msg);
	}

	this.close = function(){  
		this.socket.close();  
	}
	
	this.onOpenLoginapp = function()
	{  
		var bundle = new KBE_BUNDLE();
		bundle.newMessage(g_messages.Loginapp_importClientMessages);
		bundle.send(g_kbengine);
		g_kbengine.socket.onmessage = g_kbengine.onImportClientMessages;  
	}
	
	this.onImportClientMessages = function(msg)
	{  
		var stream = new KBE_MEMORYSTREAM(msg.data);
		var msgid = stream.readUint16();

		if(msgid == g_messages.Loginapp_onImportClientMessages.id)
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
				
				if(msgname.length > 0)
				{
					g_messages[msgname] = new KBE_MESSAGE(msgid, msgname, msglen, argstypes);
					g_messages[msgid] = g_messages[msgname];
				}
				else
				{
					g_messages[msgid] = new KBE_MESSAGE(msgid, msgname, msglen, argstypes);
				}
				
			}
			
			g_kbengine.socket.onmessage = g_kbengine.onmessage; 
			g_kbengine.hello();
			g_kbengine.login_loginapp(false);
		}
		else
			alert("KBE_NETWORK::onmessage: not found msg(" + msgid + ")!");
	}
	
	this.login_loginapp = function(noconnect)
	{  
		if(noconnect)
		{
			g_kbengine.connect('ws://127.0.0.1:20013');
			g_kbengine.socket.onopen = g_kbengine.onOpenLoginapp;  
		}
		else
		{
			var bundle = new KBE_BUNDLE();
			bundle.newMessage(g_messages.Loginapp_login);
			bundle.writeInt8(3); // clientType
			bundle.writeBlob("");
			bundle.writeString("kbengine");
			bundle.writeString("123456");
			bundle.send(g_kbengine);
		}
	}
}

var g_kbengine = new KBENGINE();

