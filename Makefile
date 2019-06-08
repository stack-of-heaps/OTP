all: otp_enc_d otp_enc otp_dec_d otp_dec keygen

otp_enc_d: 
	gcc otp_enc_d.c magicsauce.c

otp_enc: 
	gcc otp_enc.c magicsauce.c

otp_dec_d: 
	gcc otp_dec_d.c magicsauce.c

otp_dec: 
	gcc otp_dec.c magicsauce.c

keygen: 
	gcc keygen.c magicsauce.c

clean:
	rm keygen otp_enc_d otp_enc otp_dec_d otp_dec 