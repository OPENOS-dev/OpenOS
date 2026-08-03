Please refer to the following document for more context:
https://docs.google.com/a/google.com/Doc?docid=0ARAbWVi1BBqoY2dzaHYyNDRfNjdoY3I4d2Rocw&hl=en

The contents of this directory are as follows:

drivers/
	Contains device driver for Infineon TPM in Intel Macs. As explained in
	the aforementioned document, the TPM in Intel Macs is very suitable for
	testing and development because it can be completely reset in software.

keychain/
	Source for a command-line application to help us work with TPM-based
	keys both in the context of Chrome OS and potentially other projects.
	This is work in progress.

keychain/modules/openssh/
	Source for a dynamically loadable module that transparently enables
	the use of TPM-based keys for public key based authentication in
	OpenSSH.

keychain/modules/openssl/
	Semi-functional source for making OpenSSL work seamlessly with a TPM.

nvtool/
	Source for a command-line application for managing the TPM's protected
	non-volatile (NV) memory. 

patches/
	Our patches for the TrouSerS libray and the TPM Emulator.

tools/
	Miscellaneous tools.
