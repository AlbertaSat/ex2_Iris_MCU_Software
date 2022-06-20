/*
 * nand_errno.h
 *
 *  Created on: Jun 14, 2022
 *      Author: robert
 */

#ifndef NAND_ERRNO_H_
#define NAND_ERRNO_H_

/*  The errno numbers are the same as Linux.
 */

/** Operation not permitted. */
#define NAND_EPERM 1

/** No such file or directory. */
#define NAND_ENOENT 2

/** I/O error. */
#define NAND_EIO 5

/** Bad file number. */
#define NAND_EBADF 9

/** Out of memory */
#define NAND_ENOMEM 12

/** Device or resource busy. */
#define NAND_EBUSY 16

/** File exists. */
#define NAND_EEXIST 17

/** Cross-device link. */
#define NAND_EXDEV 18

/** Not a directory. */
#define NAND_ENOTDIR 20

/** Is a directory. */
#define NAND_EISDIR 21

/** Invalid argument. */
#define NAND_EINVAL 22

/** File table overflow. */
#define NAND_ENFILE 23

/** Too many open files. */
#define NAND_EMFILE 24

/** File too large. */
#define NAND_EFBIG 27

/** No space left on device. */
#define NAND_ENOSPC 28

/** Read-only file system. */
#define NAND_EROFS 30

/** Too many links. */
#define NAND_EMLINK 31

/** Math result not representable. */
#define NAND_ERANGE 34

/** File name too long. */
#define NAND_ENAMETOOLONG 36

/** Function not implemented. */
#define NAND_ENOSYS 38

/** Directory not empty. */
#define NAND_ENOTEMPTY 39

/** No data available. */
#define NAND_ENODATA 61

/** Too many users. */
#define NAND_EUSERS 87

/** Nothing will be okay ever again. */
#define NAND_EFUBAR NAND_EINVAL

#endif /* NAND_ERRNO_H_ */
