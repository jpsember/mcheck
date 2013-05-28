#ifndef _BITSTORE
#define _BITSTORE

class BitStore : public Sink {
public:
	BitStore() {
		construct();
	}
	virtual ~BitStore() {}

	/*	Add some bits to the store
			> data						bits to add (least sig bit added first)
			> nBits						# bits in data (0..32)
	*/
	void add(int data, int nBits);

	/*	Perform a bitwise OR of this BitStore with another.
			Sets current to current | other.

			> other						BitStore to read from
	*/
	void bitwiseOr(const BitStore &other);
	void bitwiseAnd(const BitStore &other);
	void bitwiseXor(const BitStore &other);

	/*	Write bits to store.  Pads store with zero bits if writing
			past end of current store
			> position				starting bit number
			> data						bits to set to
			> nBits						number of bits to change
	*/
	void set(int position, int data, int nBits);

	/*	Write a single bit to the store
	*/
	void set(int position, bool value = true) 
		{ set(position,value ? 1 : 0,1);}

	/*	Read bits from store
			> position				starting bit number
			> nBits						number of bits to read
			< bits read (bit #position = least sig bit)
	*/
	int get(int position, int nBits) const;

	/*	Read a single bit from the store
			> position
			< true if bit was true
	*/
	bool get(int position) const {return get(position,1) != 0;}

	/*	Convert float to the value it would have if it was stored
			and retrieved in the bitstore.
			Useful for ensuring synchronicity during compression.
	*/
	static float floatValue(float f);
	/*	Convert double to the value it would have if it was stored
			and retrieved in the bitstore.
			Useful for ensuring synchronicity during compression.
	*/
	static double doubleValue(double f);

	/*	Read bits from store to a destination store
			> position				starting bit number
			> nBits						number of bits to read
			> dest						destination store (it's appended to)
	*/
	void get(int position, int nBits, BitStore &dest) const;
			
	/*	Grow to at least a certain length
	*/
	void growTo(int length);

	/*	Pad to a multiple of some number of bits
	*/
	void pad(int len = 8) {
		int i = length() + len - 1;
		growTo(i - i % len);
	}

	/*	Determine # bits in store
	*/
	int length() const {return length_;}

	int lengthInBytes() const {return (length_ + 7) / 8;}

	/*	Mask off high bits
			> data						bits to mask
			> nBits						# bits to isolate
	*/
	static void maskOff(int &data, int nBits) {
		if (nBits < 32)
			((uint &)data) &= (uint)((1 << nBits)-1);
	}

	/*	Prepare for reading bits; reset cursor to bit #0
	*/
	void start() {
		cursor_ = 0;
	}

	/*	Skip a certain number of bits while reading
	*/
	void skip(int nBits) {
		cursor_ += nBits;
	}

	void align(int nBits = 8);

	bool next() {return next(1) != 0;}
	int next(int nBits);
	float nextFloat();

	int cursor() const {
		return cursor_;
	}

	bool done() const {
		return cursor_ == length();
	}

	/*	Print bit storage usage as '....X..XXX.XX..'
			> s								sink to print to
			> minLen					minimum number of bits to print (can be larger
												than actual # bits in buffer)
	*/
	void print(Sink *s = 0, int minLen = 0) const;

	/*	Count the number of bits with a certain value
			> value						true to count ON bits, false for OFF bits
			< number of bits with that value
	*/
	int countBits(bool value = true) const;

	bool operator[](int i) const {return get(i);}

	/*	Write bits to byte buffer.
			Pads to byte boundary.
			
	*/
	void write(Sink &sink);

#if DEBUG
	String debInfo(bool brief = false) const;
	const char *s(bool brief = false) const;
#endif
	void clear() {
		length_ = 0;
		cursor_ = 0;
		store_.clear();
	}

	// sink interface:
	virtual Sink& operator << (const char *s) {
		for (int i = 0; s[i]; i++)
			*this << s[i];
		return *this;
	}
	virtual Sink& operator << (const String &s) {
		return this->operator << (s.chars());}
	virtual Sink& operator << (int i) {
		add(i, 32);
		return *this;
	}
	virtual Sink& operator << (char c) {
		add(c, 8);
		return *this;
	}
	virtual Sink& operator << (short i){
		add(i, 16);
		return *this;
	}
	virtual Sink& operator << (double d){
		int *ptr = (int *)&d;
		*this << ptr[0];
		*this << ptr[1];
		return *this;
	}
	virtual Sink& operator << (float f){
		int *ptr = (int *)&f;
		*this << ptr[0];
		return *this;
	}

private:
#if DEBUG
	//int id_;
#endif
	void construct();

	enum {
		BITS_PER_INT = 32,
	};
	static int bitToInt(int bitNum) {
		// 32 bits per int, so divide by 32
		return bitNum >> 5;
	};
	static void bitPos(int bitNum, int &pos, int &bit) {
		pos = bitToInt(bitNum);
		bit = bitNum & 0x1f;
	}

	// storage for bits
	Array<int> store_;

	// # bits in storage (may be less than n * store_.length() )
	int length_;

	// bit position for iterating
	int cursor_;
};

#if DEBUG
void Test_BitStore();
#endif

#endif // _BITSTORE
