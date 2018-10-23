namespace KBEngine
{
    using UnityEngine;
    using System;
    using System.Collections;
    using System.Collections.Generic;

    public abstract class EncryptionFilter
    {
        public abstract void encrypt(MemoryStream stream);

        public abstract void decrypt(MemoryStream stream);
        public abstract void decrypt(byte[] buffer, int startIndex, int length);

        public abstract bool send(PacketSenderBase sender, MemoryStream stream);
        public abstract bool recv(MessageReaderBase reader, byte[] buffer, UInt32 rpos, UInt32 len);
    }


    class BlowfishFilter : EncryptionFilter
    {
        private Blowfish _blowfish = new Blowfish();
        
        private MemoryStream _packet = new MemoryStream();
        MemoryStream _enctyptStrem = new MemoryStream();

        private UINT8 _padSize = 0;
        private UInt16 _packLen = 0;
        const UInt32 BLOCK_SIZE = 8;
        const UInt32 MIN_PACKET_SIZE = sizeof(UInt16) + 1 + BLOCK_SIZE;

        public BlowfishFilter()
        {
        }

        ~BlowfishFilter()
        {
        }

        public  byte[] key()
        {
            return _blowfish.key();
        }

        public override void encrypt(MemoryStream stream)
        {
            int padSize = 0;
            if (stream.length() % BLOCK_SIZE != 0)
            {
                padSize = (int)(BLOCK_SIZE - (stream.length() % BLOCK_SIZE));
                stream.wpos += padSize;

				if(stream.wpos > MemoryStream.BUFFER_MAX)
					Debug.LogError("BlowfishFilter::encrypt: stream.wpos(" + stream.wpos + ") > MemoryStream.BUFFER_MAX(" + MemoryStream.BUFFER_MAX + ")!");
            }

            _blowfish.encipher(stream.data(), (int)stream.length());

            UInt16 packLen = (UInt16)(stream.length() + 1);
            _enctyptStrem.writeUint16(packLen);
            _enctyptStrem.writeUint8((UINT8)padSize);
            _enctyptStrem.append(stream.data(), (UInt32)stream.rpos, stream.length());

            stream.swap(_enctyptStrem);
            _enctyptStrem.clear();
        }

        public override void decrypt(MemoryStream stream)
        {
            _blowfish.decipher(stream.data(), stream.rpos, (int)stream.length());
        }

        public override void decrypt(byte[] buffer, int startIndex, int length)
        {
            _blowfish.decipher(buffer, startIndex, length);
        }

        public override bool send(PacketSenderBase sender, MemoryStream stream)
        {
            if(!_blowfish.isGood())
            {
                Dbg.ERROR_MSG("BlowfishFilter::send: Dropping packet, due to invalid filter");
                return false;
            }

            encrypt(stream);
            return sender.send(stream);
        }

        public override bool recv(MessageReaderBase reader, byte[] buffer, UInt32 rpos, UInt32 len)
        {
            if (!_blowfish.isGood())
            {
                Dbg.ERROR_MSG("BlowfishFilter::recv: Dropping packet, due to invalid filter");
                return false;
            }

            if (_packet.length() == 0 && len >= MIN_PACKET_SIZE && BitConverter.ToUInt16(buffer, (int)rpos) - 1 == len - 3)
            {
                int packLen = BitConverter.ToUInt16(buffer, (int)rpos) - 1;
                int padSize = buffer[rpos + 2];

                decrypt(buffer, (int)(rpos + 3), (int)(len - 3));

                int length = packLen - padSize;
                if (reader != null)
                {
                    reader.process(buffer, rpos + 3, (UInt32)length);
                }

                return true;
            }

            _packet.append(buffer, rpos, len);

            while(_packet.length() > 0)
            {
                UInt32 currLen = 0;
                int oldwpos = 0;
                if (_packLen <= 0)
                {
                    // 如果满足一个最小包则尝试解包, 否则缓存这个包待与下一个包合并然后解包
                    if (_packet.length() >= MIN_PACKET_SIZE)
                    {
                        _packLen = _packet.readUint16();
                        _padSize = _packet.readUint8();

                        _packLen -= 1;

                        if (_packet.length() > _packLen)
                        {
                            currLen = (UInt32)(_packet.rpos + _packLen);
                            oldwpos = _packet.wpos;
                            _packet.wpos = (int)currLen;
                        }
                        else if (_packet.length() < _packLen)
                        {
                            return false;
                        }
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    // 如果上一次有做过解包行为但包还没有完整则继续处理
                    // 如果包是完整的下面流程会解密， 如果有多余的内容需要将其剪裁出来待与下一个包合并
                    if (_packet.length() > _packLen)
                    {
                        currLen = (UInt32)(_packet.rpos + _packLen);
                        oldwpos = _packet.wpos;
                        _packet.wpos = (int)currLen;
                    }
                    else if (_packet.length() < _packLen)
                    {
                        return false;
                    }
                }

                decrypt(_packet);
                _packet.wpos -= _padSize;

                // 上面的流程能保证wpos之后不会有多余的包
                // 如果有多余的包数据会放在_recvStream
                if (reader != null)
                {
                    reader.process(_packet.data(), (UInt32)_packet.rpos, _packet.length());
                }

                if (currLen > 0)
                {
                    _packet.rpos = (int)currLen;
                    _packet.wpos = oldwpos;
                }
                else
                {
                    _packet.clear();
                }

                _packLen = 0;
                _padSize = 0;
            }
            
            return true;
        }
    }
}

