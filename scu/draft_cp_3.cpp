#include <cstdlib>
#include <cassert>
#include <vector>
#include <iostream>

using namespace std;

struct chunk
{
	void init(size_t blockSize, unsigned char blockNum)
	{
		pData_ = static_cast<unsigned char*>(malloc(blockSize * blockNum));

		firstAvailBlock_ = 0;
		blocksAvailable_ = blockNum;

		unsigned char i = 0;
		unsigned char* p = pData_;
		
		for (; i != blockNum; p += blockSize)
			*p = ++i;
	}

	void* allocate(size_t blockSize)
	{
		if (!blocksAvailable_)
			return 0;

		unsigned char* pRes = pData_ + (firstAvailBlock_ * blockSize);

		firstAvailBlock_ = *pRes;
		--blocksAvailable_;
		return pRes;
	}

	void deallocate(void* p, size_t blockSize)
	{
		assert(p >= pData_);

		unsigned char* pRel = static_cast<unsigned char*>(p);
		assert((pRel - pData_) % blockSize == 0);

		*pRel = firstAvailBlock_;
		firstAvailBlock_ = static_cast<unsigned char>((pRel - pData_) / blockSize);

		assert(firstAvailBlock_ == (pRel - pData_) / blockSize);
		++blocksAvailable_;
	}

	unsigned char* pData_;
	unsigned char  firstAvailBlock_;
	unsigned char  blocksAvailable_;
};


class FixedAllocator
{
public:
	void* allocate()
	{
		if (!allocChunk_ || allocChunk_->blocksAvailable_ == 0)
		{
			Chunks::iterator it = chunks_.begin();

			for (; it != chunks_.end(); ++it)
			{
				if (it->blocksAvailable_)
				{
					allocChunk_ = &*it;
					break;
				}
			}

			if (it == chunks_.end())
			{
				chunks_.reserve(chunks_.size() + 1);
				chunk c;
				c.init(blockSize_, blockNum_);
				chunks_.push_back(c);
				
				allocChunk_ = &chunks_.back();
				deallocChunk_ = &chunks_.back();
			}
		}

		assert(allocChunk_);
		assert(allocChunk_->blocksAvailable_);

		return allocChunk_->allocate(blockSize_);
	}
private:
	size_t blockSize_;
	unsigned char blockNum_;
	typedef vector<chunk> Chunks;
	Chunks chunks_;
	chunk* allocChunk_;
	chunk* deallocChunk_;
};

class SmallObjectAllocator
{
public:
	SmallObjectAllocator(size_t chunkSize, size_t maxObjSize) : chunkSize_(chunkSize), maxObjSize_(maxObjSize)
	{
	}

	void* allocate(size_t numBytes)
	{
		if (numBytes > maxObjSize_)
			return malloc(numBytes);

		// binary search pool_ for a FixedAllocator of the given size
	}

	void deallocate(void* p, size_t sz)
	{
		if (sz > maxObjSize_)
			free(p);
		else
		{

		}
	}
private:
	size_t chunkSize_;
	size_t maxObjSize_;

	vector<FixedAllocator> pool_;
	FixedAllocator* pLastAlloc_;
	FixedAllocator* pLastDealloc_;
};

#define CH_SIZE 	100
#define MAX_OBJ_SIZE 	100
//TODO
template <typename T>
struct Singleton 
{
	static T& instance()
	{
		static T t(CH_SIZE, MAX_OBJ_SIZE);
		return t;
	}
};

typedef Singleton<SmallObjectAllocator> MyAlloc;

class SmallObject
{
public:
	static void* operator new(size_t size)
	{
		return MyAlloc::instance().allocate(size);
	}

	static void operator delete(void* p, size_t sz)
	{
		MyAlloc::instance().deallocate(p, sz);
	}

	virtual ~SmallObject() {}
};

int main()
{
};
