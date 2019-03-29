
#include "MessageReader.h"
#include "Messages.h"
#include "KBDebug.h"
#include "MemoryStream.h"

namespace KBEngine
{

MessageReader::MessageReader():
	msgid_(0),
	msglen_(0),
	expectSize_(2),
	state_(READ_STATE_MSGID),
	pMemoryStream_(new MemoryStream())
{
}

MessageReader::~MessageReader()
{
	KBE_SAFE_RELEASE(pMemoryStream_);
}

void MessageReader::process(const uint8* datas, MessageLengthEx offset, MessageLengthEx length)
{
	MessageLengthEx totallen = offset;

	while (length > 0 && expectSize_ > 0)
	{
		if (state_ == READ_STATE_MSGID)
		{
			if (length >= expectSize_)
			{
				memcpy(pMemoryStream_->data() + pMemoryStream_->wpos(), datas + totallen, expectSize_);
				totallen += expectSize_;
				pMemoryStream_->wpos(pMemoryStream_->wpos() + expectSize_);
				length -= expectSize_;

				(*pMemoryStream_) >> msgid_;
				pMemoryStream_->clear(false);

				Message* pMsg = Messages::findClientMessage(msgid_);
				if (pMsg == NULL)
				{
					SCREEN_ERROR_MSG("MessageReader::process(): not found Message(%d)!", msgid_);
					break;
				}

				if (pMsg->msglen == -1)
				{
					state_ = READ_STATE_MSGLEN;
					expectSize_ = 2;
				}
				else if (pMsg->msglen == 0)
				{
					// 如果是0个参数的消息，那么没有后续内容可读了，处理本条消息并且直接跳到下一条消息
					pMsg->handleMessage(*pMemoryStream_);

					state_ = READ_STATE_MSGID;
					expectSize_ = 2;
				}
				else
				{
					expectSize_ = (MessageLengthEx)pMsg->msglen;
					state_ = READ_STATE_BODY;
				}
			}
			else
			{
				memcpy(pMemoryStream_->data() + pMemoryStream_->wpos(), datas + totallen, length);
				pMemoryStream_->wpos(pMemoryStream_->wpos() + length);
				expectSize_ -= length;
				break;
			}
		}
		else if (state_ == READ_STATE_MSGLEN)
		{
			if (length >= expectSize_)
			{
				memcpy(pMemoryStream_->data() + pMemoryStream_->wpos(), datas + totallen, expectSize_);
				totallen += expectSize_;
				pMemoryStream_->wpos(pMemoryStream_->wpos() + expectSize_);
				length -= expectSize_;

				(*pMemoryStream_) >> msglen_;
				pMemoryStream_->clear(false);

				// 长度扩展
				if (msglen_ >= 65535)
				{
					state_ = READ_STATE_MSGLEN_EX;
					expectSize_ = 4;
				}
				else
				{
					state_ = READ_STATE_BODY;
					expectSize_ = msglen_;
				}
			}
			else
			{
				memcpy(pMemoryStream_->data() + pMemoryStream_->wpos(), datas + totallen, length);
				pMemoryStream_->wpos(pMemoryStream_->wpos() + length);
				expectSize_ -= length;
				break;
			}
		}
		else if (state_ == READ_STATE_MSGLEN_EX)
		{
			if (length >= expectSize_)
			{
				memcpy(pMemoryStream_->data() + pMemoryStream_->wpos(), datas + totallen, expectSize_);
				totallen += expectSize_;
				pMemoryStream_->wpos(pMemoryStream_->wpos() + expectSize_);
				length -= expectSize_;

				(*pMemoryStream_) >> expectSize_;
				pMemoryStream_->clear(false);

				state_ = READ_STATE_BODY;
			}
			else
			{
				memcpy(pMemoryStream_->data() + pMemoryStream_->wpos(), datas + totallen, length);
				pMemoryStream_->wpos(pMemoryStream_->wpos() + length);
				expectSize_ -= length;
				break;
			}
		}
		else if (state_ == READ_STATE_BODY)
		{
			if (length >= expectSize_)
			{
				pMemoryStream_->append(datas, totallen, expectSize_);
				totallen += expectSize_;
				length -= expectSize_;

				Message* pMsg = Messages::findClientMessage(msgid_);

				if (pMsg == NULL)
				{
					SCREEN_ERROR_MSG("MessageReader::process(): not found Message(%d)!", msgid_);
					break;
				}

				pMsg->handleMessage(*pMemoryStream_);

				pMemoryStream_->clear(false);
				state_ = READ_STATE_MSGID;
				expectSize_ = 2;
			}
			else
			{
				pMemoryStream_->append(datas, totallen, length);
				expectSize_ -= length;
				break;
			}
		}
	}
}

}