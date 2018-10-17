
#ifndef _SHA1_H_
#define _SHA1_H_

namespace KBEngine {

class KBE_SHA1
{
public:

	KBE_SHA1();
	virtual ~KBE_SHA1();

	/*
		*  Re-initialize the class
		*/
	void Reset();

	/*
		*  Returns the message digest
		*/
	bool Result(unsigned *message_digest_array);

	/*
		*  Provide input to SHA1
		*/
	void Input(const unsigned char *message_array,
		unsigned            length);
	void Input(const char  *message_array,
		unsigned    length);
	void Input(unsigned char message_element);
	void Input(char message_element);
	KBE_SHA1& operator<<(const char *message_array);
	KBE_SHA1& operator<<(const unsigned char *message_array);
	KBE_SHA1& operator<<(const char message_element);
	KBE_SHA1& operator<<(const unsigned char message_element);

private:

	/*
		*  Process the next 512 bits of the message
		*/
	void ProcessMessageBlock();

	/*
		*  Pads the current message block to 512 bits
		*/
	void PadMessage();

	/*
		*  Performs a circular left shift operation
		*/
	inline unsigned CircularShift(int bits, unsigned word);

	unsigned H[5];                      // Message digest buffers

	unsigned Length_Low;                // Message length in bits
	unsigned Length_High;               // Message length in bits

	unsigned char Message_Block[64];    // 512-bit message blocks
	int Message_Block_Index;            // Index into message block array

	bool Computed;                      // Is the digest computed?
	bool Corrupted;                     // Is the message digest corruped?

};

}

#endif // _SHA1_H_
