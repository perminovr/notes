
#include <stdint.h>

#define CONTROLBYTE_DLE ((uint8_t)(0x10))
#define CONTROLBYTE_STX ((uint8_t)(0x55))
#define CONTROLBYTE_ETX ((uint8_t)(0x77))

typedef struct {
	// input
	int fcsSize;	// max bytes in fcs
	int maxSize;	// max frame size
	void *priv;		// priv for getByIdx
	uint8_t (*getByIdx)(int idx, void *priv); // get value by idx; simple case is 'return priv->storage[idx];'
	// output
	uint8_t *fcs;	// storage for fcs after ... dle etx ; could be 0
} custom_buffer_t;

static int findFrame(const custom_buffer_t *buf, int *_start, int *_end)
{
	int maxFrameIdx;

	*_start = 0;
	*_end = 0;

	int maxSize = buf->maxSize - buf->fcsSize;

	#undef byIdx
	#define byIdx(ii) buf->getByIdx(ii,buf->priv)
	for (int i = 0; i < buf->maxSize; ++i) {
		// поиск старт байт
		if (byIdx(i) == CONTROLBYTE_DLE && byIdx(i+1) == CONTROLBYTE_STX) {
			// фиксация старта
			*_start = i;
			// начало поиска хвоста
			for (i += 2; i < maxSize; ++i) {
				// найден конец кадра
				if (byIdx(i) == CONTROLBYTE_DLE && byIdx(i+1) == CONTROLBYTE_ETX) {
					// конец закрыт dle? // . dle dle etx
					if (byIdx(i-1) == CONTROLBYTE_DLE && byIdx(i-2) != CONTROLBYTE_DLE) {
						continue; // продолжить поиск конца
					} else {
						// кадр найден
						*_end = i+1; // to etx
						return 0;
					}
				}
				// найден новый старт
				if (byIdx(i) == CONTROLBYTE_DLE && byIdx(i+1) == CONTROLBYTE_STX) {
					// не закрыт => новый старт // ... dle dle stx
					// crc здесь быть не может, иначе бы встретился конец кадра
					if (byIdx(i-1) != CONTROLBYTE_DLE) {
						i--; // начать со старта
						break; // to for (int i = 0; i < buf->maxSize; ++i)
					}
				}
			}
			// превышение размера
			return -1;
		}
	}
	return -1;
}

int decode(const custom_buffer_t *buf, uint8_t *output/* , int osize not less buf->maxSize*/)
{
	int start, end;
	int oi = 0;
	memset(output, 0, buf->maxSize);
	if (findFrame(buf, &start, &end) == 0) {
		#undef byIdx
		#define byIdx(ii) buf->getByIdx(ii,buf->priv)
		for (int i = start+2; i < end-2; ++i) { // dle + stx / dle + etx
			if (byIdx(i) == CONTROLBYTE_DLE) {
				i++;
			}
			output[oi++] = byIdx(i);
		}
		if (buf->fcs) {
			for (int i = 0; i < buf->fcsSize; ++i) {
				buf->fcs[i] = byIdx(end+1+i);
			}
		}
		return 0;
	}
	return -1;
}

int encode(const uint8_t *input, int isize, uint8_t *output, int osize)
{
	int i = 0;
	output[i++] = CONTROLBYTE_DLE;
	output[i++] = CONTROLBYTE_STX;
	for (int j = 0; j < isize && i < osize-4; ++j) {
		uint8_t v = input[j];
		if (v == CONTROLBYTE_DLE) {
			output[i++] = CONTROLBYTE_DLE;
		}
		output[i++] = v;
	}
	output[i++] = CONTROLBYTE_DLE;
	output[i++] = CONTROLBYTE_ETX;
	return i;
}