var KBEngine = KBEngine || {};

/*-----------------------------------------------------------------------------------------
					    	JavaScript Inheritance
-----------------------------------------------------------------------------------------*/
/* Simple JavaScript Inheritance for ES 5.1
 * based on http://ejohn.org/blog/simple-javascript-inheritance/
 *  (inspired by base2 and Prototype)
 * MIT Licensed.
 */

// The base Class implementation (does nothing)
KBEngine.Class = function(){};
// Create a new Class that inherits from this class
KBEngine.Class.extend = function(props) {
	var _super = this.prototype;
    var fnTest = /xyz/.test(function(){xyz;}) ? /\b_super\b/ : /.*/;
	// Set up the prototype to inherit from the base class
	// (but without running the ctor constructor)
	var proto = Object.create(_super);

	// Copy the properties over onto the new prototype
	for (var name in props) {
		// Check if we're overwriting an existing function
		proto[name] = typeof props[name] === "function" &&
		typeof _super[name] == "function" && fnTest.test(props[name])
			? (function(name, fn){
				return function() {
					var tmp = this._super;

					// Add a new ._super() method that is the same method
					// but on the super-class
					this._super = _super[name];

					// The method only need to be bound temporarily, so we
					// remove it when we're done executing
					var ret = fn.apply(this, arguments);
					this._super = tmp;

					return ret;
				};
			})(name, props[name])
			: props[name];
	}

	// The new constructor
	var newClass = typeof proto.ctor === "function"
		? proto.hasOwnProperty("ctor")
			? proto.ctor // All construction is actually done in the ctor method
			: function SubClass(){ _super.ctor.apply(this, arguments); }
		: function EmptyClass(){};

	// Populate our constructed prototype object
	newClass.prototype = proto;

	// Enforce the constructor to be what we expect
	proto.constructor = newClass;

	// And make this class extendable
	newClass.extend = KBEngine.Class.extend;

	return newClass;
};

/*
	如果ArrayBuffer没有transfer()的方法, 则为ArrayBuffer添加transfer()方法
	该方法回一个新的ArrayBuffer， 其内容取自oldBuffer的数据，并且根据 newByteLength 的大小来对数据进行截取
	参考:https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects/ArrayBuffer/transfer
 */
if(!ArrayBuffer.transfer) {
    ArrayBuffer.transfer = function (source, length) {
        source = Object(source);
		var dest = new ArrayBuffer(length);
		
        if(!(source instanceof ArrayBuffer) || !(dest instanceof ArrayBuffer)) {
            throw new TypeError("ArrayBuffer.transfer, error: Source and destination must be ArrayBuffer instances");
		}
		
        if(dest.byteLength >= source.byteLength) {
			var buf = new Uint8Array(dest);
			buf.set(new Uint8Array(source), 0);
		}
		else {
			throw new RangeError("ArrayBuffer.transfer, error: destination has not enough space");
		}
		
		return dest;
    };
};

// export
window.Class = KBEngine.Class;

/*-----------------------------------------------------------------------------------------
												global
-----------------------------------------------------------------------------------------*/
KBEngine.PACKET_MAX_SIZE		 = 1500;
KBEngine.PACKET_MAX_SIZE_TCP	 = 1460;
KBEngine.PACKET_MAX_SIZE_UDP	 = 1472;

KBEngine.MESSAGE_ID_LENGTH		 = 2;
KBEngine.MESSAGE_LENGTH_LENGTH	 = 2;
KBEngine.MESSAGE_LENGTH1_LENGTH  = 4;
KBEngine.MESSAGE_MAX_SIZE		 = 65535;

KBEngine.CLIENT_NO_FLOAT		 = 0;
KBEngine.KBE_FLT_MAX			 = 3.402823466e+38;

/*-----------------------------------------------------------------------------------------
												number64bits
-----------------------------------------------------------------------------------------*/
KBEngine.INT64 = function(lo, hi)
{
	this.lo = lo;
	this.hi = hi;
	
	this.sign = 1;
	
	if(hi >= 2147483648)
	{
		this.sign = -1;
		if(this.lo > 0)
		{
			this.lo = (4294967296 - this.lo) & 0xffffffff;
			this.hi = 4294967295 - this.hi;
		}
		else
		{
			this.lo = (4294967296 - this.lo) & 0xffffffff;
			this.hi = 4294967296 - this.hi;
		}
	}
	
	this.toString = function()
	{
		var result = "";
		
		if(this.sign < 0)
		{
			result += "-"
		}
		
		var low = this.lo.toString(16);
		var high = this.hi.toString(16);
		
		if(this.hi > 0)
		{
			result += high;
			for(var i = 8 - low.length; i > 0; --i)
			{
				result += "0";
			}
		}
		result += low;
		
		return result;
		
	}
}

KBEngine.UINT64 = function(lo, hi)
{
	this.lo = lo;
	this.hi = hi;
	
	this.toString = function()
	{
		var low = this.lo.toString(16);
		var high = this.hi.toString(16);
		
		var result = "";
		if(this.hi > 0)
		{
			result += high;
			for(var i = 8 - low.length; i > 0; --i)
			{
				result += "0";
			}
		}
		result += low;
		return result;
	}
}

/*-----------------------------------------------------------------------------------------
												debug
-----------------------------------------------------------------------------------------*/
KBEngine.INFO_MSG = function(s)
{
	console.info(s);
}

KBEngine.DEBUG_MSG = function(s)
{
	console.debug(s);
}

KBEngine.ERROR_MSG = function(s)
{
	console.error(s);
}

KBEngine.WARNING_MSG = function(s)
{
	console.warn(s);
}

/*-----------------------------------------------------------------------------------------
												string
-----------------------------------------------------------------------------------------*/
KBEngine.utf8ArrayToString = function(array)
{
    var out, i, len, c;
    var char2, char3;

    out = "";
    len = array.length;
    i = 0;

    while(i < len)
    {
        c = array[i++];

        switch(c >> 4)
        {
            case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
            // 0xxxxxxx
            out += String.fromCharCode(c);
            break;
            case 12: case 13:
            // 110x xxxx   10xx xxxx
            char2 = array[i++];
            out += String.fromCharCode(((c & 0x1F) << 6) | (char2 & 0x3F));
            break;
            case 14:
                // 1110 xxxx  10xx xxxx  10xx xxxx
                char2 = array[i++];
                char3 = array[i++];
                out += String.fromCharCode(((c & 0x0F) << 12) |
                    ((char2 & 0x3F) << 6) |
                    ((char3 & 0x3F) << 0));
                break;
        }
    }
    
    return out;
}

KBEngine.stringToUTF8Bytes = function(str) 
{
    var utf8 = [];
    for (var i=0; i < str.length; i++) {
        var charcode = str.charCodeAt(i);
        if (charcode < 0x80) utf8.push(charcode);
        else if (charcode < 0x800) {
            utf8.push(0xc0 | (charcode >> 6), 
                      0x80 | (charcode & 0x3f));
        }
        else if (charcode < 0xd800 || charcode >= 0xe000) {
            utf8.push(0xe0 | (charcode >> 12), 
                      0x80 | ((charcode>>6) & 0x3f), 
                      0x80 | (charcode & 0x3f));
        }
        // surrogate pair
        else {
            i++;
            // UTF-16 encodes 0x10000-0x10FFFF by
            // subtracting 0x10000 and splitting the
            // 20 bits of 0x0-0xFFFFF into two halves
            charcode = 0x10000 + (((charcode & 0x3ff)<<10)
                      | (str.charCodeAt(i) & 0x3ff))
            utf8.push(0xf0 | (charcode >>18), 
                      0x80 | ((charcode>>12) & 0x3f), 
                      0x80 | ((charcode>>6) & 0x3f), 
                      0x80 | (charcode & 0x3f));
        }
    }
    return utf8;
}

/*-----------------------------------------------------------------------------------------
												event
-----------------------------------------------------------------------------------------*/
KBEngine.EventInfo = function(classinst, callbackfn)
{
	this.callbackfn = callbackfn;
	this.classinst = classinst;
}

KBEngine.FiredEvent = function(evtName, evtInfo, ars)
{
	this.evtName = evtName;
	this.evtInfo = evtInfo;
	this.ars = ars;
}

KBEngine.Event = function()
{
	this._events = {};
	this._isPause = false;
	this._firedEvents = [];
	
	this.register = function(evtName, classinst, strCallback)
	{
		var callbackfn = classinst[strCallback];
		if(callbackfn == undefined)
		{
			KBEngine.ERROR_MSG('KBEngine.Event::fire: not found strCallback(' + classinst  + ")!"+strCallback);  
			return;
		}

		var evtlst = this._events[evtName];
		if(evtlst == undefined)
		{
			evtlst = [];
			this._events[evtName] = evtlst;
		}
		
		var info = new KBEngine.EventInfo(classinst, callbackfn);
		evtlst.push(info);
	}

	this.deregisterAll = function(classinst)
	{
		for(var itemkey in this._events)
		{
			this.deregister(itemkey, classinst);
		}
	}
	
	this.deregister = function(evtName, classinst)
	{
		var evtlst = this._events[evtName];

		if(evtlst == undefined)
		{
			return;
		}

		while(true)
		{
			var found = false;
			for(var i=0; i<evtlst.length; i++)
			{
				var info = evtlst[i];
				if(info.classinst == classinst)
				{
					evtlst.splice(i, 1);
					found = true;
					break;
				}
			}
			
			if(!found)
				break;
		}

		this.removeFiredEvent(evtName, classinst);
	}

	this.removeAllFiredEvent = function(classinst)
	{
		this.removeFiredEvent("", classinst);
	}

	this.removeFiredEvent = function(evtName, classinst)
	{
		var firedEvents = this._firedEvents;
		while(true)
		{
			var found = false;
			for(var i=0; i<firedEvents.length; i++)
			{
				var evt = firedEvents[i];
				if((evtName == "" || evt.evtName == evtName) && evt.evtInfo.classinst == classinst)
				{
					firedEvents.splice(i, 1);
					found = true;
					break;
				}
			}

			if(!found)
				break;
		}
	}
	
	this.fire = function()
	{
		if(arguments.length < 1)
		{
			//KBEngine.ERROR_MSG('KBEngine.Event::fire: not found eventName!');  
			return;
		}

		var evtName = arguments[0];
		var evtlst = this._events[evtName];
		
		if(evtlst == undefined)
		{
			return;			
		}
		
		var ars = [];
		for(var i=1; i<arguments.length; i++) 
			ars.push(arguments[i]);
		
		for(var i=0; i<evtlst.length; i++)
		{
			var info = evtlst[i];

			if(!this._isPause)
			{
				if(ars.length < 1)
				{
					info.callbackfn.apply(info.classinst);
				}
				else
				{
					info.callbackfn.apply(info.classinst, ars);
				}
			}
			else
			{
				var eobj = new KBEngine.FiredEvent(evtName, info, ars);
				this._firedEvents.push(eobj);
			}
		}
	}

	this.pause = function() 
	{
		this._isPause = true;
	}

	this.resume = function()
	{
		this._isPause = false;

		var firedEvents = this._firedEvents;
		while(firedEvents.length > 0)
		{
			var evt = firedEvents.shift();
			var info = evt.evtInfo;
			var ars = evt.ars;

			if(ars.length < 1)
			{
				info.callbackfn.apply(info.classinst);
			}
			else
			{
				info.callbackfn.apply(info.classinst, ars);
			}
		}
	}

	this.clear = function()
	{
		this._events = {};
		this._firedEvents.splice(0, this._firedEvents.length);
	}
}

KBEngine.Event = new KBEngine.Event();

/*-----------------------------------------------------------------------------------------
												memorystream
-----------------------------------------------------------------------------------------*/

/*
	union PackFloatXType
	{
		float	fv;
		uint32	uv;
		int		iv;
	};	
*/
KBEngine.PackFloatXType = function()
{
	this._unionData = new ArrayBuffer(4);
	this.fv = new Float32Array(this._unionData, 0, 1);
	this.uv = new Uint32Array(this._unionData, 0, 1);
	this.iv = new Int32Array(this._unionData, 0, 1);
};

KBEngine._xPackData = new KBEngine.PackFloatXType();
KBEngine._yPackData = new KBEngine.PackFloatXType();
KBEngine._zPackData = new KBEngine.PackFloatXType();

KBEngine.MemoryStream = function(size_or_buffer)
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
		var lo = this.readInt32();
		var hi = this.readInt32();
		return new KBEngine.INT64(lo, hi);
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
		var lo = this.readUint32();
		var hi = this.readUint32();
		return new KBEngine.UINT64(lo, hi);
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
				throw(new Error("KBEngine.MemoryStream::readString: rpos(" + (this.rpos + i) + ")>=" + 
					this.buffer.byteLength + " overflow!"));
		}
		
		this.rpos += i;
		return s;
	}

	this.readBlob = function()
	{
		var size = this.readUint32();
		var buf = new Uint8Array(this.buffer, this.rpos, size);
		this.rpos += size;
		return buf;
	}

	this.readStream = function()
	{
		var buf = new Uint8Array(this.buffer, this.rpos, this.buffer.byteLength - this.rpos);
		this.rpos = this.buffer.byteLength;
		return new KBEngine.MemoryStream(buf);
	}
	
	this.readPackXZ = function()
	{
		var xPackData = KBEngine._xPackData;
		var zPackData = KBEngine._zPackData;
		
		xPackData.fv[0] = 0.0;
		zPackData.fv[0] = 0.0;

		xPackData.uv[0] = 0x40000000;
		zPackData.uv[0] = 0x40000000;
			
		var v1 = this.readUint8();
		var v2 = this.readUint8();
		var v3 = this.readUint8();

		var data = 0;
		data |= (v1 << 16);
		data |= (v2 << 8);
		data |= v3;

		xPackData.uv[0] |= (data & 0x7ff000) << 3;
		zPackData.uv[0] |= (data & 0x0007ff) << 15;

		xPackData.fv[0] -= 2.0;
		zPackData.fv[0] -= 2.0;
	
		xPackData.uv[0] |= (data & 0x800000) << 8;
		zPackData.uv[0] |= (data & 0x000800) << 20;
			
		var data = new Array(2);
		data[0] = xPackData.fv[0];
		data[1] = zPackData.fv[0];
		return data;
	}

	this.readPackY = function()
	{
		var v = this.readUint16();
		
		var yPackData = KBEngine._yPackData;
		yPackData.uv[0] = 0x40000000;
		yPackData.uv[0] |= (v & 0x7fff) << 12;
		yPackData.fv[0] -= 2.0;
		yPackData.uv[0] |= (v & 0x8000) << 16;
		return yPackData.fv[0];
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
		for(var i=0; i<4; i++)
			this.writeInt8(v >> i * 8 & 0xff);
	}

	this.writeInt64 = function(v)
	{
		this.writeInt32(v.lo);
		this.writeInt32(v.hi);
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
		for(var i=0; i<4; i++)
			this.writeUint8(v >> i * 8 & 0xff);
	}

	this.writeUint64 = function(v)
	{
		this.writeUint32(v.lo);
		this.writeUint32(v.hi);
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
		var size = v.length;
		if(size + 4> this.space())
		{
			KBEngine.ERROR_MSG("memorystream::writeBlob: no free!");
			return;
		}
		
		this.writeUint32(size);
		var buf = new Uint8Array(this.buffer, this.wpos, size);
		
		if(typeof(v) == "string")
		{
			for(var i=0; i<size; i++)
			{
				buf[i] = v.charCodeAt(i);
			}
		}
		else
		{
			for(var i=0; i<size; i++)
			{
				buf[i] = v[i];
			}
		}
		
		this.wpos += size;
	}
	
	this.writeString = function(v)
	{
		if(v.length > this.space())
		{
			KBEngine.ERROR_MSG("memorystream::writeString: no free!");
			return;
		}
		
		var buf = new Uint8Array(this.buffer, this.wpos);
		var i = 0;
		for(var idx=0; idx<v.length; idx++)
		{
			buf[i++] = v.charCodeAt(idx);
		}
		
		buf[i++] = 0;
		this.wpos += i;
	}

	this.append = function(stream, offset, size)
	{
		if(!(stream instanceof KBEngine.MemoryStream)) 
		{
			KBEngine.ERROR_MSG("MemoryStream::append(): stream must be MemoryStream instances");
			return;
		}

		if(size > this.space())
		{
			this.buffer = ArrayBuffer.transfer(this.buffer, this.buffer.byteLength + size * 2);
		}

		var buf = new Uint8Array(this.buffer, this.wpos, size);
		buf.set(new Uint8Array(stream.buffer, offset, size), 0);
		this.wpos += size;
	}
	
	//---------------------------------------------------------------------------------
	this.readSkip = function(v)
	{
		this.rpos += v;
	}
	
	//---------------------------------------------------------------------------------
	this.space = function()
	{
		return this.buffer.byteLength - this.wpos;
	}

	//---------------------------------------------------------------------------------
	this.length = function()
	{
		return this.wpos - this.rpos;
	}

	//---------------------------------------------------------------------------------
	this.readEOF = function()
	{
		return this.buffer.byteLength - this.rpos <= 0;
	}

	//---------------------------------------------------------------------------------
	this.done = function()
	{
		this.rpos = this.wpos;
	}
	
	//---------------------------------------------------------------------------------
	this.getbuffer = function(v)
	{
		return this.buffer.slice(this.rpos, this.wpos);
	}

	//---------------------------------------------------------------------------------
	this.setbuffer = function(buffer)
	{
		this.clear();
		this.buffer = buffer;
	}

	//---------------------------------------------------------------------------------
	this.size = function()
	{
		return this.buffer.byteLength;
	}

	//---------------------------------------------------------------------------------
	this.clear = function()
	{
		this.rpos = 0;
		this.wpos = 0;

		if(this.buffer.byteLength > KBEngine.PACKET_MAX_SIZE)
			this.buffer = new ArrayBuffer(KBEngine.PACKET_MAX_SIZE);
	}

	this.reclaimObject = function()
	{
		this.clear();

		if(KBEngine.MemoryStream._objects != undefined)
			KBEngine.MemoryStream._objects.push(this);
	}
}

KBEngine.MemoryStream.createObject = function()
{
	if(KBEngine.MemoryStream._objects == undefined)
		KBEngine.MemoryStream._objects = [];

	return KBEngine.MemoryStream._objects.length > 0 ? KBEngine.MemoryStream._objects.pop() : new KBEngine.MemoryStream(KBEngine.PACKET_MAX_SIZE_TCP);
}



/*-----------------------------------------------------------------------------------------
												bundle
-----------------------------------------------------------------------------------------*/
KBEngine.Bundle = function()
{
	this.memorystreams = new Array();
	this.stream = KBEngine.MemoryStream.createObject();
	
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
			this.messageLengthBuffer = new Uint8Array(this.stream.buffer, this.stream.wpos + KBEngine.MESSAGE_ID_LENGTH, 2);
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

			this.stream = KBEngine.MemoryStream.createObject();
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
		
		for(var i=0; i<this.memorystreams.length; i++)
		{
			var tmpStream = this.memorystreams[i];
			network.send(tmpStream.getbuffer());
		}

		this.reclaimObject();
	}
	
	//---------------------------------------------------------------------------------
	this.checkStream = function(v)
	{
		if(v > this.stream.space())
		{
			this.memorystreams.push(this.stream);
			this.stream = KBEngine.MemoryStream.createObject();
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

	this.clear = function()
	{
		for(var i=0; i<this.memorystreams.length; i++)
		{
			if(this.stream != this.memorystreams[i])
				this.memorystreams[i].reclaimObject();
		}

		if(this.stream)
			this.stream.clear();
		else
			this.stream = KBEngine.MemoryStream.createObject();

		this.memorystreams = new Array();
		this.numMessage = 0;
		this.messageLengthBuffer = null;
		this.messageLength = 0;
		this.msgtype = null;
	}

	this.reclaimObject = function()
	{
		this.clear();

		if(KBEngine.Bundle._objects != undefined)
			KBEngine.Bundle._objects.push(this);
	}
}

KBEngine.Bundle.createObject = function()
{
	if(KBEngine.Bundle._objects == undefined)
		KBEngine.Bundle._objects = [];

	return KBEngine.Bundle._objects.length > 0 ? KBEngine.Bundle._objects.pop() : new KBEngine.Bundle();
}

/*-----------------------------------------------------------------------------------------
												messages
-----------------------------------------------------------------------------------------*/
KBEngine.reader = new KBEngine.MemoryStream(0);
KBEngine.datatype2id = {};

KBEngine.mappingDataType = function(writer, argType)
{
	KBEngine.datatype2id = {};
	KBEngine.datatype2id["STRING"] = 1;
	KBEngine.datatype2id["STD::STRING"] = 1;

	KBEngine.datatype2id["UINT8"] = 2;
	KBEngine.datatype2id["BOOL"] = 2;
	KBEngine.datatype2id["DATATYPE"] = 2;
	KBEngine.datatype2id["CHAR"] = 2;
	KBEngine.datatype2id["DETAIL_TYPE"] = 2;
	KBEngine.datatype2id["ENTITYCALL_CALL_TYPE"] = 2;

	KBEngine.datatype2id["UINT16"] = 3;
	KBEngine.datatype2id["UNSIGNED SHORT"] = 3;
	KBEngine.datatype2id["SERVER_ERROR_CODE"] = 3;
	KBEngine.datatype2id["ENTITY_TYPE"] = 3;
	KBEngine.datatype2id["ENTITY_PROPERTY_UID"] = 3;
	KBEngine.datatype2id["ENTITY_METHOD_UID"] = 3;
	KBEngine.datatype2id["ENTITY_SCRIPT_UID"] = 3;
	KBEngine.datatype2id["DATATYPE_UID"] = 3;

	KBEngine.datatype2id["UINT32"] = 4;
	KBEngine.datatype2id["UINT"] = 4;
	KBEngine.datatype2id["UNSIGNED INT"] = 4;
	KBEngine.datatype2id["ARRAYSIZE"] = 4;
	KBEngine.datatype2id["SPACE_ID"] = 4;
	KBEngine.datatype2id["GAME_TIME"] = 4;
	KBEngine.datatype2id["TIMER_ID"] = 4;

	KBEngine.datatype2id["UINT64"] = 5;
	KBEngine.datatype2id["DBID"] = 5;
	KBEngine.datatype2id["COMPONENT_ID"] = 5;

	KBEngine.datatype2id["INT8"] = 6;
	KBEngine.datatype2id["COMPONENT_ORDER"] = 6;

	KBEngine.datatype2id["INT16"] = 7;
	KBEngine.datatype2id["SHORT"] = 7;

	KBEngine.datatype2id["INT32"] = 8;
	KBEngine.datatype2id["INT"] = 8;
	KBEngine.datatype2id["ENTITY_ID"] = 8;
	KBEngine.datatype2id["CALLBACK_ID"] = 8;
	KBEngine.datatype2id["COMPONENT_TYPE"] = 8;

	KBEngine.datatype2id["INT64"] = 9;

	KBEngine.datatype2id["PYTHON"] = 10;
	KBEngine.datatype2id["PY_DICT"] = 10;
	KBEngine.datatype2id["PY_TUPLE"] = 10;
	KBEngine.datatype2id["PY_LIST"] = 10;

	KBEngine.datatype2id["BLOB"] = 11;

	KBEngine.datatype2id["UNICODE"] = 12;

	KBEngine.datatype2id["FLOAT"] = 13;

	KBEngine.datatype2id["DOUBLE"] = 14;

	KBEngine.datatype2id["VECTOR2"] = 15;

	KBEngine.datatype2id["VECTOR3"] = 16;

	KBEngine.datatype2id["VECTOR4"] = 17;

	KBEngine.datatype2id["FIXED_DICT"] = 18;

	KBEngine.datatype2id["ARRAY"] = 19;

	KBEngine.datatype2id["ENTITYCALL"] = 20;
}

KBEngine.mappingDataType();

KBEngine.bindwriter = function(writer, argType)
{
	if(argType == KBEngine.datatype2id["UINT8"])
	{
		return writer.writeUint8;
	}
	else if(argType == KBEngine.datatype2id["UINT16"])
	{
		return writer.writeUint16;
	}
	else if(argType == KBEngine.datatype2id["UINT32"])
	{
		return writer.writeUint32;
	}
	else if(argType == KBEngine.datatype2id["UINT64"])
	{
		return writer.writeUint64;
	}
	else if(argType == KBEngine.datatype2id["INT8"])
	{
		return writer.writeInt8;
	}
	else if(argType == KBEngine.datatype2id["INT16"])
	{
		return writer.writeInt16;
	}
	else if(argType == KBEngine.datatype2id["INT32"])
	{
		return writer.writeInt32;
	}
	else if(argType == KBEngine.datatype2id["INT64"])
	{
		return writer.writeInt64;
	}
	else if(argType == KBEngine.datatype2id["FLOAT"])
	{
		return writer.writeFloat;
	}
	else if(argType == KBEngine.datatype2id["DOUBLE"])
	{
		return writer.writeDouble;
	}
	else if(argType == KBEngine.datatype2id["STRING"])
	{
		return writer.writeString;
	}
	else if(argType == KBEngine.datatype2id["FIXED_DICT"])
	{
		return writer.writeStream;
	}
	else if(argType == KBEngine.datatype2id["ARRAY"])
	{
		return writer.writeStream;
	}
	else
	{
		return writer.writeStream;
	}
}

KBEngine.bindReader = function(argType)
{
	if(argType == KBEngine.datatype2id["UINT8"])
	{
		return KBEngine.reader.readUint8;
	}
	else if(argType == KBEngine.datatype2id["UINT16"])
	{
		return KBEngine.reader.readUint16;
	}
	else if(argType == KBEngine.datatype2id["UINT32"])
	{
		return KBEngine.reader.readUint32;
	}
	else if(argType == KBEngine.datatype2id["UINT64"])
	{
		return KBEngine.reader.readUint64;
	}
	else if(argType == KBEngine.datatype2id["INT8"])
	{
		return KBEngine.reader.readInt8;
	}
	else if(argType == KBEngine.datatype2id["INT16"])
	{
		return KBEngine.reader.readInt16;
	}
	else if(argType == KBEngine.datatype2id["INT32"])
	{
		return KBEngine.reader.readInt32;
	}
	else if(argType == KBEngine.datatype2id["INT64"])
	{
		return KBEngine.reader.readInt64;
	}
	else if(argType == KBEngine.datatype2id["FLOAT"])
	{
		return KBEngine.reader.readFloat;
	}
	else if(argType == KBEngine.datatype2id["DOUBLE"])
	{
		return KBEngine.reader.readDouble;
	}
	else if(argType == KBEngine.datatype2id["STRING"])
	{
		return KBEngine.reader.readString;
	}
	else if(argType == KBEngine.datatype2id["PYTHON"])
	{
		return KBEngine.reader.readStream;
	}
	else if(argType == KBEngine.datatype2id["VECTOR2"])
	{
		return KBEngine.reader.readStream;
	}
	else if(argType == KBEngine.datatype2id["VECTOR3"])
	{
		return KBEngine.reader.readStream;
	}
	else if(argType == KBEngine.datatype2id["VECTOR4"])
	{
		return KBEngine.reader.readStream;
	}
	else if(argType == KBEngine.datatype2id["BLOB"])
	{
		return KBEngine.reader.readStream;
	}
	else if(argType == KBEngine.datatype2id["UNICODE"])
	{
		return KBEngine.reader.readStream;
	}
	else if(argType == KBEngine.datatype2id["FIXED_DICT"])
	{
		return KBEngine.reader.readStream;
	}
	else if(argType == KBEngine.datatype2id["ARRAY"])
	{
		return KBEngine.reader.readStream;
	}
	else
	{
		return KBEngine.reader.readStream;
	}
}
	
KBEngine.Message = function(id, name, length, argstype, args, handler)
{
	this.id = id;
	this.name = name;
	this.length = length;
	this.argsType = argstype;
	
	// 绑定执行
	for(var i=0; i<args.length; i++)
	{
		args[i] = KBEngine.bindReader(args[i]);
	}
	
	this.args = args;
	this.handler = handler;
	
	this.createFromStream = function(msgstream)
	{
		if(this.args.length <= 0)
			return msgstream;
		
		var result = new Array(this.args.length);
		for(var i=0; i<this.args.length; i++)
		{
			result[i] = this.args[i].call(msgstream);
		}
		
		return result;
	}
	
	this.handleMessage = function(msgstream)
	{
		if(this.handler == null)
		{
			KBEngine.ERROR_MSG("KBEngine.Message::handleMessage: interface(" + this.name + "/" + this.id + ") no implement!");  
			return;
		}

		if(this.args.length <= 0)
		{
			if(this.argsType < 0)
				this.handler(msgstream);
			else
				this.handler();
		}
		else
		{
			this.handler.apply(KBEngine.app, this.createFromStream(msgstream));
		}
	}
}

// 上行消息
KBEngine.messages = {};
KBEngine.messages["loginapp"] = {};
KBEngine.messages["baseapp"] = {};
KBEngine.clientmessages = {};

KBEngine.messages["Loginapp_importClientMessages"] = new KBEngine.Message(5, "importClientMessages", 0, 0, new Array(), null);
KBEngine.messages["Baseapp_importClientMessages"] = new KBEngine.Message(207, "importClientMessages", 0, 0, new Array(), null);
KBEngine.messages["Baseapp_importClientEntityDef"] = new KBEngine.Message(208, "importClientEntityDef", 0, 0, new Array(), null);
KBEngine.messages["onImportClientMessages"] = new KBEngine.Message(518, "onImportClientMessages", -1, -1, new Array(), null);

KBEngine.bufferedCreateEntityMessages = {};

/*-----------------------------------------------------------------------------------------
												math
-----------------------------------------------------------------------------------------*/
KBEngine.Vector2 = KBEngine.Class.extend(
{
		ctor:function (x, y) {
			this.x = x;
			this.y = y;
			return true;
		},
	
		distance : function(pos)
		{
			var x = pos.x - this.x;
			var y = pos.y - this.y;
			return Math.sqrt(x * x + y * y);
		},

		add : function(vec3)
		{
			this.x += vec3.x;
			this.y += vec3.y;
			return this;
		},

		sub: function(vec3)
		{
			this.x -= vec3.x;
			this.y -= vec3.y;
			return this;
		},

		mul: function(num)
		{
			this.x *= num;
			this.y *= num;
			return this;
		},

		div: function(num)
		{
			this.x /= num;
			this.y /= num;
			return this;
		},

		neg: function()
		{
			this.x = -this.x;
			this.y = -this.y;
			return this;
		}
});

KBEngine.Vector3 = KBEngine.Class.extend(
{
    ctor:function (x, y, z) {
		this.x = x;
		this.y = y;
		this.z = z;
        return true;
    },

    distance : function(pos)
    {
    	var x = pos.x - this.x;
    	var y = pos.y - this.y;
    	var z = pos.z - this.z;
    	return Math.sqrt(x * x + y * y + z * z);
	},
	
	//向量加法
	add : function(vec3)
	{
		this.x += vec3.x;
		this.y += vec3.y;
		this.z += vec3.z;
		return this;
	},

	//向量减法
	sub: function(vec3)
	{
		this.x -= vec3.x;
		this.y -= vec3.y;
		this.z -= vec3.z;
		return this;
	},

	//向量乘法
	mul: function(num)
	{
		this.x *= num;
		this.y *= num;
		this.z *= num;
		return this;
	},

	//向量除法
	div: function(num)
	{
		this.x /= num;
		this.y /= num;
		this.z /= num;
		return this;
	},

	// 向量取反
	neg: function()
	{
		this.x = -this.x;
		this.y = -this.y;
		this.z = -this.z;
		return this;
	}
});

KBEngine.Vector4 = KBEngine.Class.extend(
{
	ctor:function (x, y, z, w) {
		this.x = x;
		this.y = y;
		this.z = z;
		this.w = w;
		return true;
	},

	distance : function(pos)
	{
		var x = pos.x - this.x;
		var y = pos.y - this.y;
		var z = pos.z - this.z;
		var w = pos.w - this.w;
		return Math.sqrt(x * x + y * y + z * z + w * w );
	},

	add : function(vec4)
	{
		this.x += vec4.x;
		this.y += vec4.y;
		this.z += vec4.z;
		this.w += vec4.w;
		return this;
	},

	sub: function(vec4)
	{
		this.x -= vec4.x;
		this.y -= vec4.y;
		this.z -= vec4.z;
		this.w -= vec4.w;
		return this;
	},

	mul: function(num)
	{
		this.x *= num;
		this.y *= num;
		this.z *= num;
		this.w *= num;
		return this;
	},

	div: function(num)
	{
		this.x /= num;
		this.y /= num;
		this.z /= num;
		this.w /= num;
		return this;
	},

	neg: function()
	{
		this.x = -this.x;
		this.y = -this.y;
		this.z = -this.z;
		this.w = -this.w;
		return this;
	}
});

KBEngine.clampf = function (value, min_inclusive, max_inclusive) 
{
    if (min_inclusive > max_inclusive) {
        var temp = min_inclusive;
        min_inclusive = max_inclusive;
        max_inclusive = temp;
    }
    return value < min_inclusive ? min_inclusive : value < max_inclusive ? value : max_inclusive;
};

KBEngine.int82angle = function(angle/*int8*/, half/*bool*/)
{
	return angle * (Math.PI / (half ? 254.0 : 128.0));
};

KBEngine.angle2int8 = function(v/*float*/, half/*bool*/)
{
	var angle = 0;
	if(!half)
	{
		angle = Math.floor((v * 128.0) / float(Math.PI) + 0.5);
	}
	else
	{
		angle = KBEngine.clampf(floorf( (v * 254.0) / float(Math.PI) + 0.5), -128.0, 127.0);
	}

	return angle;
};

/*-----------------------------------------------------------------------------------------
												entity
-----------------------------------------------------------------------------------------*/
KBEngine.Entity = KBEngine.Class.extend(
{
    ctor:function () {
		this.id = 0;
		this.className = "";
		this.position = new KBEngine.Vector3(0.0, 0.0, 0.0);
		this.direction = new KBEngine.Vector3(0.0, 0.0, 0.0);
		this.velocity = 0.0
			
		this.cell = null;
		this.base = null;
		
		// enterworld之后设置为true
		this.inWorld = false;
		
		// __init__调用之后设置为true
		this.inited = false;
		
		// 是否被控制
		this.isControlled = false;

		this.entityLastLocalPos = new KBEngine.Vector3(0.0, 0.0, 0.0);
		this.entityLastLocalDir = new KBEngine.Vector3(0.0, 0.0, 0.0);
		
		// 玩家是否在地面上
		this.isOnGround = false;

        return true;
    },
    
    // 与服务端实体脚本中__init__类似, 代表初始化实体
	__init__ : function()
	{
	},

	callPropertysSetMethods : function()
	{
		var currModule = KBEngine.moduledefs[this.className];
		for(var name in currModule.propertys)
		{
			var propertydata = currModule.propertys[name];
			var properUtype = propertydata[0];
			var name = propertydata[2];
			var setmethod = propertydata[5];
			var flags = propertydata[6];
			var oldval = this[name];
			
			if(setmethod != null)
			{
				// base类属性或者进入世界后cell类属性会触发set_*方法
				// ED_FLAG_BASE_AND_CLIENT、ED_FLAG_BASE
				if(flags == 0x00000020 || flags == 0x00000040)
				{
					if(this.inited && !this.inWorld)
						setmethod.call(this, oldval);
				}
				else
				{
					if(this.inWorld)
					{
						if(flags == 0x00000008 || flags == 0x00000010)
						{
							if(!this.isPlayer())
								continue;
						}
						
						setmethod.call(this, oldval);
					}
				}
			}
		};
	},

	onDestroy : function()
	{
	},

	onControlled : function(bIsControlled)
	{
	},

	isPlayer : function()
	{
		return this.id == KBEngine.app.entity_id;
	},

	baseCall : function()
	{
		if(arguments.length < 1)
		{
			KBEngine.ERROR_MSG('KBEngine.Entity::baseCall: not fount interfaceName!');  
			return;
		}

		if(this.base == undefined)
		{
			KBEngine.ERROR_MSG('KBEngine.Entity::baseCall: base is None!');  
			return;			
		}
		
		var method = KBEngine.moduledefs[this.className].base_methods[arguments[0]];

		if(method == undefined)
		{
			KBEngine.ERROR_MSG("KBEngine.Entity::baseCall: The server did not find the def_method(" + this.className + "." + arguments[0] + ")!");
			return;
		}
		
		var methodID = method[0];
		var args = method[3];
		
		if(arguments.length - 1 != args.length)
		{
			KBEngine.ERROR_MSG("KBEngine.Entity::baseCall: args(" + (arguments.length - 1) + "!= " + args.length + ") size is error!");  
			return;
		}
		
		this.base.newCall();
		this.base.bundle.writeUint16(methodID);
		
		try
		{
			for(var i=0; i<args.length; i++)
			{
				if(args[i].isSameType(arguments[i + 1]))
				{
					args[i].addToStream(this.base.bundle, arguments[i + 1]);
				}
				else
				{
					throw new Error("KBEngine.Entity::baseCall: arg[" + i + "] is error!");
				}
			}
		}
		catch(e)
		{
			KBEngine.ERROR_MSG(e.toString());
			KBEngine.ERROR_MSG('KBEngine.Entity::baseCall: args is error!');
			this.base.bundle = null;
			return;
		}
		
		this.base.sendCall();
	},
	
	cellCall : function()
	{
		if(arguments.length < 1)
		{
			KBEngine.ERROR_MSG('KBEngine.Entity::cellCall: not fount interfaceName!');  
			return;
		}
		
		if(this.cell == undefined)
		{
			KBEngine.ERROR_MSG('KBEngine.Entity::cellCall: cell is None!');  
			return;			
		}
		
		var method = KBEngine.moduledefs[this.className].cell_methods[arguments[0]];
		
		if(method == undefined)
		{
			KBEngine.ERROR_MSG("KBEngine.Entity::cellCall: The server did not find the def_method(" + this.className + "." + arguments[0] + ")!");
			return;
		}
		
		var methodID = method[0];
		var args = method[3];
		
		if(arguments.length - 1 != args.length)
		{
			KBEngine.ERROR_MSG("KBEngine.Entity::cellCall: args(" + (arguments.length - 1) + "!= " + args.length + ") size is error!");  
			return;
		}
		
		this.cell.newCall();
		this.cell.bundle.writeUint16(methodID);
		
		try
		{
			for(var i=0; i<args.length; i++)
			{
				if(args[i].isSameType(arguments[i + 1]))
				{
					args[i].addToStream(this.cell.bundle, arguments[i + 1]);
				}
				else
				{
					throw new Error("KBEngine.Entity::cellCall: arg[" + i + "] is error!");
				}
			}
		}
		catch(e)
		{
			KBEngine.ERROR_MSG(e.toString());
			KBEngine.ERROR_MSG('KBEngine.Entity::cellCall: args is error!');
			this.cell.bundle = null;
			return;
		}
		
		this.cell.sendCall();
	},
	
	enterWorld : function()
	{
		KBEngine.INFO_MSG(this.className + '::enterWorld: ' + this.id); 
		this.inWorld = true;
		this.onEnterWorld();
		
		KBEngine.Event.fire(KBEngine.EventTypes.onEnterWorld, this);
	},

	onEnterWorld : function()
	{
	},
		
	leaveWorld : function()
	{
		KBEngine.INFO_MSG(this.className + '::leaveWorld: ' + this.id); 
		this.inWorld = false;
		this.onLeaveWorld();
		KBEngine.Event.fire(KBEngine.EventTypes.onLeaveWorld, this);
	},

	onLeaveWorld : function()
	{
	},
		
	enterSpace : function()
	{
		KBEngine.INFO_MSG(this.className + '::enterSpace: ' + this.id); 
		this.onEnterSpace();
		KBEngine.Event.fire(KBEngine.EventTypes.onEnterSpace, this);
		
		// 要立即刷新表现层对象的位置
		KBEngine.Event.fire(KBEngine.EventTypes.set_position, this);
		KBEngine.Event.fire(KBEngine.EventTypes.set_direction, this);
	},

	onEnterSpace : function()
	{
	},
		
	leaveSpace : function()
	{
		KBEngine.INFO_MSG(this.className + '::leaveSpace: ' + this.id); 
		this.onLeaveSpace();
		KBEngine.Event.fire("onLeaveSpace", this);
	},

	onLeaveSpace : function()
	{
	},
		
	set_position : function(old)
	{
		// KBEngine.DEBUG_MSG(this.className + "::set_position: " + old);  
		
		if(this.isPlayer())
		{
			KBEngine.app.entityServerPos.x = this.position.x;
			KBEngine.app.entityServerPos.y = this.position.y;
			KBEngine.app.entityServerPos.z = this.position.z;
		}
		
		KBEngine.Event.fire(KBEngine.EventTypes.set_position, this);
	},

	onUpdateVolatileData : function()
	{
	},
	
	set_direction : function(old)
	{
		// KBEngine.DEBUG_MSG(this.className + "::set_direction: " + old);  
		KBEngine.Event.fire(KBEngine.EventTypes.set_direction, this);
	}
});

/*-----------------------------------------------------------------------------------------
												EntityCall
-----------------------------------------------------------------------------------------*/
KBEngine.ENTITYCALL_TYPE_CELL = 0;
KBEngine.ENTITYCALL_TYPE_BASE = 1;

KBEngine.EntityCall = function()
{
	this.id = 0;
	this.className = "";
	this.type = KBEngine.ENTITYCALL_TYPE_CELL;
	this.networkInterface = KBEngine.app;
	
	this.bundle = null;
	
	this.isBase = function()
	{
		return this.type == KBEngine.ENTITYCALL_TYPE_BASE;
	}

	this.isCell = function()
	{
		return this.type == KBEngine.ENTITYCALL_TYPE_CELL;
	}
	
	this.newCall = function()
	{  
		if(this.bundle == null)
			this.bundle = KBEngine.Bundle.createObject();
		
		if(this.type == KBEngine.ENTITYCALL_TYPE_CELL)
			this.bundle.newMessage(KBEngine.messages.Baseapp_onRemoteCallCellMethodFromClient);
		else
			this.bundle.newMessage(KBEngine.messages.Entity_onRemoteMethodCall);

		this.bundle.writeInt32(this.id);
		
		return this.bundle;
	}
	
	this.sendCall = function(bundle)
	{
		if(bundle == undefined)
			bundle = this.bundle;
		
		bundle.send(this.networkInterface);
		
		if(this.bundle == bundle)
			this.bundle = null;
	}
}

/*-----------------------------------------------------------------------------------------
												entitydef
-----------------------------------------------------------------------------------------*/
KBEngine.moduledefs = {};
KBEngine.datatypes = {};

KBEngine.DATATYPE_UINT8 = function()
{
	this.bind = function()
	{
	}
	
	this.createFromStream = function(stream)
	{
		return KBEngine.reader.readUint8.call(stream);
	}
	
	this.addToStream = function(stream, v)
	{
		stream.writeUint8(v);
	}

	this.parseDefaultValStr = function(v)
	{
		return parseInt(v);
	}
	
	this.isSameType = function(v)
	{
		if(typeof(v) != "number")
		{
			return false;
		}
		
		if(v < 0 || v > 0xff)
		{
			return false;
		}
		
		return true;
	}
}

KBEngine.DATATYPE_UINT16 = function()
{
	this.bind = function()
	{
	}
	
	this.createFromStream = function(stream)
	{
		return KBEngine.reader.readUint16.call(stream);
	}
	
	this.addToStream = function(stream, v)
	{
		stream.writeUint16(v);
	}
	
	this.parseDefaultValStr = function(v)
	{
		return parseInt(v);
	}
	
	this.isSameType = function(v)
	{
		if(typeof(v) != "number")
		{
			return false;
		}
		
		if(v < 0 || v > 0xffff)
		{
			return false;
		}
		
		return true;
	}
}

KBEngine.DATATYPE_UINT32 = function()
{
	this.bind = function()
	{
	}
	
	this.createFromStream = function(stream)
	{
		return KBEngine.reader.readUint32.call(stream);
	}
	
	this.addToStream = function(stream, v)
	{
		stream.writeUint32(v);
	}
	
	this.parseDefaultValStr = function(v)
	{
		return parseInt(v);
	}
	
	this.isSameType = function(v)
	{
		if(typeof(v) != "number")
		{
			return false;
		}
		
		if(v < 0 || v > 0xffffffff)
		{
			return false;
		}
		
		return true;
	}
}

KBEngine.DATATYPE_UINT64 = function()
{
	this.bind = function()
	{
	}
	
	this.createFromStream = function(stream)
	{
		return KBEngine.reader.readUint64.call(stream);
	}
	
	this.addToStream = function(stream, v)
	{
		stream.writeUint64(v);
	}
	
	this.parseDefaultValStr = function(v)
	{
		return parseInt(v);
	}
	
	this.isSameType = function(v)
	{
		return v instanceof KBEngine.UINT64;
	}
}

KBEngine.DATATYPE_INT8 = function()
{
	this.bind = function()
	{
	}
	
	this.createFromStream = function(stream)
	{
		return KBEngine.reader.readInt8.call(stream);
	}
	
	this.addToStream = function(stream, v)
	{
		stream.writeInt8(v);
	}
	
	this.parseDefaultValStr = function(v)
	{
		return parseInt(v);
	}
	
	this.isSameType = function(v)
	{
		if(typeof(v) != "number")
		{
			return false;
		}
		
		if(v < -0x80 || v > 0x7f)
		{
			return false;
		}
		
		return true;
	}
}

KBEngine.DATATYPE_INT16 = function()
{
	this.bind = function()
	{
	}
	
	this.createFromStream = function(stream)
	{
		return KBEngine.reader.readInt16.call(stream);
	}
	
	this.addToStream = function(stream, v)
	{
		stream.writeInt16(v);
	}
	
	this.parseDefaultValStr = function(v)
	{
		return parseInt(v);
	}
	
	this.isSameType = function(v)
	{
		if(typeof(v) != "number")
		{
			return false;
		}
		
		if(v < -0x8000 || v > 0x7fff)
		{
			return false;
		}
		
		return true;
	}
}

KBEngine.DATATYPE_INT32 = function()
{
	this.bind = function()
	{
	}
	
	this.createFromStream = function(stream)
	{
		return KBEngine.reader.readInt32.call(stream);
	}
	
	this.addToStream = function(stream, v)
	{
		stream.writeInt32(v);
	}
	
	this.parseDefaultValStr = function(v)
	{
		return parseInt(v);
	}
	
	this.isSameType = function(v)
	{
		if(typeof(v) != "number")
		{
			return false;
		}
		
		if(v < -0x80000000 || v > 0x7fffffff)
		{
			return false;
		}
		
		return true;
	}
}

KBEngine.DATATYPE_INT64 = function()
{
	this.bind = function()
	{
	}
	
	this.createFromStream = function(stream)
	{
		return KBEngine.reader.readInt64.call(stream);
	}
	
	this.addToStream = function(stream, v)
	{
		stream.writeInt64(v);
	}
	
	this.parseDefaultValStr = function(v)
	{
		return parseInt(v);
	}
	
	this.isSameType = function(v)
	{
		return v instanceof KBEngine.INT64;
	}
}

KBEngine.DATATYPE_FLOAT = function()
{
	this.bind = function()
	{
	}
	
	this.createFromStream = function(stream)
	{
		return KBEngine.reader.readFloat.call(stream);
	}
	
	this.addToStream = function(stream, v)
	{
		stream.writeFloat(v);
	}
	
	this.parseDefaultValStr = function(v)
	{
		return parseFloat(v);
	}
	
	this.isSameType = function(v)
	{
		return typeof(v) == "number";
	}
}

KBEngine.DATATYPE_DOUBLE = function()
{
	this.bind = function()
	{
	}
	
	this.createFromStream = function(stream)
	{
		return KBEngine.reader.readDouble.call(stream);
	}
	
	this.addToStream = function(stream, v)
	{
		stream.writeDouble(v);
	}
	
	this.parseDefaultValStr = function(v)
	{
		return parseFloat(v);
	}
	
	this.isSameType = function(v)
	{
		return typeof(v) == "number";
	}
}

KBEngine.DATATYPE_STRING = function()
{
	this.bind = function()
	{
	}
	
	this.createFromStream = function(stream)
	{
		return KBEngine.reader.readString.call(stream);
	}
	
	this.addToStream = function(stream, v)
	{
		stream.writeString(v);
	}
	
	this.parseDefaultValStr = function(v)
	{
		if(typeof(v) == "string")
			return v;
		
		return "";
	}
	
	this.isSameType = function(v)
	{
		return typeof(v) == "string";
	}
}

KBEngine.DATATYPE_VECTOR2 = function()
{
	this.bind = function()
	{
	}
	
	this.createFromStream = function(stream)
	{
		if(KBEngine.CLIENT_NO_FLOAT)
		{
			var x = KBEngine.reader.readInt32.call(stream);
			var y = KBEngine.reader.readInt32.call(stream);
			return new KBEngine.Vector2(x, y);
		}
		else
		{
			var x = KBEngine.reader.readFloat.call(stream);
			var y = KBEngine.reader.readFloat.call(stream);
			return new KBEngine.Vector2(x, y);
		}
		
		return undefined;
	}
	
	this.addToStream = function(stream, v)
	{
		if(KBEngine.CLIENT_NO_FLOAT)
		{
			stream.writeInt32(v.x);
			stream.writeInt32(v.y);
		}
		else
		{
			stream.writeFloat(v.x);
			stream.writeFloat(v.y);			
		}
	}
	
	this.parseDefaultValStr = function(v)
	{
		return new KBEngine.Vector2(0.0, 0.0);
	}
	
	this.isSameType = function(v)
	{
		if(! v instanceof KBEngine.Vector2)
		{
			return false;
		}
		
		return true;
	}
}

KBEngine.DATATYPE_VECTOR3 = function()
{
	this.bind = function()
	{
	}
	
	this.createFromStream = function(stream)
	{
		if(KBEngine.CLIENT_NO_FLOAT)
		{
			var x = KBEngine.reader.readInt32.call(stream);
			var y = KBEngine.reader.readInt32.call(stream);
			var z = KBEngine.reader.readInt32.call(stream);
			return new KBEngine.Vector3(x, y, z);
		}
		else
		{
			var x = KBEngine.reader.readFloat.call(stream);
			var y = KBEngine.reader.readFloat.call(stream);
			var z = KBEngine.reader.readFloat.call(stream);
			return new KBEngine.Vector3(x, y, z);
		}
	
		return undefined;
	}
	
	this.addToStream = function(stream, v)
	{
		if(KBEngine.CLIENT_NO_FLOAT)
		{
			stream.writeInt32(v.x);
			stream.writeInt32(v.y);
			stream.writeInt32(v.z);
		}
		else
		{
			stream.writeFloat(v.x);
			stream.writeFloat(v.y);
			stream.writeFloat(v.z);
		}
	}
	
	this.parseDefaultValStr = function(v)
	{
		return new KBEngine.Vector3(0.0, 0.0, 0.0);
	}
	
	this.isSameType = function(v)
	{
		if(! v instanceof KBEngine.Vector3)
		{
			return false;
		}
		
		return true;
	}
}

KBEngine.DATATYPE_VECTOR4 = function()
{
	this.bind = function()
	{
	}
	
	this.createFromStream = function(stream)
	{
		if(KBEngine.CLIENT_NO_FLOAT)
		{
			var x = KBEngine.reader.readInt32.call(stream);
			var y = KBEngine.reader.readInt32.call(stream);
			var z = KBEngine.reader.readInt32.call(stream);
			var w = KBEngine.reader.readInt32.call(stream);
			return new KBEngine.Vector4(x, y, z, w);
		}
		else
		{
			var x = KBEngine.reader.readFloat.call(stream);
			var y = KBEngine.reader.readFloat.call(stream);
			var z = KBEngine.reader.readFloat.call(stream);
			var w = KBEngine.reader.readFloat.call(stream);
			return new KBEngine.Vector4(x, y, z, w);
		}
		
		return undefined;
	}
	
	this.addToStream = function(stream, v)
	{
		if(KBEngine.CLIENT_NO_FLOAT)
		{
			stream.writeInt32(v.x);
			stream.writeInt32(v.y);
			stream.writeInt32(v.z);
			stream.writeInt32(v.w);
		}
		else
		{
			stream.writeFloat(v.x);
			stream.writeFloat(v.y);		
			stream.writeFloat(v.z);
			stream.writeFloat(v.w);		
		}
	}
	
	this.parseDefaultValStr = function(v)
	{
		return new KBEngine.Vector4(0.0, 0.0, 0.0, 0.0);
	}
	
	this.isSameType = function(v)
	{
		if(! v instanceof KBEngine.Vector4)
		{
			return false;
		}
		
		return true;
	}
}

KBEngine.DATATYPE_PYTHON = function()
{
	this.bind = function()
	{
	}
	
	this.createFromStream = function(stream)
	{
		return stream.readBlob();
	}
	
	this.addToStream = function(stream, v)
	{
		stream.writeBlob(v);
	}
	
	this.parseDefaultValStr = function(v)
	{
		return new Uint8Array();
	}
	
	this.isSameType = function(v)
	{
		return false;
	}
}

KBEngine.DATATYPE_UNICODE = function()
{
	this.bind = function()
	{
	}

	this.createFromStream = function(stream)
	{
		return KBEngine.utf8ArrayToString(KBEngine.reader.readBlob.call(stream));
	}
	
	this.addToStream = function(stream, v)
	{
		stream.writeBlob(KBEngine.stringToUTF8Bytes(v));
	}
	
	this.parseDefaultValStr = function(v)
	{
		if(typeof(v) == "string")
			return v;
		
		return "";
	}
	
	this.isSameType = function(v)
	{
		return typeof(v) == "string";
	}
}

KBEngine.DATATYPE_ENTITYCALL = function()
{
	this.bind = function()
	{
	}
	
	this.createFromStream = function(stream)
	{
		var cid = KBEngine.reader.readUint64.call(stream);
		var id = KBEngine.reader.readInt32.call(stream);
		var type = KBEngine.reader.readUint16.call(stream);
		var utype = KBEngine.reader.readUint16.call(stream);
	}
	
	this.addToStream = function(stream, v)
	{
		var cid = new KBEngine.UINT64(0, 0);
		var id = 0;
		var type = 0;
		var utype = 0;

		stream.writeUint64(cid);
		stream.writeInt32(id);
		stream.writeUint16(type);
		stream.writeUint16(utype);
	}
	
	this.parseDefaultValStr = function(v)
	{
		return new Uint8Array();
	}
	
	this.isSameType = function(v)
	{
		return false;
	}
}

KBEngine.DATATYPE_BLOB = function()
{
	this.bind = function()
	{
	}
	
	this.createFromStream = function(stream)
	{
		var size = KBEngine.reader.readUint32.call(stream);
		var buf = new Uint8Array(stream.buffer, stream.rpos, size);
		stream.rpos += size;
		return buf;
	}
	
	this.addToStream = function(stream, v)
	{
		stream.writeBlob(v);
	}
	
	this.parseDefaultValStr = function(v)
	{
		return new Uint8Array();
	}
	
	this.isSameType = function(v)
	{
		return true;
	}
}

KBEngine.DATATYPE_ARRAY = function()
{
	this.type = null;
	
	this.bind = function()
	{
		if(typeof(this.type) == "number")
			this.type = KBEngine.datatypes[this.type];
	}
	
	this.createFromStream = function(stream)
	{
		var size = stream.readUint32();
		var datas = [];
		
		while(size > 0)
		{
			size--;
			datas.push(this.type.createFromStream(stream));
		};
		
		return datas;
	}
	
	this.addToStream = function(stream, v)
	{
		stream.writeUint32(v.length);
		for(var i=0; i<v.length; i++)
		{
			this.type.addToStream(stream, v[i]);
		}
	}
	
	this.parseDefaultValStr = function(v)
	{
		return [];
	}
	
	this.isSameType = function(v)
	{
		for(var i=0; i<v.length; i++)
		{
			if(!this.type.isSameType(v[i]))
			{
				return false;
			}
		}
		
		return true;
	}
}

KBEngine.DATATYPE_FIXED_DICT = function()
{
	this.dicttype = {};
	this.implementedBy = null;
	
	this.bind = function()
	{
		for(var itemkey in this.dicttype)
		{
			var utype = this.dicttype[itemkey];
			
			if(typeof(this.dicttype[itemkey]) == "number")
				this.dicttype[itemkey] = KBEngine.datatypes[utype];
		}
	}
	
	this.createFromStream = function(stream)
	{
		var datas = {};
		for(var itemkey in this.dicttype)
		{
			datas[itemkey] = this.dicttype[itemkey].createFromStream(stream);
		}
		
		return datas;
	}
	
	this.addToStream = function(stream, v)
	{
		for(var itemkey in this.dicttype)
		{
			this.dicttype[itemkey].addToStream(stream, v[itemkey]);
		}
	}
	
	this.parseDefaultValStr = function(v)
	{
		return {};
	}
	
	this.isSameType = function(v)
	{
		for(var itemkey in this.dicttype)
		{
			if(!this.dicttype[itemkey].isSameType(v[itemkey]))
			{
				return false;
			}
		}
		
		return true;
	}
}

KBEngine.datatypes["UINT8"]		= new KBEngine.DATATYPE_UINT8();
KBEngine.datatypes["UINT16"]	= new KBEngine.DATATYPE_UINT16();
KBEngine.datatypes["UINT32"]	= new KBEngine.DATATYPE_UINT32();
KBEngine.datatypes["UINT64"]	= new KBEngine.DATATYPE_UINT64();

KBEngine.datatypes["INT8"]		= new KBEngine.DATATYPE_INT8();
KBEngine.datatypes["INT16"]		= new KBEngine.DATATYPE_INT16();
KBEngine.datatypes["INT32"]		= new KBEngine.DATATYPE_INT32();
KBEngine.datatypes["INT64"]		= new KBEngine.DATATYPE_INT64();

KBEngine.datatypes["FLOAT"]		= new KBEngine.DATATYPE_FLOAT();
KBEngine.datatypes["DOUBLE"]	= new KBEngine.DATATYPE_DOUBLE();

KBEngine.datatypes["STRING"]	= new KBEngine.DATATYPE_STRING();
KBEngine.datatypes["VECTOR2"]	= new KBEngine.DATATYPE_VECTOR2;
KBEngine.datatypes["VECTOR3"]	= new KBEngine.DATATYPE_VECTOR3;
KBEngine.datatypes["VECTOR4"]	= new KBEngine.DATATYPE_VECTOR4;
KBEngine.datatypes["PYTHON"]	= new KBEngine.DATATYPE_PYTHON();
KBEngine.datatypes["UNICODE"]	= new KBEngine.DATATYPE_UNICODE();
KBEngine.datatypes["ENTITYCALL"]= new KBEngine.DATATYPE_ENTITYCALL();
KBEngine.datatypes["BLOB"]		= new KBEngine.DATATYPE_BLOB();

/*-----------------------------------------------------------------------------------------
												KBEngine args
-----------------------------------------------------------------------------------------*/
KBEngine.KBEngineArgs = function()
{
	this.ip = "127.0.0.1";
	this.port = @{KBE_USE_ALIAS_ENTITYID};
	this.updateHZ = @{KBE_UPDATEHZ} * 10;
	this.serverHeartbeatTick = @{KBE_SERVER_EXTERNAL_TIMEOUT};

	// Reference: http://www.kbengine.org/docs/programming/clientsdkprogramming.html, client types
	this.clientType = 5;

	// 在Entity初始化时是否触发属性的set_*事件(callPropertysSetMethods)
	this.isOnInitCallPropertysSetMethods = true;

	// 是否用wss, 默认使用ws
	this.isWss = false;
}

/*-----------------------------------------------------------------------------------------
												KBEngine app
-----------------------------------------------------------------------------------------*/
KBEngine.EventTypes =
{
	// Create new account.
	// <para> param1(string): accountName</para>
	// <para> param2(string): password</para>
	// <para> param3(bytes): datas // Datas by user defined. Data will be recorded into the KBE account database, you can access the datas through the script layer. If you use third-party account system, datas will be submitted to the third-party system.</para>
	createAccount : "createAccount",

	// Login to server.
	// <para> param1(string): accountName</para>
	// <para> param2(string): password</para>
	// <para> param3(bytes): datas // Datas by user defined. Data will be recorded into the KBE account database, you can access the datas through the script layer. If you use third-party account system, datas will be submitted to the third-party system.</para>
	login : "login",

	// Logout to baseapp, called when exiting the client.	
	logout : "logout",

	// Relogin to baseapp.
	reloginBaseapp : "reloginBaseapp",

	// Request server binding account Email.
	// <para> param1(string): emailAddress</para>
	bindAccountEmail : "bindAccountEmail",

	// Request to set up a new password for the account. Note: account must be online.
	// <para> param1(string): old_password</para>
	// <para> param2(string): new_password</para>
	newPassword : "newPassword",

	// ------------------------------------连接相关------------------------------------

	// Kicked of the current server.
	// <para> param1(uint16): retcode. // server_errors</para>
	onKicked : "onKicked",

	// Disconnected from the server.
	onDisconnected : "onDisconnected",

	// Status of connection server.
	// <para> param1(bool): success or fail</para>
	onConnectionState : "onConnectionState",

	// ------------------------------------logon相关------------------------------------

	// Create account feedback results.
	// <para> param1(uint16): retcode. // server_errors</para>
	// <para> param2(bytes): datas. // If you use third-party account system, the system may fill some of the third-party additional datas. </para>
	onCreateAccountResult : "onCreateAccountResult",

	// Engine version mismatch.
	// <para> param1(string): clientVersion
	// <para> param2(string): serverVersion
	onVersionNotMatch : "onVersionNotMatch",

	// script version mismatch.
    // <para> param1(string): clientScriptVersion
    // <para> param2(string): serverScriptVersion
	onScriptVersionNotMatch : "onScriptVersionNotMatch",

	// Login failed.
    // <para> param1(uint16): retcode. // server_errors</para>
	onLoginFailed : "onLoginFailed",

	// Login to baseapp.
	onLoginBaseapp : "onLoginBaseapp",

	// Login baseapp failed.
    // <para> param1(uint16): retcode. // server_errors</para>
	onLoginBaseappFailed : "onLoginBaseappFailed",

	// Relogin to baseapp.
	onReloginBaseapp : "onReloginBaseapp",

	// Relogin baseapp success.
	onReloginBaseappSuccessfully : "onReloginBaseappSuccessfully",

	// Relogin baseapp failed.
    // <para> param1(uint16): retcode. // server_errors</para>
	onReloginBaseappFailed : "onReloginBaseappFailed",

	// ------------------------------------实体cell相关事件------------------------------------

	// Entity enter the client-world.
    // <para> param1: Entity</para>
	onEnterWorld : "onEnterWorld",

	// Entity leave the client-world.
    // <para> param1: Entity</para>
	onLeaveWorld : "onLeaveWorld",

	// Player enter the new space.
    // <para> param1: Entity</para>
	onEnterSpace : "onEnterSpace",

	// Player leave the space.
    // <para> param1: Entity</para>
	onLeaveSpace : "onLeaveSpace",

	// Sets the current position of the entity.
	// <para> param1: Entity</para>
	set_position : "set_position",

	// Sets the current direction of the entity.
	// <para> param1: Entity</para>
	set_direction : "set_direction",

	// The entity position is updated, you can smooth the moving entity to new location.
	// <para> param1: Entity</para>
	updatePosition : "updatePosition",

	// The current space is specified by the geometry mapping.
	// Popular said is to load the specified Map Resources.
	// <para> param1(string): resPath</para>
	addSpaceGeometryMapping : "addSpaceGeometryMapping",

	// Server spaceData set data.
	// <para> param1(int32): spaceID</para>
	// <para> param2(string): key</para>
	// <para> param3(string): value</para>
	onSetSpaceData : "onSetSpaceData",

	// Start downloading data.
	// <para> param1(int32): rspaceID</para>
	// <para> param2(string): key</para>
	onDelSpaceData : "onDelSpaceData",

	// Triggered when the entity is controlled or out of control.
	// <para> param1: Entity</para>
	// <para> param2(bool): isControlled</para>
	onControlled : "onControlled",

	// Lose controlled entity.
	// <para> param1: Entity</para>
	onLoseControlledEntity : "onLoseControlledEntity",

	// ------------------------------------数据下载相关------------------------------------

	// Start downloading data.
	// <para> param1(uint16): resouce id</para>
	// <para> param2(uint32): data size</para>
	// <para> param3(string): description</para>
	onStreamDataStarted : "onStreamDataStarted",

	// Receive data.
	// <para> param1(uint16): resouce id</para>
	// <para> param2(bytes): datas</para>
	onStreamDataRecv : "onStreamDataRecv",

	// The downloaded data is completed.
	// <para> param1(uint16): resouce id</para>
	onStreamDataCompleted : "onStreamDataCompleted",
}

KBEngine.KBEngineApp = function(kbengineArgs)
{
	console.assert(KBEngine.app == null || KBEngine.app == undefined, "Assertion of KBEngine.app not is null");
	
	KBEngine.app = this;
	
	this.args = kbengineArgs;
	
	this.username = "testhtml51";
	this.password = "123456";
	this.clientdatas = "";
	this.encryptedKey = "";
	
	this.loginappMessageImported = false;
	this.baseappMessageImported = false;
	this.serverErrorsDescrImported = false;
	this.entitydefImported = false;
	
	KBEngine.getSingleton = function()
	{
		console.assert(KBEngine.app != undefined, "KBEngineApp is null");
		return KBEngine.app;
	}

	// 描述服务端返回的错误信息
	KBEngine.ServerErr = function()
	{
		this.name = "";
		this.descr = "";
		this.id = 0;
	}

	this.serverErrs = {};
		
	// 登录loginapp的地址
	this.ip = this.args.ip;
	this.port = this.args.port;
	this.isWss = this.args.isWss;
	this.protocol = this.isWss ? "wss://" : "ws://";
	
	// 服务端分配的baseapp地址
	this.baseappIP = "";
	this.baseappTcpPort = 0;
	this.baseappUdpPort = 0;
	
	this.currMsgID = 0;
	this.currMsgCount = 0;
	this.currMsgLen = 0;
	
	KBEngine.FragmentDataTypes = 
	{
		FRAGMENT_DATA_UNKNOW : 0,
		FRAGMENT_DATA_MESSAGE_ID : 1,
		FRAGMENT_DATA_MESSAGE_LENGTH : 2,
		FRAGMENT_DATA_MESSAGE_LENGTH1 : 3,
		FRAGMENT_DATA_MESSAGE_BODY : 4
	};

	this.fragmentStream = null;
	this.fragmentDatasFlag = KBEngine.FragmentDataTypes.FRAGMENT_DATA_UNKNOW;
	this.fragmentDatasRemain = 0;

	this.msgStream = new KBEngine.MemoryStream(KBEngine.PACKET_MAX_SIZE_TCP);

	this.resetSocket = function()
	{
		try
		{  
			if(KBEngine.app.socket != undefined && KBEngine.app.socket != null)
			{
				var sock = KBEngine.app.socket;
				
				sock.onopen = undefined;
				sock.onerror = undefined;
				sock.onmessage = undefined;
				sock.onclose = undefined;
				KBEngine.app.socket = null;
				sock.close();
			}
		}
		catch(e)
		{ 
		}
	}
	
	this.reset = function()
	{
		if(KBEngine.app.entities != undefined && KBEngine.app.entities != null)
		{
			KBEngine.app.clearEntities(true);
		}
		
		KBEngine.app.resetSocket();
		
		KBEngine.app.currserver = "loginapp";
		KBEngine.app.currstate = "create";
		KBEngine.app.currconnect = "loginapp";

		// 扩展数据
		KBEngine.app.serverdatas = "";
		
		// 版本信息
		KBEngine.app.serverVersion = "";
		KBEngine.app.serverScriptVersion = "";
		KBEngine.app.serverProtocolMD5 = "@{KBE_SERVER_PROTO_MD5}";
		KBEngine.app.serverEntityDefMD5 = "@{KBE_SERVER_ENTITYDEF_MD5}";
		KBEngine.app.clientVersion = "@{KBE_VERSION}";
		KBEngine.app.clientScriptVersion = "@{KBE_SCRIPT_VERSION}";
		
		// player的相关信息
		KBEngine.app.entity_uuid = null;
		KBEngine.app.entity_id = 0;
		KBEngine.app.entity_type = "";

		// 这个参数的选择必须与kbengine_defs.xml::cellapp/aliasEntityID的参数保持一致
		KBEngine.app.useAliasEntityID = @{KBE_USE_ALIAS_ENTITYID};

		// 当前玩家最后一次同步到服务端的位置与朝向与服务端最后一次同步过来的位置
		KBEngine.app.entityServerPos = new KBEngine.Vector3(0.0, 0.0, 0.0);
		
		// 客户端所有的实体
		KBEngine.app.entities = {};
		KBEngine.app.entityIDAliasIDList = [];
		KBEngine.app.controlledEntities = [];

		// 空间的信息
		KBEngine.app.spacedata = {};
		KBEngine.app.spaceID = 0;
		KBEngine.app.spaceResPath = "";
		KBEngine.app.isLoadedGeometry = false;
		
		var dateObject = new Date();
		KBEngine.app.lastTickTime = dateObject.getTime();
		KBEngine.app.lastTickCBTime = dateObject.getTime();
		
		KBEngine.mappingDataType();
		
		// 当前组件类别， 配套服务端体系
		KBEngine.app.component = "client";
	}

	this.installEvents = function()
	{
		KBEngine.Event.register(KBEngine.EventTypes.createAccount, KBEngine.app, "createAccount");
		KBEngine.Event.register(KBEngine.EventTypes.login, KBEngine.app, "login");
		KBEngine.Event.register(KBEngine.EventTypes.logout, KBEngine.app, "logout");
		KBEngine.Event.register(KBEngine.EventTypes.reloginBaseapp, KBEngine.app, "reloginBaseapp");
		KBEngine.Event.register(KBEngine.EventTypes.bindAccountEmail, KBEngine.app, "bindAccountEmail");
		KBEngine.Event.register(KBEngine.EventTypes.newPassword, KBEngine.app, "newPassword");
	}

	this.uninstallEvents = function()
	{
		KBEngine.Event.deregister(KBEngine.EventTypes.createAccount, KBEngine.app);
		KBEngine.Event.deregister(KBEngine.EventTypes.login, KBEngine.app);
		KBEngine.Event.deregister(KBEngine.EventTypes.logout, KBEngine.app);
		KBEngine.Event.deregister(KBEngine.EventTypes.reloginBaseapp, KBEngine.app);
		KBEngine.Event.deregister(KBEngine.EventTypes.bindAccountEmail, KBEngine.app);
		KBEngine.Event.deregister(KBEngine.EventTypes.newPassword, KBEngine.app);
	}
	
	this.hello = function()
	{  
		var bundle = KBEngine.Bundle.createObject();
		
		if(KBEngine.app.currserver == "loginapp")
			bundle.newMessage(KBEngine.messages.Loginapp_hello);
		else
			bundle.newMessage(KBEngine.messages.Baseapp_hello);
		
		bundle.writeString(KBEngine.app.clientVersion);
		bundle.writeString(KBEngine.app.clientScriptVersion);
		bundle.writeBlob(KBEngine.app.encryptedKey);
		bundle.send(KBEngine.app);
	}

	this.player = function()
	{
		return KBEngine.app.entities[KBEngine.app.entity_id];
	}

	this.findEntity = function(entityID)
	{
		return KBEngine.app.entities[entityID];
	}
		
	this.connect = function(addr)
	{
		console.assert(KBEngine.app.socket == null, "Assertion of socket not is null");
		
		try
		{  
			KBEngine.app.socket = new WebSocket(addr);  
		}
		catch(e)
		{  
			KBEngine.ERROR_MSG('WebSocket init error(' + e.toString() + ')!'); 
			KBEngine.Event.fire(KBEngine.EventTypes.onConnectionState, false);
			return;  
		}
		
		KBEngine.app.socket.binaryType = "arraybuffer";
		KBEngine.app.socket.onopen = KBEngine.app.onopen;  
		KBEngine.app.socket.onerror = KBEngine.app.onerror_before_onopen;  
		KBEngine.app.socket.onmessage = KBEngine.app.onmessage;  
		KBEngine.app.socket.onclose = KBEngine.app.onclose;
	}

	this.disconnect = function()
	{
		KBEngine.app.resetSocket();
	}
	
	this.onopen = function()
	{  
		KBEngine.INFO_MSG('connect success!');
		KBEngine.app.socket.onerror = KBEngine.app.onerror_after_onopen;
		KBEngine.Event.fire(KBEngine.EventTypes.onConnectionState, true);
	}

	this.onerror_before_onopen = function(evt)
	{  
		KBEngine.ERROR_MSG('onerror_before_onopen error:' + evt.data);
		KBEngine.app.resetSocket();
		KBEngine.Event.fire(KBEngine.EventTypes.onConnectionState, false);
	}
	
	this.onerror_after_onopen = function(evt)
	{
		KBEngine.ERROR_MSG('onerror_after_onopen error:' + evt.data);
		KBEngine.app.resetSocket();
		KBEngine.Event.fire(KBEngine.EventTypes.onDisconnected);
	}
	
	this.onmessage = function(msg)
	{ 
		var stream = KBEngine.app.msgStream;
		stream.setbuffer(msg.data);
		stream.wpos = msg.data.byteLength;

		var app =  KBEngine.app;
		var FragmentDataTypes = KBEngine.FragmentDataTypes;

		while(stream.length() > 0 || app.fragmentStream != null)
		{
			if(app.fragmentDatasFlag == FragmentDataTypes.FRAGMENT_DATA_UNKNOW)
			{
				if(app.currMsgID == 0)
				{
					if(KBEngine.MESSAGE_ID_LENGTH > 1 && stream.length() < KBEngine.MESSAGE_ID_LENGTH)
					{
						app.writeFragmentMessage(FragmentDataTypes.FRAGMENT_DATA_MESSAGE_ID, stream, KBEngine.MESSAGE_ID_LENGTH);
						break;
					}

					app.currMsgID = stream.readUint16();
				}
					
				var msgHandler = KBEngine.clientmessages[app.currMsgID];
				
				if(!msgHandler)
				{
					app.currMsgID = 0;
					app.currMsgLen = 0;
					KBEngine.ERROR_MSG("KBEngineApp::onmessage[" + app.currserver + "]: not found msg(" + app.currMsgID + ")!");
					break;
				}

				if(app.currMsgLen == 0)
				{
					var msglen = msgHandler.length;
					if(msglen == -1)
					{
						if(stream.length() < KBEngine.MESSAGE_LENGTH_LENGTH)
						{
							app.writeFragmentMessage(FragmentDataTypes.FRAGMENT_DATA_MESSAGE_LENGTH, stream, KBEngine.MESSAGE_LENGTH_LENGTH);
							break;
						}
						else
						{
							msglen = stream.readUint16();
							app.currMsgLen = msglen;

							// 扩展长度
							if(msglen == KBEngine.MESSAGE_MAX_SIZE)
							{
								if(stream.length() < KBEngine.MESSAGE_LENGTH1_LENGTH)
								{
									app.writeFragmentMessage(FragmentDataTypes.FRAGMENT_DATA_MESSAGE_LENGTH1, stream, KBEngine.MESSAGE_LENGTH1_LENGTH);
									break;
								}

								app.currMsgLen = stream.readUint32();
							}
						}
					}
					else
					{
						app.currMsgLen = msglen;
					}
				}

				if(app.fragmentStream != null && app.fragmentStream.length() >= app.currMsgLen)
				{
					msgHandler.handleMessage(app.fragmentStream);
					app.fragmentStream = null;
				}
				else if(stream.length() < app.currMsgLen && stream.length() > 0)
				{
					app.writeFragmentMessage(FragmentDataTypes.FRAGMENT_DATA_MESSAGE_BODY, stream, app.currMsgLen);
					break;
				}
				else
				{
					var wpos = stream.wpos;
					var rpos = stream.rpos + msglen;
					stream.wpos = rpos;
					msgHandler.handleMessage(stream);
					stream.wpos = wpos;
					stream.rpos = rpos;
				}

				app.currMsgID = 0;
				app.currMsgLen = 0;
				app.fragmentStream = null;
			}
			else
			{
				if(app.mergeFragmentMessage(stream))
					break;
			}
		}
	}  

	this.writeFragmentMessage = function(FragmentDataType, stream, datasize)
	{
		if(!(stream instanceof KBEngine.MemoryStream))
		{
			KBEngine.ERROR_MSG("writeFragmentMessage(): stream must be MemoryStream instances!");
			return;
		}

		var app = KBEngine.app;
		var opsize = stream.length();
		
		app.fragmentDatasRemain = datasize - opsize;
		app.fragmentDatasFlag = FragmentDataType;
		app.fragmentStream = stream;
	}

	this.mergeFragmentMessage = function(stream)
	{
		if(!(stream instanceof KBEngine.MemoryStream))
		{
			KBEngine.ERROR_MSG("mergeFragmentMessage(): stream must be MemoryStream instances!");
			return false;
		}

		var opsize = stream.length();
		if(opsize == 0)
			return false;

		var app = KBEngine.app;
		var fragmentStream = app.fragmentStream;
		console.assert(fragmentStream != null);

		if(opsize >= app.fragmentDatasRemain)
		{
			var FragmentDataTypes = KBEngine.FragmentDataTypes;
			fragmentStream.append(stream, stream.rpos, app.fragmentDatasRemain);

			switch(app.fragmentDatasFlag)
			{
				case FragmentDataTypes.FRAGMENT_DATA_MESSAGE_ID:
					app.currMsgID = fragmentStream.readUint16();
					app.fragmentStream = null;
					break;

				case FragmentDataTypes.FRAGMENT_DATA_MESSAGE_LENGTH:
					app.currMsgLen = fragmentStream.readUint16();
					app.fragmentStream = null;
					break;

				case FragmentDataTypes.FRAGMENT_DATA_MESSAGE_LENGTH1:
					app.currMsgLen = fragmentStream.readUint32();
					app.fragmentStream = null;
					break;

				case FragmentDataTypes.FRAGMENT_DATA_MESSAGE_BODY:
				default:
					break;
			}

			stream.rpos += app.fragmentDatasRemain;
			app.fragmentDatasFlag = FragmentDataTypes.FRAGMENT_DATA_UNKNOW;
			app.fragmentDatasRemain = 0;
			return false;
		}
		else
		{
			fragmentStream.append(stream, stream.rpos, opsize);
			app.fragmentDatasRemain -= opsize;
			stream.done();
			return true;
		}
	}

	this.onclose = function()
	{  
		KBEngine.INFO_MSG('connect close:' + KBEngine.app.currserver);

		if(KBEngine.app.currconnect != KBEngine.app.currserver)
			return;

		KBEngine.app.resetSocket();
		KBEngine.Event.fire(KBEngine.EventTypes.onDisconnected);
		//if(KBEngine.app.currserver != "loginapp")
		//	KBEngine.app.reset();
	}

	this.send = function(msg)
	{
		KBEngine.app.socket.send(msg);
	}

	this.close = function() {
		KBEngine.INFO_MSG('KBEngine::close()');
		KBEngine.app.socket.close();  
		KBEngine.app.reset();
	}
	
	this.update = function()
	{
		if(KBEngine.app.socket == null)
			return;

		var dateObject = new Date();
		if(KBEngine.app.args.serverHeartbeatTick > 0 && (dateObject.getTime() - KBEngine.app.lastTickTime) / 1000 > (KBEngine.app.args.serverHeartbeatTick / 2))
		{
			// 如果心跳回调接收时间小于心跳发送时间，说明没有收到回调
			// 此时应该通知客户端掉线了
			if(KBEngine.app.lastTickCBTime < KBEngine.app.lastTickTime)
			{
				KBEngine.ERROR_MSG("sendTick: Receive appTick timeout!");
				KBEngine.app.socket.close();  
			}
			
			if(KBEngine.app.currserver == "loginapp")
			{
				if(KBEngine.messages.Loginapp_onClientActiveTick != undefined)
				{
					var bundle = KBEngine.Bundle.createObject();
					bundle.newMessage(KBEngine.messages.Loginapp_onClientActiveTick);
					bundle.send(KBEngine.app);
				}
			}
			else
			{
				if(KBEngine.messages.Baseapp_onClientActiveTick != undefined)
				{
					var bundle = KBEngine.Bundle.createObject();
					bundle.newMessage(KBEngine.messages.Baseapp_onClientActiveTick);
					bundle.send(KBEngine.app);
				}
			}
			
			KBEngine.app.lastTickTime = dateObject.getTime();
		}
		
		KBEngine.app.updatePlayerToServer();
	}

	/*
		服务器心跳回调
	*/
	this.Client_onAppActiveTickCB = function()
	{
		var dateObject = new Date();
		KBEngine.app.lastTickCBTime = dateObject.getTime();
	}

	/*
		通过错误id得到错误描述
	*/
	this.serverErr = function(id)
	{
		var e = KBEngine.app.serverErrs[id];
		
		if(e == undefined)
		{
			return "";
		}

		return e.name + " [" + e.descr + "]";
	}

	/*
		服务端错误描述导入了
	*/
	this.Client_onImportServerErrorsDescr = function(stream)
	{
		var size = stream.readUint16();
		while(size > 0)
		{
			size -= 1;
			
			var e = new KBEngine.ServerErr();
			e.id = stream.readUint16();
			e.name = KBEngine.utf8ArrayToString(stream.readBlob());
			e.descr = KBEngine.utf8ArrayToString(stream.readBlob());
			
			KBEngine.app.serverErrs[e.id] = e;
				
			KBEngine.INFO_MSG("Client_onImportServerErrorsDescr: id=" + e.id + ", name=" + e.name + ", descr=" + e.descr);
		}
	}

	this.Client_onImportClientSdk = function(stream)
	{
		var remainingFiles = stream.readInt32();
		var fileName = stream.readString();
		var fileSize = stream.readInt32();
		var fileDatas = stream.readBlob()
		KBEngine.Event.fire("onImportClientSDK", remainingFiles, fileName, fileSize, fileDatas);
	}

	this.onOpenLoginapp_login = function()
	{  
		KBEngine.INFO_MSG("KBEngineApp::onOpenLoginapp_login: successfully!");
		KBEngine.Event.fire(KBEngine.EventTypes.onConnectionState, true);
		
		KBEngine.app.currserver = "loginapp";
		KBEngine.app.currstate = "login";
		
		if(!KBEngine.app.loginappMessageImported)
		{
			var bundle = KBEngine.Bundle.createObject();
			bundle.newMessage(KBEngine.messages.Loginapp_importClientMessages);
			bundle.send(KBEngine.app);
			KBEngine.app.socket.onmessage = KBEngine.app.Client_onImportClientMessages;  
			KBEngine.INFO_MSG("KBEngineApp::onOpenLoginapp_login: start importClientMessages ...");
			KBEngine.Event.fire("Loginapp_importClientMessages");
		}
		else
		{
			KBEngine.app.onImportClientMessagesCompleted();
		}
	}
	
	this.onOpenLoginapp_createAccount = function()
	{  
		KBEngine.Event.fire(KBEngine.EventTypes.onConnectionState, true);
		KBEngine.INFO_MSG("KBEngineApp::onOpenLoginapp_createAccount: successfully!");
		KBEngine.app.currserver = "loginapp";
		KBEngine.app.currstate = "createAccount";
		
		if(!KBEngine.app.loginappMessageImported)
		{
			var bundle = KBEngine.Bundle.createObject();
			bundle.newMessage(KBEngine.messages.Loginapp_importClientMessages);
			bundle.send(KBEngine.app);
			KBEngine.app.socket.onmessage = KBEngine.app.Client_onImportClientMessages;  
			KBEngine.INFO_MSG("KBEngineApp::onOpenLoginapp_createAccount: start importClientMessages ...");
			KBEngine.Event.fire("Loginapp_importClientMessages");
		}
		else
		{
			KBEngine.app.onImportClientMessagesCompleted();
		}
	}
	
	this.onImportClientMessagesCompleted = function()
	{
		KBEngine.INFO_MSG("KBEngineApp::onImportClientMessagesCompleted: successfully!");
		KBEngine.app.socket.onmessage = KBEngine.app.onmessage; 
		KBEngine.app.hello();
		
		if(KBEngine.app.currserver == "loginapp")
		{
			if(!KBEngine.app.serverErrorsDescrImported)
			{
				KBEngine.INFO_MSG("KBEngine::onImportClientMessagesCompleted(): send importServerErrorsDescr!");
				KBEngine.app.serverErrorsDescrImported = true;
				var bundle = KBEngine.Bundle.createObject();
				bundle.newMessage(KBEngine.messages.Loginapp_importServerErrorsDescr);
				bundle.send(KBEngine.app);
			}
							
			if(KBEngine.app.currstate == "login")
				KBEngine.app.login_loginapp(false);
			else if(KBEngine.app.currstate == "resetpassword")
				KBEngine.app.resetpassword_loginapp(false);
			else
				KBEngine.app.createAccount_loginapp(false);
			
			KBEngine.app.loginappMessageImported = true;
		}
		else
		{
			KBEngine.app.baseappMessageImported = true;
			
			if(!KBEngine.app.entitydefImported)
			{
				KBEngine.INFO_MSG("KBEngineApp::onImportClientMessagesCompleted: start importEntityDef ...");
				var bundle = KBEngine.Bundle.createObject();
				bundle.newMessage(KBEngine.messages.Baseapp_importClientEntityDef);
				bundle.send(KBEngine.app);
				KBEngine.Event.fire("Baseapp_importClientEntityDef");
			}
			else
			{
				KBEngine.app.onImportEntityDefCompleted();
			}
		}
	}

	this.createDataTypeFromStreams = function(stream, canprint)
	{
		var aliassize = stream.readUint16();
		KBEngine.INFO_MSG("KBEngineApp::createDataTypeFromStreams: importAlias(size=" + aliassize + ")!");
		
		while(aliassize > 0)
		{
			aliassize--;
			KBEngine.app.createDataTypeFromStream(stream, canprint);
		};	
		
		for(var datatype in KBEngine.datatypes)
		{
			if(KBEngine.datatypes[datatype] != undefined)
			{
				KBEngine.datatypes[datatype].bind();
			}
		}
	}

	this.createDataTypeFromStream = function(stream, canprint)
	{
		var utype = stream.readUint16();
		var name = stream.readString();
		var valname = stream.readString();
		
		/* 有一些匿名类型，我们需要提供一个唯一名称放到datatypes中
			如：
			<onRemoveAvatar>
				<Arg>	ARRAY <of> INT8 </of>		</Arg>
			</onRemoveAvatar>				
		*/
		if(valname.length == 0)
			valname = "Null_" + utype;
			
		if(canprint)
			KBEngine.INFO_MSG("KBEngineApp::Client_onImportClientEntityDef: importAlias(" + name + ":" + valname + ")!");
		
		if(name == "FIXED_DICT")
		{
			var datatype = new KBEngine.DATATYPE_FIXED_DICT();
			var keysize = stream.readUint8();
			datatype.implementedBy = stream.readString();
				
			while(keysize > 0)
			{
				keysize--;
				
				var keyname = stream.readString();
				var keyutype = stream.readUint16();
				datatype.dicttype[keyname] = keyutype;
			};
			
			KBEngine.datatypes[valname] = datatype;
		}
		else if(name == "ARRAY")
		{
			var uitemtype = stream.readUint16();
			var datatype = new KBEngine.DATATYPE_ARRAY();
			datatype.type = uitemtype;
			KBEngine.datatypes[valname] = datatype;
		}
		else
		{
			KBEngine.datatypes[valname] = KBEngine.datatypes[name];
		}

		KBEngine.datatypes[utype] = KBEngine.datatypes[valname];
		
		// 将用户自定义的类型补充到映射表中
		KBEngine.datatype2id[valname] = utype;
	}
	
	this.Client_onImportClientEntityDef = function(stream)
	{
		KBEngine.app.createDataTypeFromStreams(stream, true);
		
		while(stream.length() > 0)
		{
			var scriptmodule_name = stream.readString();
			var scriptUtype = stream.readUint16();
			var propertysize = stream.readUint16();
			var methodsize = stream.readUint16();
			var base_methodsize = stream.readUint16();
			var cell_methodsize = stream.readUint16();
			
			KBEngine.INFO_MSG("KBEngineApp::Client_onImportClientEntityDef: import(" + scriptmodule_name + "), propertys(" + propertysize + "), " +
					"clientMethods(" + methodsize + "), baseMethods(" + base_methodsize + "), cellMethods(" + cell_methodsize + ")!");
			
			KBEngine.moduledefs[scriptmodule_name] = {};
			var currModuleDefs = KBEngine.moduledefs[scriptmodule_name];
			currModuleDefs["name"] = scriptmodule_name;
			currModuleDefs["propertys"] = {};
			currModuleDefs["methods"] = {};
			currModuleDefs["base_methods"] = {};
			currModuleDefs["cell_methods"] = {};
			KBEngine.moduledefs[scriptUtype] = currModuleDefs;
			
			var self_propertys = currModuleDefs["propertys"];
			var self_methods = currModuleDefs["methods"];
			var self_base_methods = currModuleDefs["base_methods"];
			var self_cell_methods = currModuleDefs["cell_methods"];
			
			var Class = KBEngine[scriptmodule_name];
			
			while(propertysize > 0)
			{
				propertysize--;
				
				var properUtype = stream.readUint16();
				var properFlags = stream.readUint32();
				var aliasID = stream.readInt16();
				var name = stream.readString();
				var defaultValStr = stream.readString();
				var utype = KBEngine.datatypes[stream.readUint16()];
				var setmethod = null;
				
				if(Class != undefined)
				{
					setmethod = Class.prototype["set_" + name];
					if(setmethod == undefined)
						setmethod = null;
				}
				
				var savedata = [properUtype, aliasID, name, defaultValStr, utype, setmethod, properFlags];
				self_propertys[name] = savedata;
				
				if(aliasID != -1)
				{
					self_propertys[aliasID] = savedata;
					currModuleDefs["usePropertyDescrAlias"] = true;
				}
				else
				{
					self_propertys[properUtype] = savedata;
					currModuleDefs["usePropertyDescrAlias"] = false;
				}
				
				KBEngine.INFO_MSG("KBEngineApp::Client_onImportClientEntityDef: add(" + scriptmodule_name + "), property(" + name + "/" + properUtype + ").");
			};
			
			while(methodsize > 0)
			{
				methodsize--;
				
				var methodUtype = stream.readUint16();
				var aliasID = stream.readInt16();
				var name = stream.readString();
				var argssize = stream.readUint8();
				var args = [];
				
				while(argssize > 0)
				{
					argssize--;
					args.push(KBEngine.datatypes[stream.readUint16()]);
				};
				
				var savedata = [methodUtype, aliasID, name, args];
				self_methods[name] = savedata;
				
				if(aliasID != -1)
				{
					self_methods[aliasID] = savedata;
					currModuleDefs["useMethodDescrAlias"] = true;
				}
				else
				{
					self_methods[methodUtype] = savedata;
					currModuleDefs["useMethodDescrAlias"] = false;
				}
				
				KBEngine.INFO_MSG("KBEngineApp::Client_onImportClientEntityDef: add(" + scriptmodule_name + "), method(" + name + ").");
			};

			while(base_methodsize > 0)
			{
				base_methodsize--;
				
				var methodUtype = stream.readUint16();
				var aliasID = stream.readInt16();
				var name = stream.readString();
				var argssize = stream.readUint8();
				var args = [];
				
				while(argssize > 0)
				{
					argssize--;
					args.push(KBEngine.datatypes[stream.readUint16()]);
				};
				
				self_base_methods[name] = [methodUtype, aliasID, name, args];
				KBEngine.INFO_MSG("KBEngineApp::Client_onImportClientEntityDef: add(" + scriptmodule_name + "), base_method(" + name + ").");
			};
			
			while(cell_methodsize > 0)
			{
				cell_methodsize--;
				
				var methodUtype = stream.readUint16();
				var aliasID = stream.readInt16();
				var name = stream.readString();
				var argssize = stream.readUint8();
				var args = [];
				
				while(argssize > 0)
				{
					argssize--;
					args.push(KBEngine.datatypes[stream.readUint16()]);
				};
				
				self_cell_methods[name] = [methodUtype, aliasID, name, args];
				KBEngine.INFO_MSG("KBEngineApp::Client_onImportClientEntityDef: add(" + scriptmodule_name + "), cell_method(" + name + ").");
			};
			
			var defmethod = KBEngine[scriptmodule_name];

			if(defmethod == undefined)
			{
				KBEngine.ERROR_MSG("KBEngineApp::Client_onImportClientEntityDef: module(" + scriptmodule_name + ") not found!");
			}
			
			for(var name in currModuleDefs.propertys)
			{
				var infos = currModuleDefs.propertys[name];
				var properUtype = infos[0];
				var aliasID = infos[1];
				var name = infos[2];
				var defaultValStr = infos[3];
				var utype = infos[4];

				if(defmethod != undefined)
					defmethod.prototype[name] = utype.parseDefaultValStr(defaultValStr);
			};

			for(var name in currModuleDefs.methods)
			{
				var infos = currModuleDefs.methods[name];
				var properUtype = infos[0];
				var aliasID = infos[1];
				var name = infos[2];
				var args = infos[3];
				
				if(defmethod != undefined && defmethod.prototype[name] == undefined)
				{
					KBEngine.WARNING_MSG(scriptmodule_name + ":: method(" + name + ") no implement!");
				}
			};
		}
		
		KBEngine.app.onImportEntityDefCompleted();
	}

	this.Client_onVersionNotMatch = function(stream)
	{
		KBEngine.app.serverVersion = stream.readString();
		KBEngine.ERROR_MSG("Client_onVersionNotMatch: verInfo=" + KBEngine.app.clientVersion + " not match(server: " + KBEngine.app.serverVersion + ")");
		KBEngine.Event.fire(KBEngine.EventTypes.onVersionNotMatch, KBEngine.app.clientVersion, KBEngine.app.serverVersion);
	}

	this.Client_onScriptVersionNotMatch = function(stream)
	{
		KBEngine.app.serverScriptVersion = stream.readString();
		KBEngine.ERROR_MSG("Client_onScriptVersionNotMatch: verInfo=" + KBEngine.app.clientScriptVersion + " not match(server: " + KBEngine.app.serverScriptVersion + ")");
		KBEngine.Event.fire(KBEngine.EventTypes.onScriptVersionNotMatch, KBEngine.app.clientScriptVersion, KBEngine.app.serverScriptVersion);
	}
	
	this.onImportEntityDefCompleted = function()
	{
		KBEngine.INFO_MSG("KBEngineApp::onImportEntityDefCompleted: successfully!");
		KBEngine.app.entitydefImported = true;
		KBEngine.app.login_baseapp(false);
	}

	this.importClientMessages = function(stream)
	{
		var app = KBEngine.app;

		while(app.currMsgCount > 0)
		{
			app.currMsgCount--;
		
			var msgid = stream.readUint16();
			var msglen = stream.readInt16();
			var msgname = stream.readString();
			var argtype = stream.readInt8();
			var argsize = stream.readUint8();
			var argstypes = new Array(argsize);
			
			for(var i=0; i<argsize; i++)
			{
				argstypes[i] = stream.readUint8();
			}
			
			var handler = null;
			var isClientMethod = msgname.indexOf("Client_") >= 0;
			if(isClientMethod)
			{
				handler = app[msgname];
				if(handler == null || handler == undefined)
				{
					KBEngine.WARNING_MSG("KBEngineApp::onImportClientMessages[" + app.currserver + "]: interface(" + msgname + "/" + msgid + ") no implement!");
					handler = null;
				}
				else
				{
					KBEngine.INFO_MSG("KBEngineApp::onImportClientMessages: import(" + msgname + ") successfully!");
				}
			}
	
			if(msgname.length > 0)
			{
				KBEngine.messages[msgname] = new KBEngine.Message(msgid, msgname, msglen, argtype, argstypes, handler);
				
				if(isClientMethod)
					KBEngine.clientmessages[msgid] = KBEngine.messages[msgname];
				else
					KBEngine.messages[KBEngine.app.currserver][msgid] = KBEngine.messages[msgname];
			}
			else
			{
				KBEngine.messages[app.currserver][msgid] = new KBEngine.Message(msgid, msgname, msglen, argtype, argstypes, handler);
			}
		};

		app.onImportClientMessagesCompleted();
		app.currMsgID = 0;
		app.currMsgLen = 0;
		app.currMsgCount = 0;
		app.fragmentStream = null;
	}
	
	this.Client_onImportClientMessages = function(msg)
	{
		var stream = new KBEngine.MemoryStream(msg.data);
		stream.wpos = msg.data.byteLength;
		var app = KBEngine.app;

		if(app.currMsgID == 0)
		{
			app.currMsgID = stream.readUint16();
		} 

		if(app.currMsgID == KBEngine.messages.onImportClientMessages.id)
		{
			if(app.currMsgLen == 0) 
			{
				app.currMsgLen = stream.readUint16();
				app.currMsgCount = stream.readUint16();
			}
			
			var FragmentDataTypes = KBEngine.FragmentDataTypes
			if(stream.length() + 2 < app.currMsgLen && app.fragmentStream == null)
			{
				app.writeFragmentMessage(FragmentDataTypes.FRAGMENT_DATA_MESSAGE_BODY, stream, app.currMsgLen - 2);
			}
			else if(app.fragmentStream != null)
			{
				app.mergeFragmentMessage(stream);
				
				if(app.fragmentStream.length() + 2 >= app.currMsgLen)
				{
					app.importClientMessages(app.fragmentStream);
				}
			}
			else
			{
				app.importClientMessages(stream);
			}
		}
		else
		{
			KBEngine.ERROR_MSG("KBEngineApp::onmessage: not found msg(" + app.currMsgID + ")!");
		}
	}
	
	this.createAccount = function(username, password, datas)
	{  
		KBEngine.app.reset();
		KBEngine.app.username = username;
		KBEngine.app.password = password;
		KBEngine.app.clientdatas = datas;
		
		KBEngine.app.createAccount_loginapp(true);
	}
	
	this.getServerAddr = function(ip, port)
	{
		var serverAddr = KBEngine.app.protocol + ip;
		if(port != "")
		{
			serverAddr += ":" + port;
		}
		
		return serverAddr;
	}
	
	this.createAccount_loginapp = function(noconnect)
	{  
		if(noconnect)
		{
			var serverAddr = this.getServerAddr(KBEngine.app.ip, KBEngine.app.port);
			KBEngine.INFO_MSG("KBEngineApp::createAccount_loginapp: start connect to " + serverAddr + "!");
			KBEngine.app.currconnect = "loginapp";
			KBEngine.app.connect(serverAddr);
			KBEngine.app.socket.onopen = KBEngine.app.onOpenLoginapp_createAccount;  
		}
		else
		{
			var bundle = KBEngine.Bundle.createObject();
			bundle.newMessage(KBEngine.messages.Loginapp_reqCreateAccount);
			bundle.writeString(KBEngine.app.username);
			bundle.writeString(KBEngine.app.password);
			bundle.writeBlob(KBEngine.app.clientdatas);
			bundle.send(KBEngine.app);
		}
	}
	
	this.bindAccountEmail = function(emailAddress)
	{  
		var bundle = KBEngine.Bundle.createObject();
		bundle.newMessage(KBEngine.messages.Baseapp_reqAccountBindEmail);
		bundle.writeInt32(KBEngine.app.entity_id);
		bundle.writeString(KBEngine.app.password);
		bundle.writeString(emailAddress);
		bundle.send(KBEngine.app);
	}
	
	this.newPassword = function(old_password, new_password)
	{
		var bundle = KBEngine.Bundle.createObject();
		bundle.newMessage(KBEngine.messages.Baseapp_reqAccountNewPassword);
		bundle.writeInt32(KBEngine.app.entity_id);
		bundle.writeString(old_password);
		bundle.writeString(new_password);
		bundle.send(KBEngine.app);
	}
	
	this.login = function(username, password, datas)
	{  
		KBEngine.app.reset();
		KBEngine.app.username = username;
		KBEngine.app.password = password;
		KBEngine.app.clientdatas = datas;
		
		KBEngine.app.login_loginapp(true);
	}
	
	this.logout = function()
	{
		var bundle = KBEngine.Bundle.createObject();
		bundle.newMessage(KBEngine.messages.Baseapp_logoutBaseapp);
		bundle.writeUint64(KBEngine.app.entity_uuid);
		bundle.writeInt32(KBEngine.app.entity_id);
		bundle.send(KBEngine.app);
	}

	this.login_loginapp = function(noconnect)
	{  
		if(noconnect)
		{
			var serverAddr = this.getServerAddr(KBEngine.app.ip, KBEngine.app.port);
			KBEngine.INFO_MSG("KBEngineApp::login_loginapp: start connect to " + serverAddr + "!");
			KBEngine.app.currconnect = "loginapp";
			KBEngine.app.connect(serverAddr);
			KBEngine.app.socket.onopen = KBEngine.app.onOpenLoginapp_login;  
		}
		else
		{
			var bundle = KBEngine.Bundle.createObject();
			bundle.newMessage(KBEngine.messages.Loginapp_login);
			bundle.writeInt8(KBEngine.app.args.clientType); // clientType
			bundle.writeBlob(KBEngine.app.clientdatas);
			bundle.writeString(KBEngine.app.username);
			bundle.writeString(KBEngine.app.password);
			bundle.send(KBEngine.app);
		}
	}

	this.onOpenLoginapp_resetpassword = function()
	{  
		KBEngine.INFO_MSG("KBEngineApp::onOpenLoginapp_resetpassword: successfully!");
		KBEngine.app.currserver = "loginapp";
		KBEngine.app.currstate = "resetpassword";
		
		if(!KBEngine.app.loginappMessageImported)
		{
			var bundle = KBEngine.Bundle.createObject();
			bundle.newMessage(KBEngine.messages.Loginapp_importClientMessages);
			bundle.send(KBEngine.app);
			KBEngine.app.socket.onmessage = KBEngine.app.Client_onImportClientMessages;  
			KBEngine.INFO_MSG("KBEngineApp::onOpenLoginapp_resetpassword: start importClientMessages ...");
		}
		else
		{
			KBEngine.app.onImportClientMessagesCompleted();
		}
	}

	this.reset_password = function(username)
	{ 
		KBEngine.app.reset();
		KBEngine.app.username = username;
		KBEngine.app.resetpassword_loginapp(true);
	}
	
	this.resetpassword_loginapp = function(noconnect)
	{  
		if(noconnect)
		{
			var serverAddr = this.getServerAddr(KBEngine.app.ip, KBEngine.app.port);
			KBEngine.INFO_MSG("KBEngineApp::resetpassword_loginapp: start connect to " + serverAddr + "!");
			KBEngine.app.currconnect = "loginapp";
			KBEngine.app.connect(serverAddr);
			KBEngine.app.socket.onopen = KBEngine.app.onOpenLoginapp_resetpassword;  
		}
		else
		{
			var bundle = KBEngine.Bundle.createObject();
			bundle.newMessage(KBEngine.messages.Loginapp_reqAccountResetPassword);
			bundle.writeString(KBEngine.app.username);
			bundle.send(KBEngine.app);
		}
	}
	
	this.onOpenBaseapp = function()
	{
		KBEngine.INFO_MSG("KBEngineApp::onOpenBaseapp: successfully!");
		KBEngine.app.currserver = "baseapp";
		
		if(!KBEngine.app.baseappMessageImported)
		{
			var bundle = KBEngine.Bundle.createObject();
			bundle.newMessage(KBEngine.messages.Baseapp_importClientMessages);
			bundle.send(KBEngine.app);
			KBEngine.app.socket.onmessage = KBEngine.app.Client_onImportClientMessages;  
			KBEngine.Event.fire("Baseapp_importClientMessages");
		}
		else
		{
			KBEngine.app.onImportClientMessagesCompleted();
		}
	}
	
	this.login_baseapp = function(noconnect)
	{  
		if(noconnect)
		{
			KBEngine.Event.fire(KBEngine.EventTypes.onLoginBaseapp);
			var serverAddr = this.getServerAddr(KBEngine.app.baseappIp, KBEngine.app.baseappPort);
			KBEngine.INFO_MSG("KBEngineApp::login_baseapp: start connect to " + serverAddr + "!");
			KBEngine.app.currconnect = "baseapp";
			KBEngine.app.connect(serverAddr);
			
			if(KBEngine.app.socket != undefined && KBEngine.app.socket != null)
				KBEngine.app.socket.onopen = KBEngine.app.onOpenBaseapp;  
		}
		else
		{
			var bundle = KBEngine.Bundle.createObject();
			bundle.newMessage(KBEngine.messages.Baseapp_loginBaseapp);
			bundle.writeString(KBEngine.app.username);
			bundle.writeString(KBEngine.app.password);
			bundle.send(KBEngine.app);
		}
	}
	
	this.reloginBaseapp = function()
	{
		var dateObject = new Date();
		KBEngine.app.lastTickTime = dateObject.getTime();
		KBEngine.app.lastTickCBTime = dateObject.getTime();

		if(KBEngine.app.socket != undefined && KBEngine.app.socket != null)
			return;
		
		KBEngine.app.resetSocket();
		KBEngine.Event.fire(KBEngine.EventTypes.onReloginBaseapp);

		var serverAddr = this.getServerAddr(KBEngine.app.baseappIp, KBEngine.app.baseappPort);
		KBEngine.INFO_MSG("KBEngineApp::reloginBaseapp: start connect to " + serverAddr + "!");
		KBEngine.app.currconnect = "baseapp";
		KBEngine.app.connect(serverAddr);
		
		if(KBEngine.app.socket != undefined && KBEngine.app.socket != null)
			KBEngine.app.socket.onopen = KBEngine.app.onReOpenBaseapp;  
	}
	
	this.onReOpenBaseapp = function()
	{
		KBEngine.INFO_MSG("KBEngineApp::onReOpenBaseapp: successfully!");
		KBEngine.app.currserver = "baseapp";
		
		var bundle = KBEngine.Bundle.createObject();
		bundle.newMessage(KBEngine.messages.Baseapp_reloginBaseapp);
		bundle.writeString(KBEngine.app.username);
		bundle.writeString(KBEngine.app.password);
		bundle.writeUint64(KBEngine.app.entity_uuid);
		bundle.writeInt32(KBEngine.app.entity_id);
		bundle.send(KBEngine.app);
		
		var dateObject = new Date();
		KBEngine.app.lastTickCBTime = dateObject.getTime();
	}
	
	this.Client_onHelloCB = function(args)
	{
		KBEngine.app.serverVersion = args.readString();
		KBEngine.app.serverScriptVersion = args.readString();
		KBEngine.app.serverProtocolMD5 = args.readString();
		KBEngine.app.serverEntityDefMD5 = args.readString();
		
		var ctype = args.readInt32();
		
		KBEngine.INFO_MSG("KBEngineApp::Client_onHelloCB: verInfo(" + KBEngine.app.serverVersion + "), scriptVerInfo(" + 
			KBEngine.app.serverScriptVersion + "), serverProtocolMD5(" + KBEngine.app.serverProtocolMD5 + "), serverEntityDefMD5(" + 
			KBEngine.app.serverEntityDefMD5 + "), ctype(" + ctype + ")!");
		
		var dateObject = new Date();
		KBEngine.app.lastTickCBTime = dateObject.getTime();
	}
	
	this.Client_onLoginFailed = function(args)
	{
		var failedcode = args.readUint16();
		KBEngine.app.serverdatas = args.readBlob();
		KBEngine.ERROR_MSG("KBEngineApp::Client_onLoginFailed: failedcode=" + failedcode + "(" + KBEngine.app.serverErrs[failedcode].name + "), datas(" + KBEngine.app.serverdatas.length + ")!");
		KBEngine.Event.fire(KBEngine.EventTypes.onLoginFailed, failedcode);
	}
	
	this.Client_onLoginSuccessfully = function(args)
	{
		var accountName = args.readString();
		KBEngine.app.username = accountName;
		KBEngine.app.baseappIp = args.readString();
		KBEngine.app.baseappTcpPort = args.readUint16();
		KBEngine.app.baseappUdpPort = args.readUint16();
		KBEngine.app.serverdatas = args.readBlob();
		
		KBEngine.INFO_MSG("KBEngineApp::Client_onLoginSuccessfully: accountName(" + accountName + "), addr(" + 
				KBEngine.app.baseappIp + ":" + KBEngine.app.baseappTcpPort + ":" + KBEngine.app.baseappUdpPort + "), datas(" + KBEngine.app.serverdatas.length + ")!");
		
		KBEngine.app.disconnect();
		KBEngine.app.login_baseapp(true);
	}
	
	this.Client_onLoginBaseappFailed = function(failedcode)
	{
		KBEngine.ERROR_MSG("KBEngineApp::Client_onLoginBaseappFailed: failedcode=" + failedcode + "(" + KBEngine.app.serverErrs[failedcode].name + ")!");
		KBEngine.Event.fire(KBEngine.onLoginBaseappFailed.onLoginBaseappFailed, failedcode);
	}

	this.Client_onReloginBaseappFailed = function(failedcode)
	{
		KBEngine.ERROR_MSG("KBEngineApp::Client_onReloginBaseappFailed: failedcode="+ failedcode + "(" + KBEngine.app.serverErrs[failedcode].name + ")!");
		KBEngine.Event.fire(KBEngine.EventTypes.onReloginBaseappFailed, failedcode);
	}

	this.Client_onReloginBaseappSuccessfully = function(stream)
	{
		KBEngine.app.entity_uuid = stream.readUint64();
		KBEngine.DEBUG_MSG("KBEngineApp::Client_onReloginBaseappSuccessfully: " + KBEngine.app.username);
		KBEngine.Event.fire(KBEngine.EventTypes.onReloginBaseappSuccessfully);
	}
	
	this.entityclass = {};
	this.getentityclass = function(entityType)
	{
		var runclass = KBEngine.app.entityclass[entityType];
		if(runclass == undefined)
		{
			runclass = KBEngine[entityType];
			if(runclass == undefined)
			{
				KBEngine.ERROR_MSG("KBEngineApp::getentityclass: entityType(" + entityType + ") is error!");
				return runclass;
			}
			else
				KBEngine.app.entityclass[entityType] = runclass;
		}

		return runclass;
	}
	
	this.Client_onCreatedProxies = function(rndUUID, eid, entityType)
	{
		KBEngine.INFO_MSG("KBEngineApp::Client_onCreatedProxies: eid(" + eid + "), entityType(" + entityType + ")!");
		
		var entity = KBEngine.app.entities[eid];
		
		KBEngine.app.entity_uuid = rndUUID;
		KBEngine.app.entity_id = eid;
		
		if(entity == undefined)
		{
			var runclass = KBEngine.app.getentityclass(entityType);
			if(runclass == undefined)
				return;
			
			var entity = new runclass();
			entity.id = eid;
			entity.className = entityType;
			
			entity.base = new KBEngine.EntityCall();
			entity.base.id = eid;
			entity.base.className = entityType;
			entity.base.type = KBEngine.ENTITYCALL_TYPE_BASE;
			
			KBEngine.app.entities[eid] = entity;
			
			var entityMessage = KBEngine.bufferedCreateEntityMessages[eid];
			if(entityMessage != undefined)
			{
				KBEngine.app.Client_onUpdatePropertys(entityMessage);
				delete KBEngine.bufferedCreateEntityMessages[eid];
			}
				
			entity.__init__();
			entity.inited = true;
			
			if(KBEngine.app.args.isOnInitCallPropertysSetMethods)
				entity.callPropertysSetMethods();
		}
		else
		{
			var entityMessage = KBEngine.bufferedCreateEntityMessages[eid];
			if(entityMessage != undefined)
			{
				KBEngine.app.Client_onUpdatePropertys(entityMessage);
				delete KBEngine.bufferedCreateEntityMessages[eid];
			}
		}
	}
	
	this.getViewEntityIDFromStream = function(stream)
	{
		var id = 0;
		if(KBEngine.app.entityIDAliasIDList.Length > 255)
		{
			id = stream.readInt32();
		}
		else
		{
			var aliasID = stream.readUint8();

			// 如果为0且客户端上一步是重登陆或者重连操作并且服务端entity在断线期间一直处于在线状态
			// 则可以忽略这个错误, 因为cellapp可能一直在向baseapp发送同步消息， 当客户端重连上时未等
			// 服务端初始化步骤开始则收到同步信息, 此时这里就会出错。
			if(KBEngine.app.entityIDAliasIDList.length <= aliasID)
				return 0;
		
			id = KBEngine.app.entityIDAliasIDList[aliasID];
		}
		
		return id;
	}
	
	this.onUpdatePropertys_ = function(eid, stream)
	{
		var entity = KBEngine.app.entities[eid];
		
		if(entity == undefined)
		{
			var entityMessage = KBEngine.bufferedCreateEntityMessages[eid];
			if(entityMessage != undefined)
			{
				KBEngine.ERROR_MSG("KBEngineApp::Client_onUpdatePropertys: entity(" + eid + ") not found!");
				return;
			}
			
			var stream1 = new KBEngine.MemoryStream(stream.buffer);
			stream1.wpos = stream.wpos;
			stream1.rpos = stream.rpos - 4;
			KBEngine.bufferedCreateEntityMessages[eid] = stream1;
			return;
		}
		
		var currModule = KBEngine.moduledefs[entity.className];
		var pdatas = currModule.propertys;
		while(stream.length() > 0)
		{
			var utype = 0;
			if(currModule.usePropertyDescrAlias)
				utype = stream.readUint8();
			else
				utype = stream.readUint16();
		
			var propertydata = pdatas[utype];
			var setmethod = propertydata[5];
			var flags = propertydata[6];
			var val = propertydata[4].createFromStream(stream);
			var oldval = entity[propertydata[2]];
			
			KBEngine.INFO_MSG("KBEngineApp::Client_onUpdatePropertys: " + entity.className + "(id=" + eid  + " " + propertydata[2] + ", val=" + val + ")!");
			
			entity[propertydata[2]] = val;
			if(setmethod != null)
			{
				// base类属性或者进入世界后cell类属性会触发set_*方法
				if(flags == 0x00000020 || flags == 0x00000040)
				{
					if(entity.inited)
						setmethod.call(entity, oldval);
				}
				else
				{
					if(entity.inWorld)
						setmethod.call(entity, oldval);
				}
			}
		}
	}

	this.Client_onUpdatePropertysOptimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		KBEngine.app.onUpdatePropertys_(eid, stream);
	}
	
	this.Client_onUpdatePropertys = function(stream)
	{
		var eid = stream.readInt32();
		KBEngine.app.onUpdatePropertys_(eid, stream);
	}

	this.onRemoteMethodCall_ = function(eid, stream)
	{
		var entity = KBEngine.app.entities[eid];
		
		if(entity == undefined)
		{
			KBEngine.ERROR_MSG("KBEngineApp::Client_onRemoteMethodCall: entity(" + eid + ") not found!");
			return;
		}
		
		var methodUtype = 0;
		if(KBEngine.moduledefs[entity.className].useMethodDescrAlias)
			methodUtype = stream.readUint8();
		else
			methodUtype = stream.readUint16();
		
		var methoddata = KBEngine.moduledefs[entity.className].methods[methodUtype];
		var args = [];
		var argsdata = methoddata[3];
		for(var i=0; i<argsdata.length; i++)
		{
			args.push(argsdata[i].createFromStream(stream));
		}
		
		if(entity[methoddata[2]] != undefined)
		{
			entity[methoddata[2]].apply(entity, args);
		}
		else
		{
			KBEngine.ERROR_MSG("KBEngineApp::Client_onRemoteMethodCall: entity(" + eid + ") not found method(" + methoddata[2] + ")!");
		}
	}
	
	this.Client_onRemoteMethodCallOptimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		KBEngine.app.onRemoteMethodCall_(eid, stream);
	}
	
	this.Client_onRemoteMethodCall = function(stream)
	{
		var eid = stream.readInt32();
		KBEngine.app.onRemoteMethodCall_(eid, stream);
	}
	
	this.Client_onEntityEnterWorld = function(stream)
	{
		var eid = stream.readInt32();
		if(KBEngine.app.entity_id > 0 && eid != KBEngine.app.entity_id)
			KBEngine.app.entityIDAliasIDList.push(eid)
		
		var entityType;
		if(KBEngine.moduledefs.Length > 255)
			entityType = stream.readUint16();
		else
			entityType = stream.readUint8();
		
		var isOnGround = true;
		
		if(stream.length() > 0)
			isOnGround = stream.readInt8();
		
		entityType = KBEngine.moduledefs[entityType].name;
		KBEngine.INFO_MSG("KBEngineApp::Client_onEntityEnterWorld: " + entityType + "(" + eid + "), spaceID(" + KBEngine.app.spaceID + "), isOnGround(" + isOnGround + ")!");
		
		var entity = KBEngine.app.entities[eid];
		if(entity == undefined)
		{
			var entityMessage = KBEngine.bufferedCreateEntityMessages[eid];
			if(entityMessage == undefined)
			{
				KBEngine.ERROR_MSG("KBEngineApp::Client_onEntityEnterWorld: entity(" + eid + ") not found!");
				return;
			}
			
			var runclass = KBEngine.app.getentityclass(entityType);
			if(runclass == undefined)
				return;
			
			var entity = new runclass();
			entity.id = eid;
			entity.className = entityType;
			
			entity.cell = new KBEngine.EntityCall();
			entity.cell.id = eid;
			entity.cell.className = entityType;
			entity.cell.type = KBEngine.ENTITYCALL_TYPE_CELL;
			
			KBEngine.app.entities[eid] = entity;
			
			KBEngine.app.Client_onUpdatePropertys(entityMessage);
			delete KBEngine.bufferedCreateEntityMessages[eid];
			
			entity.isOnGround = isOnGround > 0;
			entity.__init__();
			entity.inited = true;
			entity.inWorld = true;
			entity.enterWorld();
			
			if(KBEngine.app.args.isOnInitCallPropertysSetMethods)
				entity.callPropertysSetMethods();
			
			entity.set_direction(entity.direction);
			entity.set_position(entity.position);
		}
		else
		{
			if(!entity.inWorld)
			{
				entity.cell = new KBEngine.EntityCall();
				entity.cell.id = eid;
				entity.cell.className = entityType;
				entity.cell.type = KBEngine.ENTITYCALL_TYPE_CELL;

				// 安全起见， 这里清空一下
				// 如果服务端上使用giveClientTo切换控制权
				// 之前的实体已经进入世界， 切换后的实体也进入世界， 这里可能会残留之前那个实体进入世界的信息
				KBEngine.app.entityIDAliasIDList = [];
				KBEngine.app.entities = {}
				KBEngine.app.entities[entity.id] = entity;

				entity.set_direction(entity.direction);
				entity.set_position(entity.position);

				KBEngine.app.entityServerPos.x = entity.position.x;
				KBEngine.app.entityServerPos.y = entity.position.y;
				KBEngine.app.entityServerPos.z = entity.position.z;
				
				entity.isOnGround = isOnGround > 0;
				entity.inWorld = true;
				entity.enterWorld();
				
				if(KBEngine.app.args.isOnInitCallPropertysSetMethods)
					entity.callPropertysSetMethods();
			}
		}
	}

	this.Client_onEntityLeaveWorldOptimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		KBEngine.app.Client_onEntityLeaveWorld(eid);
	}
	
	this.Client_onEntityLeaveWorld = function(eid)
	{
		var entity = KBEngine.app.entities[eid];
		if(entity == undefined)
		{
			KBEngine.ERROR_MSG("KBEngineApp::Client_onEntityLeaveWorld: entity(" + eid + ") not found!");
			return;
		}
		
		if(entity.inWorld)
			entity.leaveWorld();
		
		if(KBEngine.app.entity_id > 0 && eid != KBEngine.app.entity_id)
		{
			var newArray0 = [];

			for(var i=0; i<KBEngine.app.controlledEntities.length; i++)
			{
				if(KBEngine.app.controlledEntities[i] != eid)
				{
			       	newArray0.push(KBEngine.app.controlledEntities[i]);
				}
				else
				{
					KBEngine.Event.fire(KBEngine.EventTypes.onLoseControlledEntity);
				}
			}
			
			KBEngine.app.controlledEntities = newArray0
				
			delete KBEngine.app.entities[eid];
			
			var newArray = [];
			for(var i=0; i<KBEngine.app.entityIDAliasIDList.length; i++)
			{
				if(KBEngine.app.entityIDAliasIDList[i] != eid)
				{
					newArray.push(KBEngine.app.entityIDAliasIDList[i]);
				}
			}
			
			KBEngine.app.entityIDAliasIDList = newArray
		}
		else
		{
			KBEngine.app.clearSpace(false);
			entity.cell = null;
		}
	}

	this.Client_onEntityDestroyed = function(eid)
	{
		KBEngine.INFO_MSG("KBEngineApp::Client_onEntityDestroyed: entity(" + eid + ")!");
		
		var entity = KBEngine.app.entities[eid];
		if(entity == undefined)
		{
			KBEngine.ERROR_MSG("KBEngineApp::Client_onEntityDestroyed: entity(" + eid + ") not found!");
			return;
		}

		if(entity.inWorld)
		{
			if(KBEngine.app.entity_id == eid)
				KBEngine.app.clearSpace(false);

			entity.leaveWorld();
		}
			
		delete KBEngine.app.entities[eid];
	}
	
	this.Client_onEntityEnterSpace = function(stream)
	{
		var eid = stream.readInt32();
		KBEngine.app.spaceID = stream.readUint32();
		var isOnGround = true;
		
		if(stream.length() > 0)
			isOnGround = stream.readInt8();
		
		var entity = KBEngine.app.entities[eid];
		if(entity == undefined)
		{
			KBEngine.ERROR_MSG("KBEngineApp::Client_onEntityEnterSpace: entity(" + eid + ") not found!");
			return;
		}
		
		entity.isOnGround = isOnGround;
		KBEngine.app.entityServerPos.x = entity.position.x;
		KBEngine.app.entityServerPos.y = entity.position.y;
		KBEngine.app.entityServerPos.z = entity.position.z;
		entity.enterSpace();
	}
	
	this.Client_onEntityLeaveSpace = function(eid)
	{
		var entity = KBEngine.app.entities[eid];
		if(entity == undefined)
		{
			KBEngine.ERROR_MSG("KBEngineApp::Client_onEntityLeaveSpace: entity(" + eid + ") not found!");
			return;
		}
		
		KBEngine.app.clearSpace(false);
		entity.leaveSpace();
	}

	this.Client_onKicked = function(failedcode)
	{
		KBEngine.ERROR_MSG("KBEngineApp::Client_onKicked: failedcode=" + failedcode + "(" + KBEngine.app.serverErrs[failedcode].name + ")!");
		KBEngine.Event.fire(KBEngine.EventTypes.onKicked, failedcode);
	}

	this.Client_onCreateAccountResult = function(stream)
	{
		var retcode = stream.readUint16();
		var datas = stream.readBlob();
		
		KBEngine.Event.fire(KBEngine.EventTypes.onCreateAccountResult, retcode, datas);
		
		if(retcode != 0)
		{
			KBEngine.ERROR_MSG("KBEngineApp::Client_onCreateAccountResult: " + KBEngine.app.username + " create is failed! code=" + retcode + "(" + KBEngine.app.serverErrs[retcode].name + "!");
			return;
		}

		KBEngine.INFO_MSG("KBEngineApp::Client_onCreateAccountResult: " + KBEngine.app.username + " create is successfully!");
	}

	this.Client_onControlEntity = function(eid, isControlled)
	{
		var eid = stream.readInt32();
		var entity = KBEngine.app.entities[eid];
		if(entity == undefined)
		{
			KBEngine.ERROR_MSG("KBEngineApp::Client_onControlEntity: entity(" + eid + ") not found!");
			return;
		}
		
		var isCont = isControlled != 0;
		if (isCont)
		{
			// 如果被控制者是玩家自己，那表示玩家自己被其它人控制了
			// 所以玩家自己不应该进入这个被控制列表
			if (KBEngine.app.player().id != entity.id)
			{
				KBEngine.app.controlledEntities.push(entity)
			}
		}
		else
		{
			var newArray = [];

			for(var i=0; i<KBEngine.app.controlledEntities.length; i++)
				if(KBEngine.app.controlledEntities[i] != entity.id)
			       	newArray.push(KBEngine.app.controlledEntities[i]);
			
			KBEngine.app.controlledEntities = newArray
		}
		
		entity.isControlled = isCont;
		
		try
		{
			entity.onControlled(isCont);
			KBEngine.Event.fire(KBEngine.EventTypes.onControlled, entity, isCont);
		}
		catch (e)
		{
			KBEngine.ERROR_MSG("KBEngine::Client_onControlEntity: entity id = '" + eid + "', is controlled = '" + isCont + "', error = '" + e + "'");
		}
	}

	this.updatePlayerToServer = function()
	{
		var player = KBEngine.app.player();
		if(player == undefined || player.inWorld == false || KBEngine.app.spaceID == 0 || player.isControlled)
			return;
		
		if(player.entityLastLocalPos.distance(player.position) > 0.001 || player.entityLastLocalDir.distance(player.direction) > 0.001)
		{
			// 记录玩家最后一次上报位置时自身当前的位置
			player.entityLastLocalPos.x = player.position.x;
			player.entityLastLocalPos.y = player.position.y;
			player.entityLastLocalPos.z = player.position.z;
			player.entityLastLocalDir.x = player.direction.x;
			player.entityLastLocalDir.y = player.direction.y;
			player.entityLastLocalDir.z = player.direction.z;	
							
			var bundle = KBEngine.Bundle.createObject();
			bundle.newMessage(KBEngine.messages.Baseapp_onUpdateDataFromClient);
			bundle.writeFloat(player.position.x);
			bundle.writeFloat(player.position.y);
			bundle.writeFloat(player.position.z);
			bundle.writeFloat(player.direction.x);
			bundle.writeFloat(player.direction.y);
			bundle.writeFloat(player.direction.z);
			bundle.writeUint8(player.isOnGround);
			bundle.writeUint32(KBEngine.app.spaceID);
			bundle.send(KBEngine.app);
		}
		
		// 开始同步所有被控制了的entity的位置
		for (var eid in KBEngine.app.controlledEntities)  
		{ 
			var entity = KBEngine.app.controlledEntities[i];
			position = entity.position;
			direction = entity.direction;

			posHasChanged = entity.entityLastLocalPos.distance(position) > 0.001;
			dirHasChanged = entity.entityLastLocalDir.distance(direction) > 0.001;

			if (posHasChanged || dirHasChanged)
			{
				entity.entityLastLocalPos = position;
				entity.entityLastLocalDir = direction;

				var bundle = KBEngine.Bundle.createObject();
				bundle.newMessage(KBEngine.messages.Baseapp_onUpdateDataFromClientForControlledEntity);
				bundle.writeInt32(entity.id);
				bundle.writeFloat(position.x);
				bundle.writeFloat(position.y);
				bundle.writeFloat(position.z);

				bundle.writeFloat(direction.x);
				bundle.writeFloat(direction.y);
				bundle.writeFloat(direction.z);
				bundle.writeUint8(entity.isOnGround);
				bundle.writeUint32(KBEngine.app.spaceID);
				bundle.send(KBEngine.app);
			}
		}
	}
	
	this.addSpaceGeometryMapping = function(spaceID, respath)
	{
		KBEngine.INFO_MSG("KBEngineApp::addSpaceGeometryMapping: spaceID(" + spaceID + "), respath(" + respath + ")!");
		
		KBEngine.app.spaceID = spaceID;
		KBEngine.app.spaceResPath = respath;
		KBEngine.Event.fire(KBEngine.EventTypes.addSpaceGeometryMapping, respath);
	}

	this.clearSpace = function(isAll)
	{
		KBEngine.app.entityIDAliasIDList = [];
		KBEngine.app.spacedata = {};
		KBEngine.app.clearEntities(isAll);
		KBEngine.app.isLoadedGeometry = false;
		KBEngine.app.spaceID = 0;
	}

	this.clearEntities = function(isAll)
	{
		KBEngine.app.controlledEntities = []

		if(!isAll)
		{
			var entity = KBEngine.app.player();
			
			for (var eid in KBEngine.app.entities)  
			{ 
				if(eid == entity.id)
					continue;
				
				if(KBEngine.app.entities[eid].inWorld)
				{
			    	KBEngine.app.entities[eid].leaveWorld();
			    }
			    
			    KBEngine.app.entities[eid].onDestroy();
			}  
				
			KBEngine.app.entities = {}
			KBEngine.app.entities[entity.id] = entity;
		}
		else
		{
			for (var eid in KBEngine.app.entities)  
			{ 
				if(KBEngine.app.entities[eid].inWorld)
			    {
			    	KBEngine.app.entities[eid].leaveWorld();
			    }
			    
			    KBEngine.app.entities[eid].onDestroy();
			}  
				
			KBEngine.app.entities = {}
		}
	}

	this.Client_initSpaceData = function(stream)
	{
		KBEngine.app.clearSpace(false);
		
		KBEngine.app.spaceID = stream.readInt32();
		while(stream.length() > 0)
		{
			var key = stream.readString();
			var value = stream.readString();
			KBEngine.app.Client_setSpaceData(KBEngine.app.spaceID, key, value);
		}
		
		KBEngine.INFO_MSG("KBEngineApp::Client_initSpaceData: spaceID(" + KBEngine.app.spaceID + "), datas(" + KBEngine.app.spacedata + ")!");
	}
	
	this.Client_setSpaceData = function(spaceID, key, value)
	{
		KBEngine.INFO_MSG("KBEngineApp::Client_setSpaceData: spaceID(" + spaceID + "), key(" + key + "), value(" + value + ")!");
		
		KBEngine.app.spacedata[key] = value;
		
		if(key == "_mapping")
			KBEngine.app.addSpaceGeometryMapping(spaceID, value);
		
		KBEngine.Event.fire(KBEngine.EventTypes.onSetSpaceData, spaceID, key, value);
	}
	
	this.Client_delSpaceData = function(spaceID, key)
	{
		KBEngine.INFO_MSG("KBEngineApp::Client_delSpaceData: spaceID(" + spaceID + "), key(" + key + ")!");
		
		delete KBEngine.app.spacedata[key];
		KBEngine.Event.fire(KBEngine.EventTypes.onDelSpaceData, spaceID, key);
	}
	
	this.Client_getSpaceData = function(spaceID, key)
	{
		return KBEngine.app.spacedata[key];
	}
	
	this.Client_onUpdateBasePos = function(x, y, z)
	{
		KBEngine.app.entityServerPos.x = x;
		KBEngine.app.entityServerPos.y = y;
		KBEngine.app.entityServerPos.z = z;
	}
	
	this.Client_onUpdateBasePosXZ = function(x, z)
	{
		KBEngine.app.entityServerPos.x = x;
		KBEngine.app.entityServerPos.z = z;
	}
	
	this.Client_onUpdateData = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		var entity = KBEngine.app.entities[eid];
		if(entity == undefined)
		{
			KBEngine.ERROR_MSG("KBEngineApp::Client_onUpdateData: entity(" + eid + ") not found!");
			return;
		}
	}

	this.Client_onSetEntityPosAndDir = function(stream)
	{
		var eid = stream.readInt32();
		var entity = KBEngine.app.entities[eid];
		if(entity == undefined)
		{
			KBEngine.ERROR_MSG("KBEngineApp::Client_onSetEntityPosAndDir: entity(" + eid + ") not found!");
			return;
		}
		
		entity.position.x = stream.readFloat();
		entity.position.y = stream.readFloat();
		entity.position.z = stream.readFloat();
		entity.direction.x = stream.readFloat();
		entity.direction.y = stream.readFloat();
		entity.direction.z = stream.readFloat();
		
		// 记录玩家最后一次上报位置时自身当前的位置
		entity.entityLastLocalPos.x = entity.position.x;
		entity.entityLastLocalPos.y = entity.position.y;
		entity.entityLastLocalPos.z = entity.position.z;
		entity.entityLastLocalDir.x = entity.direction.x;
		entity.entityLastLocalDir.y = entity.direction.y;
		entity.entityLastLocalDir.z = entity.direction.z;	
				
		entity.set_direction(entity.direction);
		entity.set_position(entity.position);
	}
	
	this.Client_onUpdateData_ypr = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var y = stream.readFloat();
		var p = stream.readFloat();
		var r = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, y, p, r, -1, false);
	}
	
	this.Client_onUpdateData_yp = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var y = stream.readFloat();
		var p = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, y, p, KBEngine.KBE_FLT_MAX, -1, false);
	}
	
	this.Client_onUpdateData_yr = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var y = stream.readFloat();
		var r = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, y, KBEngine.KBE_FLT_MAX, r, -1, false);
	}
	
	this.Client_onUpdateData_pr = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var p = stream.readFloat();
		var r = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, p, r, -1, false);
	}
	
	this.Client_onUpdateData_y = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var y = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, y, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, -1, false);
	}
	
	this.Client_onUpdateData_p = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var p = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, p, KBEngine.KBE_FLT_MAX, -1, false);
	}
	
	this.Client_onUpdateData_r = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var r = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, r, -1, false);
	}
	
	this.Client_onUpdateData_xz = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var x = stream.readFloat();
		var z = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, x, KBEngine.KBE_FLT_MAX, z, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, 1, false);
	}
	
	this.Client_onUpdateData_xz_ypr = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var x = stream.readFloat();
		var z = stream.readFloat();

		var y = stream.readFloat();
		var p = stream.readFloat();
		var r = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, x, KBEngine.KBE_FLT_MAX, z, y, p, r, 1, false);
	}
	
	this.Client_onUpdateData_xz_yp = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var x = stream.readFloat();
		var z = stream.readFloat();

		var y = stream.readFloat();
		var p = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, x, KBEngine.KBE_FLT_MAX, z, y, p, KBEngine.KBE_FLT_MAX, 1, false);
	}
	
	this.Client_onUpdateData_xz_yr = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var x = stream.readFloat();
		var z = stream.readFloat();

		var y = stream.readFloat();
		var r = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, x, KBEngine.KBE_FLT_MAX, z, y, KBEngine.KBE_FLT_MAX, r, 1, false);
	}
	
	this.Client_onUpdateData_xz_pr = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var x = stream.readFloat();
		var z = stream.readFloat();

		var p = stream.readFloat();
		var r = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, x, KBEngine.KBE_FLT_MAX, z, KBEngine.KBE_FLT_MAX, p, r, 1, false);
	}
	
	this.Client_onUpdateData_xz_y = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var x = stream.readFloat();
		var z = stream.readFloat();

		var y = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, x, KBEngine.KBE_FLT_MAX, z, y, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, 1, false);
	}
	
	this.Client_onUpdateData_xz_p = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var x = stream.readFloat();
		var z = stream.readFloat();

		var p = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, xz[0], KBEngine.KBE_FLT_MAX, xz[1], KBEngine.KBE_FLT_MAX, p, KBEngine.KBE_FLT_MAX, 1, false);
	}
	
	this.Client_onUpdateData_xz_r = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var x = stream.readFloat();
		var z = stream.readFloat();

		var r = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, x, KBEngine.KBE_FLT_MAX, z, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, r, 1, false);
	}
	
	this.Client_onUpdateData_xyz = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var x = stream.readFloat();
		var y = stream.readFloat();
		var z = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, x, y, z, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, 0, false);
	}
	
	this.Client_onUpdateData_xyz_ypr = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var x = stream.readFloat();
		var y = stream.readFloat();
		var z = stream.readFloat();
		
		var yaw = stream.readFloat();
		var p = stream.readFloat();
		var r = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, x, y, z, yaw, p, r, 0, false);
	}
	
	this.Client_onUpdateData_xyz_yp = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var x = stream.readFloat();
		var y = stream.readFloat();
		var z = stream.readFloat();
		
		var yaw = stream.readFloat();
		var p = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, x, y, z, yaw, p, KBEngine.KBE_FLT_MAX, 0, false);
	}
	
	this.Client_onUpdateData_xyz_yr = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var x = stream.readFloat();
		var y = stream.readFloat();
		var z = stream.readFloat();
		
		var yaw = stream.readFloat();
		var r = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, x, y, z, yaw, KBEngine.KBE_FLT_MAX, r, 0, false);
	}
	
	this.Client_onUpdateData_xyz_pr = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var x = stream.readFloat();
		var y = stream.readFloat();
		var z = stream.readFloat();
		
		var p = stream.readFloat();
		var r = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, x, y, z, KBEngine.KBE_FLT_MAX, p, r, 0, false);
	}
	
	this.Client_onUpdateData_xyz_y = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var x = stream.readFloat();
		var y = stream.readFloat();
		var z = stream.readFloat();
		
		var yaw = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, x, y, z, yaw, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, 0, false);
	}
	
	this.Client_onUpdateData_xyz_p = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var x = stream.readFloat();
		var y = stream.readFloat();
		var z = stream.readFloat();
		
		var p = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, x, y, z, KBEngine.KBE_FLT_MAX, p, KBEngine.KBE_FLT_MAX, 0, false);
	}
	
	this.Client_onUpdateData_xyz_r = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var x = stream.readFloat();
		var y = stream.readFloat();
		var z = stream.readFloat();
		
		var p = stream.readFloat();
		
		KBEngine.app._updateVolatileData(eid, x, y, z, r, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, 0, false);
	}
	
	//--------------------optiom------------------------//
	this.Client_onUpdateData_ypr_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var y = stream.readInt8();
		var p = stream.readInt8();
		var r = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, y, p, r, -1, true);
	}
	
	this.Client_onUpdateData_yp_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var y = stream.readInt8();
		var p = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, y, p, KBEngine.KBE_FLT_MAX, -1, true);
	}
	
	this.Client_onUpdateData_yr_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var y = stream.readInt8();
		var r = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, y, KBEngine.KBE_FLT_MAX, r, -1, true);
	}
	
	this.Client_onUpdateData_pr_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var p = stream.readInt8();
		var r = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, p, r, -1, true);
	}
	
	this.Client_onUpdateData_y_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var y = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, y, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, -1, true);
	}
	
	this.Client_onUpdateData_p_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var p = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, p, KBEngine.KBE_FLT_MAX, -1, true);
	}
	
	this.Client_onUpdateData_r_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var r = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, r, -1, true);
	}
	
	this.Client_onUpdateData_xz_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var xz = stream.readPackXZ();
		
		KBEngine.app._updateVolatileData(eid, xz[0], KBEngine.KBE_FLT_MAX, xz[1], KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, 1, true);
	}
	
	this.Client_onUpdateData_xz_ypr_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var xz = stream.readPackXZ();

		var y = stream.readInt8();
		var p = stream.readInt8();
		var r = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, xz[0], KBEngine.KBE_FLT_MAX, xz[1], y, p, r, 1, true);
	}
	
	this.Client_onUpdateData_xz_yp_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var xz = stream.readPackXZ();

		var y = stream.readInt8();
		var p = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, xz[0], KBEngine.KBE_FLT_MAX, xz[1], y, p, KBEngine.KBE_FLT_MAX, 1, true);
	}
	
	this.Client_onUpdateData_xz_yr_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var xz = stream.readPackXZ();

		var y = stream.readInt8();
		var r = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, xz[0], KBEngine.KBE_FLT_MAX, xz[1], y, KBEngine.KBE_FLT_MAX, r, 1, true);
	}
	
	this.Client_onUpdateData_xz_pr_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var xz = stream.readPackXZ();

		var p = stream.readInt8();
		var r = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, xz[0], KBEngine.KBE_FLT_MAX, xz[1], KBEngine.KBE_FLT_MAX, p, r, 1, true);
	}
	
	this.Client_onUpdateData_xz_y_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var xz = stream.readPackXZ();

		var y = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, xz[0], KBEngine.KBE_FLT_MAX, xz[1], y, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, 1, true);
	}
	
	this.Client_onUpdateData_xz_p_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var xz = stream.readPackXZ();

		var p = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, xz[0], KBEngine.KBE_FLT_MAX, xz[1], KBEngine.KBE_FLT_MAX, p, KBEngine.KBE_FLT_MAX, 1, true);
	}
	
	this.Client_onUpdateData_xz_r_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var xz = stream.readPackXZ();

		var r = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, xz[0], KBEngine.KBE_FLT_MAX, xz[1], KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, r, 1, true);
	}
	
	this.Client_onUpdateData_xyz_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var xz = stream.readPackXZ();
		var y = stream.readPackY();
		
		KBEngine.app._updateVolatileData(eid, xz[0], y, xz[1], KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, 0, true);
	}
	
	this.Client_onUpdateData_xyz_ypr_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var xz = stream.readPackXZ();
		var y = stream.readPackY();
		
		var yaw = stream.readInt8();
		var p = stream.readInt8();
		var r = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, xz[0], y, xz[1], yaw, p, r, 0, true);
	}
	
	this.Client_onUpdateData_xyz_yp_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var xz = stream.readPackXZ();
		var y = stream.readPackY();
		
		var yaw = stream.readInt8();
		var p = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, xz[0], y, xz[1], yaw, p, KBEngine.KBE_FLT_MAX, 0, true);
	}
	
	this.Client_onUpdateData_xyz_yr_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var xz = stream.readPackXZ();
		var y = stream.readPackY();
		
		var yaw = stream.readInt8();
		var r = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, xz[0], y, xz[1], yaw, KBEngine.KBE_FLT_MAX, r, 0, true);
	}
	
	this.Client_onUpdateData_xyz_pr_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var xz = stream.readPackXZ();
		var y = stream.readPackY();
		
		var p = stream.readInt8();
		var r = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, x, y, z, KBEngine.KBE_FLT_MAX, p, r, 0, true);
	}
	
	this.Client_onUpdateData_xyz_y_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var xz = stream.readPackXZ();
		var y = stream.readPackY();
		
		var yaw = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, xz[0], y, xz[1], yaw, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, 0, true);
	}
	
	this.Client_onUpdateData_xyz_p_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var xz = stream.readPackXZ();
		var y = stream.readPackY();
		
		var p = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, xz[0], y, xz[1], KBEngine.KBE_FLT_MAX, p, KBEngine.KBE_FLT_MAX, 0, true);
	}
	
	this.Client_onUpdateData_xyz_r_optimized = function(stream)
	{
		var eid = KBEngine.app.getViewEntityIDFromStream(stream);
		
		var xz = stream.readPackXZ();
		var y = stream.readPackY();
		
		var p = stream.readInt8();
		
		KBEngine.app._updateVolatileData(eid, xz[0], y, xz[1], r, KBEngine.KBE_FLT_MAX, KBEngine.KBE_FLT_MAX, 0, true);
	}

	this._updateVolatileData = function(entityID, x, y, z, yaw, pitch, roll, isOnGround, isOptimized)
	{
		var entity = KBEngine.app.entities[entityID];
		if(entity == undefined)
		{
			// 如果为0且客户端上一步是重登陆或者重连操作并且服务端entity在断线期间一直处于在线状态
			// 则可以忽略这个错误, 因为cellapp可能一直在向baseapp发送同步消息， 当客户端重连上时未等
			// 服务端初始化步骤开始则收到同步信息, 此时这里就会出错。			
			KBEngine.ERROR_MSG("KBEngineApp::_updateVolatileData: entity(" + entityID + ") not found!");
			return;
		}
		
		// 小于0不设置
		if(isOnGround >= 0)
		{
			entity.isOnGround = (isOnGround > 0);
		}
		
		var changeDirection = false;
		
		if(roll != KBEngine.KBE_FLT_MAX)
		{
			changeDirection = true;
			entity.direction.x = KBEngine.int82angle(roll, false);
		}

		if(pitch != KBEngine.KBE_FLT_MAX)
		{
			changeDirection = true;
			entity.direction.y = KBEngine.int82angle(pitch, false);
		}
		
		if(yaw != KBEngine.KBE_FLT_MAX)
		{
			changeDirection = true;
			entity.direction.z = KBEngine.int82angle(yaw, false);
		}
		
		var done = false;
		if(changeDirection == true)
		{
			KBEngine.Event.fire(KBEngine.EventTypes.set_direction, entity);		
			done = true;
		}
		
		var positionChanged = false;
		if(x != KBEngine.KBE_FLT_MAX || y != KBEngine.KBE_FLT_MAX || z != KBEngine.KBE_FLT_MAX)
			positionChanged = true;

		if (x == KBEngine.KBE_FLT_MAX) x = 0.0;
		if (y == KBEngine.KBE_FLT_MAX) y = 0.0;
		if (z == KBEngine.KBE_FLT_MAX) z = 0.0;
        
		if(positionChanged)
		{
			if(isOptimized)
			{
				entity.position.x = x + KBEngine.app.entityServerPos.x;
				entity.position.y = y + KBEngine.app.entityServerPos.y;
				entity.position.z = z + KBEngine.app.entityServerPos.z;
			}
			else
			{
				entity.position.x = x;
				entity.position.y = y;
				entity.position.z = z;
			}
			
			
			done = true;
			KBEngine.Event.fire(KBEngine.EventTypes.updatePosition, entity);
		}
		
		if(done)
			entity.onUpdateVolatileData();		
	}
	
	this.Client_onStreamDataStarted = function(id, datasize, descr)
	{
		KBEngine.Event.fire(KBEngine.EventTypes.onStreamDataStarted, id, datasize, descr);
	}
	
	this.Client_onStreamDataRecv = function(stream)
	{
		var id = stream.readUint16();
		var data = stream.readBlob();
		KBEngine.Event.fire(KBEngine.EventTypes.onStreamDataRecv, id, data);
	}
	
	this.Client_onStreamDataCompleted = function(id)
	{
		KBEngine.Event.fire(KBEngine.EventTypes.onStreamDataCompleted, id);
	}
	
	this.Client_onReqAccountResetPasswordCB = function(failedcode)
	{
		if(failedcode != 0)
		{
			KBEngine.ERROR_MSG("KBEngineApp::Client_onReqAccountResetPasswordCB: " + KBEngine.app.username + " is failed! code=" + failedcode + "(" + KBEngine.app.serverErrs[failedcode].name + ")!");
			return;
		}

		KBEngine.INFO_MSG("KBEngineApp::Client_onReqAccountResetPasswordCB: " + KBEngine.app.username + " is successfully!");
	}
	
	this.Client_onReqAccountBindEmailCB = function(failedcode)
	{
		if(failedcode != 0)
		{
			KBEngine.ERROR_MSG("KBEngineApp::Client_onReqAccountBindEmailCB: " + KBEngine.app.username + " is failed! code=" + failedcode +"(" + KBEngine.app.serverErrs[failedcode].name + ")!");
			return;
		}

		KBEngine.INFO_MSG("KBEngineApp::Client_onReqAccountBindEmailCB: " + KBEngine.app.username + " is successfully!");
	}
	
	this.Client_onReqAccountNewPasswordCB = function(failedcode)
	{
		if(failedcode != 0)
		{
			KBEngine.ERROR_MSG("KBEngineApp::Client_onReqAccountNewPasswordCB: " + KBEngine.app.username + " is failed! code=" + failedcode + "(" +KBEngine.app.serverErrs[failedcode].name + ")!");
			return;
		}

		KBEngine.INFO_MSG("KBEngineApp::Client_onReqAccountNewPasswordCB: " + KBEngine.app.username + " is successfully!");
	}
}

KBEngine.create = function(kbengineArgs)
{
	if(KBEngine.app != undefined)
		return;

	// 一些平台如小程序上可能没有assert
	if(console.assert == undefined)
	{
		console.assert = function(bRet, s)
		{
			if(!(bRet)) {
				KBEngine.ERROR_MSG(s);
			}
		}
	}

	if(kbengineArgs.constructor != KBEngine.KBEngineArgs)
	{
		KBEngine.ERROR_MSG("KBEngine.create(): args(" + kbengineArgs + ") error! not is KBEngine.KBEngineArgs");
		return;
	}
	
	new KBEngine.KBEngineApp(kbengineArgs);
	
	KBEngine.app.reset();
	KBEngine.app.installEvents();
	KBEngine.idInterval = setInterval(KBEngine.app.update, kbengineArgs.updateHZ);
}

KBEngine.destroy = function()
{
	if(KBEngine.idInterval != undefined)
		clearInterval(KBEngine.idInterval);
	
	if(KBEngine.app == undefined)
		return;
		
	KBEngine.app.uninstallEvents();
	KBEngine.app.reset();
	KBEngine.app = undefined;

	KBEngine.Event.clear();
}

try
{
	if(module != undefined)
	{
		module.exports = KBEngine;
	}
}
catch(e)
{
	
}


