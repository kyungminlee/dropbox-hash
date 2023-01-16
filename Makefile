dhash: dhash.c
	$(CC) -O3 -o $@ $< -std=c99 -I /opt/homebrew/opt/openssl/include -L /opt/homebrew/opt/openssl/lib /opt/homebrew/opt/openssl/lib/libcrypto.a
