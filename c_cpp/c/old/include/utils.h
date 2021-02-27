

#include <stdint.h>


/*! ---------------------------------------------------------------------------
 * @fn: strnstr
 *
 * @brief: returns a pointer to the first occurrence of s2 in s1, 
 *	or a null pointer if s2 is not part of s1
 *
 * input parameters
 * @param s1 - buffer to be scanned (null does not use as terminator)
 * @param s2 - c-string containing the sequence of characters to match
 * @param len - size of s1 buffer
 *
 * output parameters
 *
 * return a pointer to the first occurrence in s1 of the entire sequence of 
 *	characters specified in s2, or a null pointer if the sequence is not 
 *	present in s1
*/
extern char *strnstr(const char *s1, const char *s2, size_t len);



/*! ---------------------------------------------------------------------------
 * @fn: replaceData
 *
 * @brief: replaces data specified by start-pointer with new data specified by
 *	data-pointer
 *
 * NOTE: size of buffer containing replaced data must be higher or equal to
 *	(size-len) bytes
 *
 * input parameters
 * @param start - a pointer to start-byte of replaced data
 * @param len - size of replaced data in bytes
 * @param end - end of buffer contains replaced data
 * @param data - a data to be inserted
 * @param size - size of inserted data
 *
 * output parameters
 *
 * no return 
*/
extern void replaceData(uint8_t *start, size_t len, uint8_t *end, const uint8_t *data, size_t size);




