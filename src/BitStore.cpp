#include "Headers.h"
#include "Files.h"
#include "BitStore.h"


void BitStore::construct() {
	cursor_ = 0;
	length_ = 0;
//	WARN("making bitstore really big");store_.ensureCapacity(20000,false);
}
void BitStore::align(int nBits) {
		int n = (cursor_ + nBits - 1);
		n -= n % nBits;
		skip(n - cursor_);
	}

float BitStore::nextFloat()
{
	int n = next(32);
	return *((float *)&n);
}

int BitStore::next(int nBits) {
	int data = get(cursor_, nBits);
	cursor_ += nBits;
	return data;
}
 
void BitStore::growTo(int length)
{
	int pos, bit;
	bitPos(length, pos, bit);
	int iLen = pos + (bit > 0 ? 1 : 0);
	while (store_.length() < iLen) {
		store_.add(0);
	}
	length_ = maxVal(length_, length);
}

void BitStore::write(Sink &sink)
{
	// pad to a byte boundary
	int nBytes = (length() + 7) / 8;

	//pr(("BitStore, writing %d bytes from %d bits to sink\n",nBytes,length()));
	start();
	for (int i = 0; i < nBytes; i++) {
		char c = (char)next(8);
		//if (i == nBytes-1) pr((" last char = %d\n",c));
		sink << c;
	}
}

void BitStore::print(Sink *s, int minLength) const {
	if (s == 0)
		s = &Utils::getActiveSink();

	for (int i = 0; i < minLength; i++) {
		int b = 0;
		if (i < length())
			b = get(i,1);
		*s << ((b == 0) ? '.' : 'X');
	}
}

int BitStore::countBits(bool value) const
{
	int total = 0;
	static int nybCounts[] = {
		0,1,1,2,
		1,2,2,3,
		1,2,2,3,
		2,3,3,4,
	};
	for (int i = 0; i < store_.length(); i++) {
		int n = store_.itemAt(i);
		if (!value)
			n ^= ~0;
		for (int j = 0; j < BITS_PER_INT/4; j++, n >>= 4)
			total += nybCounts[n & 0xf];
	}
	return total;
}

void BitStore::set(int position, int data, int nBits)
{
#undef p2
#define p2(a) //pr(a)

	growTo(position + nBits);

	int pos, bit;
	bitPos(position, pos, bit);
	p2(("BitStore::set pos=%d, data=%X, nBits=%d\n",pos,data,nBits));

	int here = minVal(nBits, BITS_PER_INT - bit);
	maskOff(data, nBits);

	int curr = store_.itemAt(pos);
	// mask off current bits
	int mask = ~(((1 << here) - 1) << bit);

	p2((" curr=%d, mask=%X, data=%X\n",curr,mask,data<<bit));

	int val = (curr & mask) | (data << bit);
//	pr((" %3d:set %d (bit %d) to %d\n",id_,pos,bit,val));
	store_.set(pos, val);

	if (here < nBits) {
		pos++;
		bit=0;
		data >>= here;
		here = nBits - here;
		curr = store_.itemAt(pos);
		mask = ~((1 << here) - 1);
		int val = (curr & mask) | data;
//		pr(("* set %d to %d\n",pos,val));
		store_.set(pos, val);
		p2(("  remainder curr=%d, mask=%X, data=%X\n",curr,mask,data));

	}
	p2(("%s",s() ));

}

float BitStore::floatValue(float f)
{
	int *ptr = (int *)&f;

	return *((float *)ptr);
}


int BitStore::get(int position, int nBits) const
{
	ASSERT(position >= 0 
		/* && position + nBits <= length() */
		);
	int pos, bit;
	bitPos(position, pos, bit);
	int data = 0;

	int here = minVal(nBits, BITS_PER_INT - bit);
	if (here > 0) {
		data = 0;
		if (pos < store_.length())
			data = store_.itemAt(pos) >> bit;
		maskOff(data, here);
		nBits -= here;
	}
	if (nBits > 0) {
		pos++;
		bit = 0;
		int d2 = 0;
		if (pos < store_.length())
			d2 = store_.itemAt(pos);
		maskOff(d2, nBits);
		data |= d2 << here;
	}
	return data;
}

void BitStore::add(int data, int nBits)
{
	set(length(), data, nBits);
}

void BitStore::get(int position, int nBits, BitStore &dest) const
{
	while (nBits > 0) {
		int chunk = minVal(nBits, 32);
		int data = get(position, chunk);
		position += chunk;
		dest.add(data,chunk);
		nBits -= chunk;
	}
}


#if DEBUG
const char *BitStore::s(bool brief) const
{
	String w;
	w = debInfo(brief);
	String &w2 = Debug::str();
	w2 << w;
	return w2.chars();
}

String BitStore::debInfo(bool brief) const {

	const int ROWLEN = 64;
	String s;
	if (!brief) {
		s << "BitStore";
		s << " cursor=" << cursor();
		s << " len=" << length() << "\n";
	} else
		s << '[';

	int rows = (length() + ROWLEN-1) / ROWLEN;
	for (int i = 0; i < rows; i++) {
		int offset = i * ROWLEN;
		if (!brief) {
			char work[20];
			sprintf(work,"  %04X: ",offset);
			s << work;
		}
		int cnt = minVal(64, length() - offset);
		static char c[] = ".1";

		for (int j = 0; j < cnt; j++) {
			int bit = get(offset + j, 1);
			s << c[bit];
		}
		if (!brief)
			s << "\n";
	}
	if (brief)
		s << ']';
	return s;
}

void Test_BitStore() {
	int tests = 2;
	int t0 = 0, t1 = tests-1;

//	t0 = 1; t1 =t0;

	for (int t = t0; t <= t1; t++) {
String r;//		StringSink r;
#define TO_STR 1
		Utils::pushSink(TO_STR ? &r : 0);
		String work;

		StringArray sa;
		String buff;

		switch (t) {
		case 0:
			{
				Cout << "BitStore\n";

				BitStore bs;

				Cout << "Writing:\n";
				int pos = 0;
				buff.clear();

				int i0 = 33, i1 = 40;
				for (int i = i0; i < i1;  i++) {
					int data = i*i;
					int nBits = 2 + (i % 29);
					{
						int data2 = data;
						BitStore::maskOff(data2, nBits);
//						Cout << "Writing data=" << (int)data2 << "\n";
					}
					{
						int data2 = data;
						BitStore::maskOff(data2,nBits);
						Utils::intToBinary(data2, nBits, work, ".1");
						buff << work << " ";
						if (buff.length() >= 70) {
							sa.add(buff);
							buff.clear();
						}
					}
					bs.add(data,nBits);
					pos += nBits;
				}
				
				Cout << "Reading:\n";
				pos = 0;
				buff.clear();

				int j = 0;
				for (int i = i0; i < i1; i++) {
					int nBits = 2 + (i % 29);
					int data = bs.get(pos, nBits);
					pos += nBits;
					{
//						Cout << "Reading data=" << (int)data << "\n";
						Utils::intToBinary(data, nBits, work, ".1");
						buff << work << " ";
						if (buff.length() >= 70) {
							String orig;
							if (j < sa.length())
								orig.append(sa.itemAt(j));
							j++;
							Cout << orig << "\n";
							Cout << buff << "\n";
							if (!orig.equals(buff))
								Cout << "!!!!!!!!! Error\n";
							else
								Cout << "\n";
							buff.clear();
						}
					}
				}
			}
			break;
			case 1:
				{
					BitStore bs;
					static int scr[] = {
						95,32,0x84218421,
						127,31,0x84218421,
						7,3,273,
						14,21,12329832,
						52,7,2398,
						37,0,1233,
						72,14,232989389,
						-999	
					};

					Array<int> stored;
					for (int pass = 0; pass < 2; pass++) {
					//	Utils::srand(1965);
						for (int i = 0; scr[i] >= 0; i+=3) {
							int pos = scr[i];
							int nBits = scr[i+1];
							int val = scr[i+2];
							BitStore::maskOff(val, nBits);
							//int pos = Utils::rand(250);
							if (pass == 0) {
								stored.add(val);
								bs.set(pos, val, nBits);
								Cout << fmt((int)val, 12);
							} else {
								int val2 = bs.get(pos, nBits);
								Cout << fmt((int)val2, 12);
							}
							if ((i/3) % 6 == 5) Cout << "\n";
						}
						Cout << "\n";
					}

					bs.start();
					int count = 0;
					while (!bs.done()) {
						int bts = minVal(4, bs.length() - bs.cursor());
						int val = bs.next(bts);
						String s;
						Utils::intToBinary(val, bts, s, ".1");
						Cout << s << " ";
						if (++count == 8) {
							count = 0; Cout << "\n";
						}
					}
					Cout << "\n";
					Cout << bs.debInfo();
				}
				break;
		}
		Utils::popSink();
	#if TO_STR
		String name("BitStore");
		name << t;
		Test(name.chars(), r);
	#endif
	}
}
#endif

void BitStore::bitwiseOr(const BitStore &other)
{
	// expand so we can do array manipulation quickly

	int ol = other.store_.length();
	growTo(ol * BITS_PER_INT);
	for (int i = store_.length() - 1; i >= 0; i--) {
		if (i < ol)
			store_[i] |= other.store_[i];
	}

}
void BitStore::bitwiseAnd(const BitStore &other)
{
	// expand so we can do array manipulation quickly
	int ol = other.store_.length();
	growTo(ol * BITS_PER_INT);
	for (int i = store_.length() - 1; i >= 0; i--) {
		if (i < ol)
			store_[i] &= other.store_[i];
		else
			store_[i] = 0;
	}
}
void BitStore::bitwiseXor(const BitStore &other)
{
	// expand so we can do array manipulation quickly
	int ol = other.store_.length();
	growTo(ol * BITS_PER_INT);
	for (int i = store_.length() - 1; i >= 0; i--) {
		if (i < ol)
			store_[i] ^= other.store_[i];
	}
}

