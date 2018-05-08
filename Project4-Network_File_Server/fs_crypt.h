/*
 * fs_crypt.h
 *
 * File server encryption routines.
 */

#ifndef _FS_CRYPT_H_
#define _FS_CRYPT_H_

/*
 * Encrypt a block of data using the null-terminated string "password" as the
 * encryption key, using AES (Rijndael) encryption.  The block of data to be
 * encrypted is pointed to by buf_cleartext and is of size size_cleartext.
 *
 * fs_encrypt allocates and returns a buffer that contains the
 * encrypted data.  It also returns the size of the encrypted data in
 * *size_ciphertext_ptr.  Caller has responsibility to free the allocated
 * buffer by calling delete [] buf, where buf is the return value from
 * fs_encrypt.
 *
 * fs_encrypt is thread safe.
 */
void *fs_encrypt(const char *password, const void *buf_cleartext,
                 const unsigned int size_cleartext,
                 unsigned int *size_ciphertext_ptr);

/*
 * Decrypt a block of data using the null-terminated string "password" as the
 * decryption key, using AES (Rijndael) decryption.  The block of data to be
 * decrypted is pointed to by buf_ciphertext and is of size size_ciphertext.
 *
 * fs_decrypt allocates and returns a buffer that contains the
 * decrypted data.  It also returns the size of the decrypted data in
 * *size_cleartext_ptr.  Caller has responsibility to free the allocated
 * buffer by calling delete [] buf, where buf is the return value from
 * fs_decrypt.
 *
 * fs_decrypt returns nullptr if decryption fails (e.g., due to a password that
 * doesn't match the one used in encryption).
 *
 * fs_decrypt is thread safe.
 */
void *fs_decrypt(const char *password, const void *buf_ciphertext,
                 const unsigned int size_ciphertext,
                 unsigned int *size_cleartext_ptr);

#endif /* _FS_CRYPT_H_ */
