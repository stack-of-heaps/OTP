all: otp_enc_d otp_enc otp_dec_d otp_dec keygen

otp_enc_d: 
	gcc -o otp_enc_d otp_enc_d.c magicsauce.c

otp_enc: 
	gcc -o otp_enc otp_enc.c magicsauce.c

otp_dec_d: 
	gcc -o otp_dec_d otp_dec_d.c magicsauce.c

otp_dec: 
	gcc -o otp_dec otp_dec.c magicsauce.c

keygen: 
	gcc -o keygen keygen.c magicsauce.c

clean:
	rm keygen otp_enc_d otp_enc otp_dec_d otp_dec
